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

#include <stdio.h>
#include <time.h>

#include "alljoyn.h"

#define CONNECT_ATTEMPTS   10
static const char ServiceName[] = "org.alljoyn.bus.samples.secure";
static const char InterfaceName[] = "org.alljoyn.bus.samples.secure.SecureInterface";
static const char ServicePath[] = "/SecureService";
static const uint16_t ServicePort = 42;

/**
 * The interface name followed by the method signatures.
 *
 * See also .\inc\aj_introspect.h
 */
static const char* secureInterface[] = {
    InterfaceName,
    "?Ping inStr<s outStr>s",  /* Method at index 0. */
    NULL
};

/**
 * A NULL terminated collection of all interfaces.
 */
static const AJ_InterfaceDescription secureInterfaces[] = {
    secureInterface,
    NULL
};

/**
 * Objects implemented by the application. The first member in the AJ_Object structure is the path.
 * The second is the collection of all interfaces at that path.
 */
static const AJ_Object AppObjects[] = {
    { ServicePath, secureInterfaces },
    { NULL }
};

/*
 * The value of the arguments are the indices of the
 * object path in AppObjects (above), interface in sampleInterfaces (above), and
 * member indices in the interface.
 * The 'ping' index is 0 because the first entry in sampleInterface is the interface name.
 * This makes the first index (index 0 of the methods) the second string in
 * secureInterfaces[].
 *
 * See also .\inc\aj_introspect.h
 */
#define BASIC_SERVICE_PING AJ_APP_MESSAGE_ID(0, 0, 0)

static AJ_Status AppHandlePing(AJ_Message* msg)
{
    AJ_Status status;
    AJ_Message reply;
    AJ_Arg arg;

    status = AJ_UnmarshalArg(msg, &arg);

    if (AJ_OK == status) {

        if (arg.typeId == AJ_ARG_STRING) {
            printf("Received ping request '%s'.\n", arg.val.v_string);
        } else {
            printf("Unexpected arg type '%d' in ping request.\n", arg.typeId);
        }

        status = AJ_MarshalReplyMsg(msg, &reply);

        if (AJ_OK == status) {
            /*
             * Just return the arg we received
             */
            status = AJ_MarshalArg(&reply, &arg);

            if (AJ_OK == status) {
                status = AJ_DeliverMsg(&reply);
            }
        }
    }

    return status;
}

static uint32_t PasswordCallback(uint8_t* buffer, uint32_t bufLen)
{
    int pin;
    char pinStr[16];
    size_t pinLength = 0;

    /* seed the random number */
    srand((unsigned int) time(NULL));
    pin = 1000 * (rand() % 1000) + (rand() % 1000);

    sprintf(pinStr, "%06d", pin);
    printf("One Time Password : '%s'.\n", pinStr);
    fflush(stdout);

    pinLength = strlen(pinStr);
    memcpy(buffer, pinStr, pinLength);

    printf("Need password of '%s' length %d.\n", buffer, pinLength);

    return (uint32_t)pinLength;
}

/* All times are expressed in milliseconds. */
#define CONNECT_TIMEOUT     (1000 * 60)
#define UNMARSHAL_TIMEOUT   (1000 * 5)
#define SLEEP_TIME          (1000 * 2)

int main(void)
{
    AJ_Status status = AJ_OK;
    AJ_BusAttachment bus;
    uint8_t connected = FALSE;
    uint32_t sessionId = 0;

    /* One time initialization before calling any other AllJoyn APIs. */
    AJ_Initialize();

    /* This is for debug purposes and is optional. */
    AJ_PrintXML(AppObjects);
    AJ_RegisterObjects(AppObjects, NULL);

    while (TRUE) {
        AJ_Message msg;

        if (!connected) {
            status = AJ_StartService(&bus,
                                     "org.alljoyn",
                                     CONNECT_TIMEOUT,
                                     ServicePort,
                                     ServiceName,
                                     AJ_NAME_REQ_DO_NOT_QUEUE);

            if (status != AJ_OK) {
                continue;
            }

            printf("StartService returned %d, session_id=%u\n", status, sessionId);
            connected = TRUE;

            AJ_BusSetPasswordCallback(&bus, PasswordCallback);
        }

        status = AJ_UnmarshalMsg(&bus, &msg, UNMARSHAL_TIMEOUT);

        if (AJ_ERR_TIMEOUT == status) {
            continue;
        }

        if (AJ_OK == status) {
            switch (msg.msgId) {
            case AJ_METHOD_ACCEPT_SESSION:
                {
                    uint16_t port;
                    char* joiner;
                    AJ_UnmarshalArgs(&msg, "qus", &port, &sessionId, &joiner);
                    status = AJ_BusReplyAcceptSession(&msg, TRUE);
                    printf("Accepted session session_id=%u joiner=%s\n", sessionId, joiner);
                }
                break;

            case BASIC_SERVICE_PING:
                status = AppHandlePing(&msg);
                break;

            case AJ_SIGNAL_SESSION_LOST:
                /* Force a disconnect. */
                status = AJ_ERR_READ;
                break;

            default:
                /* Pass to the built-in handlers. */
                status = AJ_BusHandleBusMessage(&msg);
                break;
            }
        }

        /* Messages MUST be discarded to free resources. */
        AJ_CloseMsg(&msg);

        if (status == AJ_ERR_READ) {
            printf("AllJoyn disconnect.\n");
            AJ_Disconnect(&bus);
            connected = FALSE;

            /* Sleep a little while before trying to reconnect. */
            AJ_Sleep(SLEEP_TIME);
        }
    }

    printf("Secure service exiting with status 0x%04x.\n", status);

    return status;
}
