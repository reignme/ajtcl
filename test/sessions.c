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

#include <stdio.h>
#include <alljoyn.h>

#define CONNECT_TIMEOUT    (1000ul * 200)
#define UNMARSHAL_TIMEOUT  (1000ul * 5)
#define METHOD_TIMEOUT     (1000ul * 3)

/// globals
AJ_Status status = AJ_OK;
AJ_BusAttachment bus;
uint8_t connected = FALSE;
uint32_t g_sessionId = 0ul;
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

AJ_Status AppSendChatSignal(AJ_BusAttachment* bus, uint32_t sessionId, const char* chatString, uint8_t flags, uint32_t ttl)
{
    AJ_Status status = AJ_OK;
    AJ_Message msg;

    status = AJ_MarshalSignal(bus, &msg, APP_CHAT_SIGNAL, NULL, sessionId, flags, ttl);
    if (status == AJ_OK) {
        status = AJ_MarshalArgs(&msg, "s", chatString);
    }

    if (status == AJ_OK) {
        status = AJ_DeliverMsg(&msg);
    }
    printf("TX chat: %s\n", chatString);
    return status;
}

AJ_Status AppHandleChatSignal(AJ_Message* msg)
{
    AJ_Status status = AJ_OK;
    char* chatString;

    AJ_UnmarshalArgs(msg, "s", &chatString);
    printf("RX chat from %s[%u]: %s\n", msg->sender, msg->sessionId, chatString);
    return status;
}

AJ_Status AppSetSignalRule(AJ_BusAttachment* bus, const char* ruleString, uint8_t rule)
{
    AJ_Status status;
    AJ_Message msg;
    uint32_t msgId = (rule == AJ_BUS_SIGNAL_ALLOW) ? AJ_METHOD_ADD_MATCH : AJ_METHOD_REMOVE_MATCH;

    status = AJ_MarshalMethodCall(bus, &msg, msgId, AJ_DBusDestination, 0, 0, METHOD_TIMEOUT);
    if (status == AJ_OK) {
        uint32_t sz = 0;
        uint8_t nul = 0;
        sz = (uint32_t)strlen(ruleString);
        status = AJ_DeliverMsgPartial(&msg, sz + 5);
        AJ_MarshalRaw(&msg, &sz, 4);
        AJ_MarshalRaw(&msg, ruleString, strlen(ruleString));
        AJ_MarshalRaw(&msg, &nul, 1);
    }
    if (status == AJ_OK) {
        status = AJ_DeliverMsg(&msg);
    }
    return status;
}


int AJ_Main()
{
    // you're connected now, so print out the data:
    printf("You're connected to the network\n");
    AJ_Initialize();
    AJ_PrintXML(AppObjects);
    AJ_RegisterObjects(AppObjects, NULL);

    while (!connected) {
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

    while (TRUE) {
        // check for serial input and dispatch if needed.
        char buf[1024];
        AJ_Message msg;
#ifdef ARDUINO
        if (Serial.available() > 0) {
            int countBytesRead;
            // read the incoming bytes until a newline character:
            countBytesRead = Serial.readBytesUntil('\n', buf, sizeof(buf));
            buf[countBytesRead] = '\0';
#else
        // read a line
        if (AJ_GetLine(buf, 1024, stdin) != NULL) {
#endif
            char*command;
            printf(">~~~%s\n", buf);
            command = strtok(buf, " \r\n");
            if (0 == strcmp("find", command)) {
                char* name = strtok(NULL, "\r\n");
                status = AJ_BusFindAdvertisedName(&bus, name, AJ_BUS_START_FINDING);
            } else if (0 == strcmp("cancelfind", command)) {
                char* name = strtok(NULL, "\r\n");
                status = AJ_BusFindAdvertisedName(&bus, name, AJ_BUS_STOP_FINDING);
            } else if (0 == strcmp("requestname", command)) {
                char* name = strtok(NULL, "\r\n");
                status = AJ_BusRequestName(&bus, name, AJ_NAME_REQ_DO_NOT_QUEUE);
            } else if (0 == strcmp("releasename", command)) {
                char* name = strtok(NULL, "\r\n");
                status = AJ_BusReleaseName(&bus, name);
            } else if (0 == strcmp("advertise", command)) {
                char* name = strtok(NULL, " \r\n");
                char* transports = strtok(NULL, "\r\n");
                uint16_t transportflag = 0xFFFF;
                status = AJ_BusAdvertiseName(&bus, name, transportflag, AJ_BUS_START_ADVERTISING);
            } else if (0 == strcmp("canceladvertise", command)) {
                char* name = strtok(NULL, " \r\n");
                char* transports = strtok(NULL, "\r\n");
                uint16_t transportflag = 0xFFFF;
                status = AJ_BusAdvertiseName(&bus, name, transportflag, AJ_BUS_STOP_ADVERTISING);
            } else if (0 == strcmp("addmatch", command)) {
                char* ruleString = strtok(NULL, "\r\n");
                status = AppSetSignalRule(&bus, ruleString, AJ_BUS_SIGNAL_ALLOW);
            } else if (0 == strcmp("removematch", command)) {
                char* ruleString = strtok(NULL, "\r\n");
                status = AppSetSignalRule(&bus, ruleString, AJ_BUS_SIGNAL_DENY);
            } else if (0 == strcmp("schat", command)) {
                char* chatMsg = strtok(NULL, "\r\n");
                status = AppSendChatSignal(&bus, 0, chatMsg, ALLJOYN_FLAG_SESSIONLESS, 0);
            } else if (0 == strcmp("chat", command)) {
                char* sessionIdString = strtok(NULL, " \r\n");
                char*flagsString;
                char*ttlString;
                char*chatMsg;
                uint8_t flags = 0;
                uint32_t ttl = 0;

                uint32_t session = g_sessionId;
                if (sessionIdString != NULL) {
                    if (sessionIdString[0] != '#') {
                        session = atol(sessionIdString);
                    }
                }

                flagsString = strtok(NULL, " \r\n");
                if (flagsString != NULL) {
                    flags = atoi(flagsString);
                }

                ttlString = strtok(NULL, " \r\n");
                if (ttlString != NULL) {
                    ttl = atoi(ttlString);
                }

                chatMsg = strtok(NULL, "\r\n");
                status = AppSendChatSignal(&bus, session, chatMsg, flags, ttl);
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
                uint16_t port;
                char* joiner;
                printf("Accepting...\n");
                AJ_UnmarshalArgs(&msg, "qus", &port, &g_sessionId, &joiner);
                status = AJ_BusReplyAcceptSession(&msg, TRUE);
                if (status == AJ_OK) {
                    printf("Accepted session session_id=%u joiner=%s\n", g_sessionId, joiner);
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

#ifdef AJ_MAIN
int main()
{
    return AJ_Main();
}
#endif
