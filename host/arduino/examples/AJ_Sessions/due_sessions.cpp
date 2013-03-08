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

#include "due_sessions.h"

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
static const char ServiceName[] = "org.alljoyn.bus.test.sessions";
static const uint16_t ServicePort = 25;
static uint32_t authenticate = TRUE;

static const char* testInterface[] = {
    "org.alljoyn.bus.test.sessions",
    "!Chat >s",
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
    { "/sessions", testInterfaces },
    { NULL }
};

#define APP_CHAT_SIGNAL  AJ_APP_MESSAGE_ID(0, 0, 0)


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


AJ_Status AppHandleChatSignal(AJ_Message* msg)
{
    AJ_Status status = AJ_OK;
    AJ_Message reply;
    AJ_Arg arg;
    char* chatString;

    AJ_UnmarshalArgs(msg, "s", &chatString);
    printf("RX chat from %s[%u]: %s\n", msg->sender, msg->sessionId, chatString);
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
        // check for serial input and dispatch if needed.
        if (Serial.available() > 0) {
            int countBytesRead;
            char buf[1024];
            // read the incoming bytes until a newline character:
            countBytesRead = Serial.readBytesUntil('\n', buf, sizeof(buf));
            buf[countBytesRead] = '\0';
            printf("~~~>%s\n", buf);
            char* command = strtok(buf, " \r\n");
            if (0 == strcmp("find", command)) {
                char* name = strtok(NULL, " \r\n");
                status = AJ_BusFindAdvertisedName(&bus, name, AJ_BUS_START_FINDING);
            } else if (0 == strcmp("cancelfind", command)) {
                char* name = strtok(NULL, " \r\n");
                status = AJ_BusFindAdvertisedName(&bus, name, AJ_BUS_STOP_FINDING);
            } else if (0 == strcmp("requestname", command)) {
                char* name = strtok(NULL, " \r\n");
                status = AJ_BusRequestName(&bus, name, AJ_NAME_REQ_DO_NOT_QUEUE);
            } else if (0 == strcmp("releasename", command)) {
                char* name = strtok(NULL, " \r\n");
                status = AJ_BusReleaseName(&bus, name);
            } else if (0 == strcmp("advertise", command)) {
                char* name = strtok(NULL, " \r\n");
                char* transports = strtok(NULL, " \r\n");
                uint16_t transportflag = 0xFFFF;
                if (transports != NULL) {
                  transportflag = word(transportflag);
                }
                status = AJ_BusAdvertiseName(&bus, name, transportflag ,AJ_BUS_START_ADVERTISING);
            } else if (0 == strcmp("canceladvertise", command)) {
                char* name = strtok(NULL, " \r\n");
                char* transports = strtok(NULL, " \r\n");
                uint16_t transportflag = 0xFFFF;
                if (transports != NULL) {
                  transportflag = word(transportflag);
                }
                status = AJ_BusAdvertiseName(&bus, name, transportflag,AJ_BUS_STOP_ADVERTISING);
            }

        }



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

        case APP_CHAT_SIGNAL:
            status = AppHandleChatSignal(&msg);
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