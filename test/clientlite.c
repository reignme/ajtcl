/*
 * clientlite.c
 */

/******************************************************************************
 * Copyright 2012, Qualcomm Innovation Center, Inc.
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

#include <stdio.h>
#include <assert.h>

#include "alljoyn.h"
#include "aj_debug.h"
#include "aj_crypto.h"

static const char ServiceName[] = "org.alljoyn.svclite";
static const uint16_t ServicePort = 24;

static int32_t propVal = 123456;

/*
 * Set to request authentication
 */
static uint8_t authFlag = AJ_FLAG_ENCRYPTED;

static const char* testInterface[] = {
    "org.alljoyn.alljoyn_test",
    "?my_ping inStr<s outStr>s",
    NULL
};


static const char* testValuesInterface[] = {
    "org.alljoyn.alljoyn_test.values",
    "@int_val=i",
    NULL
};

static const AJ_InterfaceDescription testInterfaces[] = {
    AJ_PropertiesIface,
    testInterface,
    testValuesInterface,
    NULL
};

/**
 * Objects implemented by the application
 */
static const AJ_Object ProxyObjects[] = {
    { "/org/alljoyn/alljoyn_test", testInterfaces },
    { NULL }
};

#define PRX_GET_PROP  AJ_PRX_MESSAGE_ID(0, 0, AJ_PROP_GET)
#define PRX_SET_PROP  AJ_PRX_MESSAGE_ID(0, 0, AJ_PROP_SET)
#define PRX_MY_PING   AJ_PRX_MESSAGE_ID(0, 1, 0)
#define PRX_GET_INT   AJ_PRX_PROPERTY_ID(0, 2, 0)
#define PRX_SET_INT   AJ_PRX_PROPERTY_ID(0, 2, 0)

/*
 * Let the application do some work
 */
static void AppDoWork()
{
}

static const char PWD[] = "ABCDEFGH";

static uint32_t PasswordCallback(uint8_t* buffer, uint32_t bufLen)
{
    memcpy(buffer, PWD, sizeof(PWD));
    return sizeof(PWD) - 1;
}

#define CONNECT_TIMEOUT    (1000 * 200)
#define UNMARSHAL_TIMEOUT  (1000 * 5)
#define METHOD_TIMEOUT     (100 * 10)


AJ_Status SendPing(AJ_BusAttachment* bus, uint32_t sessionId, unsigned int num)
{
    AJ_Status status;
    AJ_Message msg;
    char buf[80];

    sprintf(buf, "Ping String %u", num);

    status = AJ_MarshalMethodCall(bus, &msg, PRX_MY_PING, ServiceName, sessionId, authFlag, METHOD_TIMEOUT);
    if (status == AJ_OK) {
        status = AJ_MarshalArgs(&msg, "s", buf);
    }
    if (status == AJ_OK) {
        status = AJ_DeliverMsg(&msg);
    }
    return status;
}

AJ_Status SendGetProp(AJ_BusAttachment* bus, uint32_t sessionId)
{
    AJ_Status status;
    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, PRX_GET_PROP, ServiceName, sessionId, authFlag, METHOD_TIMEOUT);
    if (status == AJ_OK) {
        AJ_MarshalPropertyArgs(&msg, PRX_GET_INT);
        status = AJ_DeliverMsg(&msg);
    }
    return status;
}

AJ_Status SendSetProp(AJ_BusAttachment* bus, uint32_t sessionId, int val)
{
    AJ_Status status;
    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, PRX_SET_PROP, ServiceName, sessionId, 0, METHOD_TIMEOUT);
    if (status == AJ_OK) {
        status = AJ_MarshalPropertyArgs(&msg, PRX_SET_INT);

        if (status == AJ_OK) {
            status = AJ_MarshalArgs(&msg, "i", val);
        } else {
            printf(">>>>>>>>In SendSetProp() AJ_MarshalPropertyArgs() returned status = 0x%04x\n", status);
        }

        if (status == AJ_OK) {
            status = AJ_DeliverMsg(&msg);
        } else {
            printf(">>>>>>>>In SendSetProp() AJ_MarshalArgs() returned status = 0x%04x\n", status);
        }
    }

    return status;
}

void AuthCallback(const void* context, AJ_Status status)
{
    *((AJ_Status*)context) = status;
}

int AJ_Main(void)
{
    AJ_Status status = AJ_OK;
    AJ_BusAttachment bus;
    uint8_t connected = FALSE;
    uint32_t sessionId = 0;
    AJ_Status authStatus = AJ_ERR_NULL;

    /*
     * One time initialization before calling any other AllJoyn APIs
     */
    AJ_Initialize();

    AJ_PrintXML(ProxyObjects);
    AJ_RegisterObjects(NULL, ProxyObjects);

    while (TRUE) {
        AJ_Message msg;

        if (!connected) {
            status = AJ_StartClient(&bus, "org.alljoyn", CONNECT_TIMEOUT, ServiceName, ServicePort, &sessionId);
            if (status == AJ_OK) {
                printf("StartClient returned %d, sessionId=%u\n", status, sessionId);
                connected = TRUE;
                if (authFlag) {
                    AJ_BusSetPasswordCallback(&bus, PasswordCallback);
                    status = AJ_BusAuthenticatePeer(&bus, ServiceName, AuthCallback, &authStatus);
                    if (status != AJ_OK) {
                        printf("AJ_BusAuthenticatePeer returned %d\n", status);
                    }
                } else {
                    authStatus = AJ_OK;
                }
            } else {
                printf("StartClient returned %d\n", status);
                break;
            }
        }

        if (authStatus != AJ_ERR_NULL) {
            if (authStatus != AJ_OK) {
                AJ_Disconnect(&bus);
                break;
            }
            authStatus = AJ_ERR_NULL;
            SendPing(&bus, sessionId, 1);
        }

        status = AJ_UnmarshalMsg(&bus, &msg, UNMARSHAL_TIMEOUT);
        if (status != AJ_OK) {
            if (status == AJ_ERR_TIMEOUT) {
                AppDoWork();
            }
            continue;
        }

        switch (msg.msgId) {

        case AJ_REPLY_ID(PRX_MY_PING):
            {
                AJ_Arg arg;
                AJ_UnmarshalArg(&msg, &arg);
                status = SendGetProp(&bus, sessionId);
            }
            break;

        case AJ_REPLY_ID(PRX_GET_PROP):
            {
                const char* sig;
                status = AJ_UnmarshalVariant(&msg, &sig);
                if (status == AJ_OK) {
                    int32_t val;
                    status = AJ_UnmarshalArgs(&msg, sig, &val);
                    printf("Get prop reply %d\n", val);

                    if (status == AJ_OK) {
                        status = SendSetProp(&bus, sessionId, -val);
                    }
                }
            }
            break;

        case AJ_REPLY_ID(PRX_SET_PROP):
            printf("Set prop reply\n");
            break;

        case AJ_SIGNAL_SESSION_LOST:
            /*
             * Force a disconnect
             */
            status = AJ_ERR_READ;
            break;

        default:
            /*
             * Pass to the built-in handlers
             */
            status = AJ_BusHandleBusMessage(&msg);
            break;
        }
        /*
         * Messages must be closed to free resources
         */
        AJ_CloseMsg(&msg);

        if (status == AJ_ERR_READ) {
            printf("AllJoyn disconnect\n");
            AJ_Disconnect(&bus);
            exit(0);
        }
    }
    printf("clientlite EXIT %d\n", status);

    return status;
}

#ifdef AJ_MAIN
int main()
{
    authFlag = AJ_FLAG_ENCRYPTED;
    return AJ_Main();
}
#endif
