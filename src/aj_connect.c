/**
 * @file
 */
/******************************************************************************
 * Copyright 2012-2013, Qualcomm Innovation Center, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 ******************************************************************************/


#include "aj_target.h"
#include "aj_status.h"
#include "aj_bufio.h"
#include "aj_msg.h"
#include "aj_connect.h"
#include "aj_introspect.h"
#include "aj_sasl.h"
#include "aj_net.h"
#include "aj_debug.h"
#include "aj_bus.h"
#include "aj_disco.h"
#include "aj_std.h"
#include "aj_auth.h"


static const char daemonService[] = "org.alljoyn.BusNode";

static uint32_t DefaultBusAuthPwdFunc(uint8_t* buffer, uint32_t bufLen)
{
    const char* defaultPwd = "1234";
    strcpy((char*)buffer, defaultPwd);
    return strlen(defaultPwd);
}

static BusAuthPwdFunc busAuthPwdFunc = DefaultBusAuthPwdFunc;

void SetBusAuthPwdCallback(BusAuthPwdFunc callback)
{
    busAuthPwdFunc = callback;
}

static AJ_AuthResult AnonMechAdvance(const char* inStr, char* outStr, uint32_t outLen)
{
    return AJ_AUTH_STATUS_SUCCESS;
}

static AJ_Status AnonMechInit(uint8_t role, AJ_AuthPwdFunc pwdFunc)
{
    return AJ_OK;
}

static const AJ_AuthMechanism authAnonymous = {
    AnonMechInit,
    AnonMechAdvance,
    AnonMechAdvance,
    NULL,
    "ANONYMOUS"
};

/*
 * Authentication mechanisms in order of preference
 */
static const AJ_AuthMechanism* mechList[] = { &AJ_AuthPin, &authAnonymous, NULL };

static AJ_Status SendHello(AJ_BusAttachment* bus)
{
    AJ_Status status;
    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, AJ_METHOD_HELLO, AJ_DBusDestination, 0, AJ_FLAG_ALLOW_REMOTE_MSG, 5000);
    if (status == AJ_OK) {
        status = AJ_DeliverMsg(&msg);
    }
    return status;
}

static AJ_Status AuthAdvance(AJ_SASL_Context* context, AJ_IOBuffer* rxBuf, AJ_IOBuffer* txBuf)
{
    AJ_Status status = AJ_OK;

    if (context->state != AJ_SASL_SEND_AUTH_REQ) {
        /*
         * All the authentication messages end in a CR/LF so read until we get a newline
         */
        while ((AJ_IO_BUF_AVAIL(rxBuf) == 0) || (*(rxBuf->writePtr - 1) != '\n')) {
            status = rxBuf->recv(rxBuf, AJ_IO_BUF_SPACE(rxBuf), 3500);
            if (status != AJ_OK) {
                break;
            }
        }
    }
    if (status == AJ_OK) {
        uint32_t inLen = AJ_IO_BUF_AVAIL(rxBuf);
        *rxBuf->writePtr = '\0';
        status = AJ_SASL_Advance(context, (char*)rxBuf->readPtr, (char*)txBuf->writePtr, AJ_IO_BUF_SPACE(txBuf));
        if (status == AJ_OK) {
            rxBuf->readPtr += inLen;
            txBuf->writePtr += strlen((char*)txBuf->writePtr);
            status = txBuf->send(txBuf);
        }
    }
    return status;
}

AJ_Status AJ_Connect(AJ_BusAttachment* bus, const char* serviceName, uint32_t timeout)
{
    AJ_Status status;
    AJ_SASL_Context sasl;
    AJ_Service service;
    /*
     * Clear the bus struct
     */
    memset(bus, 0, sizeof(AJ_BusAttachment));
    /*
     * Clear stale name->GUID mappings
     */
    AJ_GUID_ClearNameMap();
    /*
     * Host-specific network bring-up procedure. This includes establishing a connection to the
     * network and initializing the I/O buffers.
     */
    status = AJ_Net_Up();
    if (status != AJ_OK) {
        return status;
    }
    /*
     * Discover a daemon or service to connect to
     */
    if (!serviceName) {
        serviceName = daemonService;
    }
#if defined _WIN32 || __linux__
    service.ipv4port = 9955;
    service.ipv4 = 0x0100007F;
    service.addrTypes = AJ_ADDR_IPV4;
#elif defined ARDUINO
    service.ipv4port = 9955;
    service.ipv4 = 0x6501A8C0; // 192.168.1.101
    service.addrTypes = AJ_ADDR_IPV4;
    status = AJ_Discover(serviceName, &service, timeout);
    if (status != AJ_OK) {
        goto ExitConnect;
    }
#else
    status = AJ_Discover(serviceName, &service, timeout);
    if (status != AJ_OK) {
        goto ExitConnect;
    }
#endif
    status = AJ_Net_Connect(&bus->sock, service.ipv4port, service.addrTypes & AJ_ADDR_IPV4, &service.ipv4);
    if (status != AJ_OK) {
        goto ExitConnect;
    }
    /*
     * Send initial NUL byte
     */
    bus->sock.tx.writePtr[0] = 0;
    bus->sock.tx.writePtr += 1;
    status = bus->sock.tx.send(&bus->sock.tx);
    if (status != AJ_OK) {
        goto ExitConnect;
    }
    AJ_SASL_InitContext(&sasl, mechList, AJ_AUTH_RESPONDER, busAuthPwdFunc);
    while (TRUE) {
        status = AuthAdvance(&sasl, &bus->sock.rx, &bus->sock.tx);
        if ((status != AJ_OK) || (sasl.state == AJ_SASL_FAILED)) {
            break;
        }
        if (sasl.state == AJ_SASL_AUTHENTICATED) {
            status = SendHello(bus);
            break;
        }
    }
    if (status == AJ_OK) {
        AJ_Message helloResponse;
        status = AJ_UnmarshalMsg(bus, &helloResponse, 5000);
        if (status == AJ_OK) {
            /*
             * The only error we might get is a timeout
             */
            if (helloResponse.hdr->msgType == AJ_MSG_ERROR) {
                status = AJ_ERR_TIMEOUT;
            } else {
                AJ_Arg arg;
                status = AJ_UnmarshalArg(&helloResponse, &arg);
                if (status == AJ_OK) {
                    if (arg.len >= (sizeof(bus->uniqueName) - 1)) {
                        status = AJ_ERR_RESOURCES;
                    } else {
                        memcpy(bus->uniqueName, arg.val.v_string, arg.len);
                        bus->uniqueName[arg.len] = '\0';
                    }
                }
            }
            AJ_CloseMsg(&helloResponse);
        }
    }

ExitConnect:

    if (status != AJ_OK) {
        AJ_Printf("AllJoyn connect failed %d\n", status);
        AJ_Disconnect(bus);
    }
    return status;
}

void AJ_Disconnect(AJ_BusAttachment* bus)
{
    /*
     * We won't be getting any more method replies.
     */
    AJ_ReleaseReplyContexts();
    /*
     * Disconnect the network closing sockets etc.
     */
    AJ_Net_Disconnect(&bus->sock);
    /*
     * Host-specific network shutdown procedure
     */
    AJ_Net_Down();
}
