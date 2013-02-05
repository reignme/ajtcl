/*
 * @file
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

#include "alljoyn.h"

static const char ServiceName[] = "org.alljoyn.Bus.sample";
static const char ServicePath[] = "/sample";
static const uint16_t ServicePort = 25;

/**
 * The interface name followed by the method signatures.
 *
 * See also .\inc\aj_introspect.h
 */
static const char* sampleInterface[] = {
    "org.alljoyn.Bus.sample",   /* The first entry is the interface name. */
    "?Dummy foo<i",             /* This is just a dummy entry at index 0 for illustration purposes. */
    "?Dummy2 fee<i",            /* This is just a dummy entry at index 1 for illustration purposes. */
    "?cat inStr1<s inStr2<s outStr>s", /* Method at index 2. */
    NULL
};

/**
 * A NULL terminated collection of all interfaces.
 */
static const AJ_InterfaceDescription sampleInterfaces[] = {
    sampleInterface,
    NULL
};

/**
 * Objects implemented by the application. The first member in the AJ_Object structure is the path.
 * The second is the collection of all interfaces at that path.
 */
static const AJ_Object AppObjects[] = {
    { ServicePath, sampleInterfaces },
    { NULL }
};

/*
 * The value of the arguments are the indices of the object path in AppObjects (above),
 * interface in sampleInterfaces (above), and member indices in the interface.
 * The 'cat' index is 2. The reason for this is as follows: The first entry in sampleInterface
 * is the interface name. This makes the first index (index 0 of the methods) the second string in
 * sampleInterface[]. The two dummy entries are indices 0 and 1. The index of the method we
 * implement for basic_client, 'cat', is 2 which is the fourth string in the array of strings
 * sampleInterface[].
 *
 * See also .\inc\aj_introspect.h
 */
#define BASIC_CLIENT_CAT AJ_PRX_MESSAGE_ID(0, 0, 2)

#define CONNECT_TIMEOUT    (1000 * 60)
#define UNMARSHAL_TIMEOUT  (1000 * 5)
#define METHOD_TIMEOUT     (100 * 10)

void MakeMethodCall(AJ_BusAttachment* bus, uint32_t sessionId)
{
    AJ_Status status;
    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, BASIC_CLIENT_CAT, ServiceName, sessionId, 0, METHOD_TIMEOUT);

    if (status == AJ_OK) {
        status = AJ_MarshalArgs(&msg, "ss", "Hello ", "World!");
    }

    if (status == AJ_OK) {
        status = AJ_DeliverMsg(&msg);
    }

    printf("MakeMethodCall() resulted in a status of 0x%04x.\n", status);
}

int main(void)
{
    AJ_Status status = AJ_OK;
    AJ_BusAttachment bus;
    uint8_t connected = FALSE;
    uint8_t done = FALSE;
    uint32_t sessionId = 0;

    /*
     * One time initialization before calling any other AllJoyn APIs
     */
    AJ_Initialize();
    AJ_PrintXML(AppObjects);
    AJ_RegisterObjects(NULL, AppObjects);

    while (!done) {
        AJ_Message msg;

        if (!connected) {
            status = AJ_StartClient(&bus,
                                    "org.alljoyn",
                                    CONNECT_TIMEOUT,
                                    ServiceName,
                                    ServicePort,
                                    &sessionId);

            if (status == AJ_OK) {
                printf("StartClient returned %d, sessionId=%u.\n", status, sessionId);
                connected = TRUE;
                MakeMethodCall(&bus, sessionId);
            } else {
                printf("StartClient returned 0x%04x.\n", status);
                break;
            }
        }

        status = AJ_UnmarshalMsg(&bus, &msg, UNMARSHAL_TIMEOUT);

        if (AJ_ERR_TIMEOUT == status) {
            continue;
        }

        if (AJ_OK == status) {
            switch (msg.msgId) {
            case AJ_REPLY_ID(BASIC_CLIENT_CAT):
                {
                    AJ_Arg arg;

                    status = AJ_UnmarshalArg(&msg, &arg);

                    if (AJ_OK == status) {
                        printf("'%s.%s' (path='%s') returned '%s'.\n", ServiceName, "cat",
                               ServicePath, arg.val.v_string);
                        done = TRUE;
                    } else {
                        printf("AJ_UnmarshalArg() returned status %d.\n", status);
                        /* Try again because of the failure. */
                        MakeMethodCall(&bus, sessionId);
                    }
                }
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
            exit(0);
        }
    }

    printf("Basic client exiting with status %d.\n", status);

    return status;
}
