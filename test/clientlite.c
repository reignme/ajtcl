/*
 * clientlite.c
 */

/******************************************************************************
 * Copyright 2012-2013, Qualcomm Innovation Center, Inc.
 *
 *    All rights reserved.
 *    This file is licensed under the 3-clause BSD license in the NOTICE.txt
 *    file for this project. A copy of the 3-clause BSD license is found at:
 *
 *        http://opensource.org/licenses/BSD-3-Clause.
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the license is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the license for the specific language governing permissions and
 *    limitations under the license.
 ******************************************************************************/

#include <stdio.h>
#include <assert.h>

#include "alljoyn.h"
#include "aj_debug.h"
#include "aj_crypto.h"

static const char ServiceName[] = "org.alljoyn.svclite";
static const uint16_t ServicePort = 24;

/*
 * Use AJ_FLAG_ENCRYPTED for authFlag to request app-to-app authentication
 */
static uint8_t authFlag = 0;

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
static AJ_Status SendPing(AJ_BusAttachment* bus, uint32_t sessionId, unsigned int num);
static int32_t g_iterCount = 0;
static void AppDoWork(AJ_BusAttachment* bus, uint32_t sessionId)
{
    AJ_Printf("AppDoWork");
    /*
     * This function is called if there are no messages to unmarshal
     */
    g_iterCount = g_iterCount + 1;
    SendPing(bus, sessionId, g_iterCount);
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

static const char PingString[] = "Ping String";

AJ_Status SendPing(AJ_BusAttachment* bus, uint32_t sessionId, unsigned int num)
{
    AJ_Status status;
    AJ_Message msg;

    status = AJ_MarshalMethodCall(bus, &msg, PRX_MY_PING, ServiceName, sessionId, authFlag, METHOD_TIMEOUT);
    if (status == AJ_OK) {
        status = AJ_MarshalArgs(&msg, "s", PingString);
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
            AJ_Printf(">>>>>>>>In SendSetProp() AJ_MarshalPropertyArgs() returned status = 0x%04x\n", status);
        }

        if (status == AJ_OK) {
            status = AJ_DeliverMsg(&msg);
        } else {
            AJ_Printf(">>>>>>>>In SendSetProp() AJ_MarshalArgs() returned status = 0x%04x\n", status);
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
            status = AJ_StartClient(&bus, NULL, CONNECT_TIMEOUT, ServiceName, ServicePort, &sessionId, NULL);
            if (status == AJ_OK) {
                AJ_Printf("StartClient returned %d, sessionId=%u\n", status, sessionId);
                AJ_Printf("Connected to Daemon:%s\n", AJ_GetUniqueName(&bus));
                connected = TRUE;
                if (authFlag) {
                    AJ_BusSetPasswordCallback(&bus, PasswordCallback);
                    status = AJ_BusAuthenticatePeer(&bus, ServiceName, AuthCallback, &authStatus);
                    if (status != AJ_OK) {
                        AJ_Printf("AJ_BusAuthenticatePeer returned %d\n", status);
                    }
                } else {
                    authStatus = AJ_OK;
                }
            } else {
                AJ_Printf("StartClient returned %d\n", status);
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
                AppDoWork(&bus, sessionId);
                continue;
            }
        } else {
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
                        status = AJ_UnmarshalArgs(&msg, sig, &g_iterCount);
                        AJ_Printf("Get prop reply %d\n", g_iterCount);

                        if (status == AJ_OK) {
                            g_iterCount = g_iterCount + 1;
                            status = SendSetProp(&bus, sessionId, g_iterCount);
                        }
                    }
                }
                break;

            case AJ_REPLY_ID(PRX_SET_PROP):
                AJ_Printf("Set prop reply\n");
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
        }
        /*
         * Messages must be closed to free resources
         */
        AJ_CloseMsg(&msg);

        if (status == AJ_ERR_READ) {
            AJ_Printf("AllJoyn disconnect\n");
            AJ_Printf("Disconnected from Daemon:%s\n", AJ_GetUniqueName(&bus));
            AJ_Disconnect(&bus);
            return status;
        }
    }
    AJ_Printf("clientlite EXIT %d\n", status);

    return status;
}

#ifdef AJ_YIELD
extern AJ_MainRoutineType AJ_MainRoutine;

int main()
{
    AJ_MainRoutine = AJ_Main;

    // authFlag = AJ_FLAG_ENCRYPTED;
    while (1) {
        AJ_Loop();
        if (AJ_GetEventState(AJWAITEVENT_EXIT)) {
            return(0); // got the signal, so exit the app.
        }
    }
}
#else
#ifdef AJ_MAIN
int main()
{
    // authFlag = AJ_FLAG_ENCRYPTED;
    return AJ_Main();
}
#endif
#endif
