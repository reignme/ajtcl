/**
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
#include "aj_debug.h"
#include "aj_crypto.h"
#include "aj_helper.h"

static const char ServiceName[] = "org.alljoyn.svclite";
static const uint16_t ServicePort = 24;

/*
 * An application property to SET or GET
 */
static int32_t propVal = 123456;

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
static const AJ_Object AppObjects[] = {
    { "/org/alljoyn/alljoyn_test", testInterfaces },
    { NULL }
};

/*
 * Message identifiers for the method calls this application implements
 */
#define APP_GET_PROP  AJ_APP_MESSAGE_ID(0, 0, AJ_PROP_GET)
#define APP_SET_PROP  AJ_APP_MESSAGE_ID(0, 0, AJ_PROP_SET)
#define APP_MY_PING   AJ_APP_MESSAGE_ID(0, 1, 0)

/*
 * Property identifiers for the properies this application implements
 */
#define APP_INT_VAL_PROP AJ_APP_PROPERTY_ID(0, 2, 0)

/*
 * Let the application do some work
 */
static void AppDoWork()
{
    /*
     * This function is called if there are no messages to unmarshal
     */
  printf("do work\n");
}

static const char PWD[] = "ABCDEFGH";

static uint32_t PasswordCallback(uint8_t* buffer, uint32_t bufLen)
{
    memcpy(buffer, PWD, sizeof(PWD));
    return sizeof(PWD) - 1;
}

static AJ_Status AppHandlePing(AJ_Message* msg)
{
    AJ_Message reply;
    AJ_Arg arg;

    AJ_UnmarshalArg(msg, &arg);
    AJ_MarshalReplyMsg(msg, &reply);
    /*
     * Just return the arg we received
     */
    AJ_MarshalArg(&reply, &arg);
    return AJ_DeliverMsg(&reply);
}

/*
 * Handles a property GET request so marshals the property value to return
 */
static AJ_Status PropGetHandler(AJ_Message* replyMsg, uint32_t propId, void* context)
{
    if (propId == APP_INT_VAL_PROP) {
        return AJ_MarshalArgs(replyMsg, "i", propVal);
    } else {
        return AJ_ERR_UNEXPECTED;
    }
}

/*
 * Handles a property SET request so unmarshals the property value to apply.
 */
static AJ_Status PropSetHandler(AJ_Message* replyMsg, uint32_t propId, void* context)
{
    if (propId == APP_INT_VAL_PROP) {
        return AJ_UnmarshalArgs(replyMsg, "i", &propVal);
    } else {
        return AJ_ERR_UNEXPECTED;
    }
}

#define CONNECT_TIMEOUT    (1000 * 1000)
#define UNMARSHAL_TIMEOUT  (1000 * 5)

int AJ_Main(void)
{
    AJ_Status status = AJ_OK;
    AJ_BusAttachment bus;
    uint8_t connected = FALSE;
    uint32_t sessionId = 0;

    /*
     * One time initialization before calling any other AllJoyn APIs
     */
    AJ_Initialize();

    AJ_PrintXML(AppObjects);
    AJ_RegisterObjects(AppObjects, NULL);


    while (TRUE) {
        AJ_Message msg;

        if (!connected) {
            status = AJ_StartService(&bus, "org.alljoyn", CONNECT_TIMEOUT, ServicePort, ServiceName, AJ_NAME_REQ_DO_NOT_QUEUE);
            if (status != AJ_OK) {
                continue;
            }
            printf("StartService returned AJ_OK\n");
            connected = TRUE;
            AJ_BusSetPasswordCallback(&bus, PasswordCallback);
        }

        status = AJ_UnmarshalMsg(&bus, &msg, UNMARSHAL_TIMEOUT);
        if (status != AJ_OK) {
            if (status == AJ_ERR_TIMEOUT) {
                AppDoWork();
                continue;
            }
        }
        if (status == AJ_OK) {
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

            case APP_MY_PING:
                status = AppHandlePing(&msg);
                break;

            case APP_GET_PROP:
                status = AJ_BusPropGet(&msg, PropGetHandler, NULL);
                break;

            case APP_SET_PROP:
                status = AJ_BusPropSet(&msg, PropSetHandler, NULL);
                if (status == AJ_OK) {
                    printf("Property successfully set to %d.\n", propVal);
                } else {
                    printf("Property set attempt unsuccessful. Status = 0x%04x.\n", status);
                }
                break;

            case AJ_SIGNAL_SESSION_LOST:
                /*
                 * Force a disconnect
                 */
                status = AJ_ERR_READ;
                break;

            default:
                /*
                 * Pass to the built-in bus message handlers
                 */
                status = AJ_BusHandleBusMessage(&msg);
                break;
            }
        }
        /*
         * Unarshaled messages must be closed to free resources
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
    printf("svclite EXIT %d\n", status);

    return status;
}

#ifdef AJ_MAIN
int main()
{
    return AJ_Main();
}
#endif
