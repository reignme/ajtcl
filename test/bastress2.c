/**
 * @file
 */
/******************************************************************************
 * Copyright 2013, Qualcomm Innovation Center, Inc.
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

#include <stdint.h>
#include <stddef.h>

#include "due_bastress2.h"

#include <alljoyn.h>

#define CONNECT_TIMEOUT    (1000ul * 200)
#define UNMARSHAL_TIMEOUT  (1000ul * 5)
#define METHOD_TIMEOUT     (1000ul * 3)

/// globals
AJ_Status status = AJ_OK;
AJ_BusAttachment bus;
uint8_t connected = FALSE;
uint32_t sessionId = 0ul;
AJ_Status authStatus = AJ_ERR_NULL;


static const char DaemonName[] = "org.alljoyn";
static const char ServiceName[] = "org.alljoyn.Bus.test.bastress";
static const uint16_t ServicePort = 25;
static uint32_t authenticate = TRUE;

static const char* testInterface[] = {
    "org.alljoyn.Bus.test.bastress",
    "?cat inStr1<s inStr2<s outStr>s",
    NULL
};


static const AJ_InterfaceDescription testInterfaces[] = {
    testInterface,
    NULL
};

/**
 * Objects implemented by the application
 */
static const AJ_Object AppObjects[] = {
    { "/org/alljoyn/alljoyn_test", testInterfaces },
    { NULL }
};

#define APP_MY_CAT  AJ_APP_MESSAGE_ID(0, 0, 0)


/*
 * Let the application do some work
 */
void AppDoWork()
{
    printf("AppDoWork\n");
}


static const char PWD[] = "1234";

static uint32_t PasswordCallback(uint8_t* buffer, uint32_t bufLen)
{
    memcpy(buffer, PWD, sizeof(PWD));
    return sizeof(PWD) - 1;
}

void AuthCallback(const void* context, AJ_Status status)
{
    *((AJ_Status*)context) = status;
}


AJ_Status AppHandleCat(AJ_Message* msg)
{
    AJ_Status status = AJ_OK;
    AJ_Message reply;
    AJ_Arg arg;
    char* partA;
    char* partB;
    char* totalString;
    printf("%s:%d:%s %d\n", __FILE__, __LINE__, __FUNCTION__, 0);

    AJ_UnmarshalArgs(msg, "ss", &partA, &partB);

    totalString = (char*) malloc(strlen(partA) + strlen(partB));
    strcpy(totalString, partA);
    strcat(totalString, partB);

    AJ_MarshalReplyMsg(msg, &reply);
    AJ_MarshalArgs(&reply, "s", totalString);

    status = AJ_DeliverMsg(&reply);
    free(totalString);
    return status;
}

int aj_main_loop()
{
    // you're connected now, so print out the data:
    printf("You're connected to the network\n");
    AJ_Initialize();
    AJ_PrintXML(AppObjects);
    AJ_RegisterObjects(AppObjects, NULL);

    while (TRUE) {
        AJ_Message msg;

        if (!connected) {
            status = AJ_StartService(&bus, DaemonName, CONNECT_TIMEOUT, ServicePort, ServiceName, AJ_NAME_REQ_DO_NOT_QUEUE);
            if (status == AJ_OK) {
                printf("StartService returned %d\n", status);
                connected = TRUE;
                if (authenticate) {
                    AJ_BusSetPasswordCallback(&bus, PasswordCallback);
                } else {
                    authStatus = AJ_OK;
                }
            } else {
                printf("StartClient returned %d\n", status);
                continue;
            }
        }

        status = AJ_UnmarshalMsg(&bus, &msg, UNMARSHAL_TIMEOUT);
        if (status != AJ_OK) {
            if (status == AJ_ERR_TIMEOUT) {
                AppDoWork();
            }
            continue;
        }

        switch (msg.msgId) {

        case AJ_METHOD_ACCEPT_SESSION:
            {
                printf("Accepting...\n");
                uint16_t port;
                char* joiner;
                AJ_UnmarshalArgs(&msg, "qus", &port, &sessionId, &joiner);
                status = AJ_BusReplyAcceptSession(&msg, TRUE);
                if (status == AJ_OK) {
                    printf("Accepted session session_id=%u joiner=%s\n", sessionId, joiner);
                } else {
                    printf("AJ_BusReplyAcceptSession: error %d\n", status);
                }
            }
            break;

        case APP_MY_CAT:
            status = AppHandleCat(&msg);
            break;

        case AJ_SIGNAL_SESSION_LOST:
            /*
             * don't force a disconnect, be ready to accept another session
             */
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
            connected = FALSE;
            /*
             * Sleep a little while before trying to reconnect
             */
            AJ_Sleep(10 * 1000);
        }
    }

    return 0;
}
