/*
 * @file
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

#include <aj_target.h>
#include <alljoyn.h>

static const char ServiceName[] = "org.alljoyn.svclite";
static const uint16_t ServicePort = 24;
static const uint32_t NumPings = 10;


/*
 * The app should authenticate the peer if one or more interfaces are secure
 * To define a secure interface, prepend '$' before the interface name, eg., "$org.alljoyn.alljoyn_test"
 */
static uint8_t authPeer = FALSE;

static const char* const testInterface[] = {
    "org.alljoyn.alljoyn_test",
    "?my_ping inStr<s outStr>s",
    "?delayed_ping inStr<s delay<u outStr>s",
    "?time_ping <u <q >u >q",
    "!my_signal >a{ys}",
    NULL
};


static const AJ_InterfaceDescription testInterfaces[] = {
    testInterface,
    NULL
};

/**
 * Objects implemented by the application
 */
static const AJ_Object ProxyObjects[] = {
    { "/org/alljoyn/alljoyn_test", testInterfaces },
    { NULL }
};


#define PRX_MY_PING         AJ_PRX_MESSAGE_ID(0, 0, 0)
#define PRX_DELAYED_PING    AJ_PRX_MESSAGE_ID(0, 0, 1)
#define PRX_TIME_PING       AJ_PRX_MESSAGE_ID(0, 0, 2)
#define PRX_MY_SIGNAL       AJ_PRX_MESSAGE_ID(0, 0, 3)

static AJ_Status SendSignal(AJ_BusAttachment* bus, uint32_t sessionId);

/*
 * Let the application do some work
 */

#define PINGS_PER_CALL 10

static AJ_Status AppDoWork(AJ_BusAttachment* bus, uint32_t sessionId)
{
    static uint32_t pings_sent = 0;
    uint32_t pings = 0;
    AJ_Status status = AJ_OK;

    while (pings_sent < NumPings && pings++ < PINGS_PER_CALL && status == AJ_OK) {
        status = SendSignal(bus, sessionId);
        ++pings_sent;
    }

    return status;
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


static AJ_Status SendSignal(AJ_BusAttachment* bus, uint32_t sessionId)
{
    AJ_Status status;
    AJ_Message msg;


    status = AJ_MarshalSignal(bus, &msg, PRX_MY_SIGNAL, ServiceName, sessionId, 0, 0);
    if (status == AJ_OK) {
        AJ_Arg arg;
        status = AJ_MarshalContainer(&msg, &arg, AJ_ARG_ARRAY);
        status = AJ_MarshalCloseContainer(&msg, &arg);
    }
    if (status == AJ_OK) {
        status = AJ_DeliverMsg(&msg);
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
                if (authPeer) {
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
        }

        status = AJ_UnmarshalMsg(&bus, &msg, UNMARSHAL_TIMEOUT);
        if (status == AJ_ERR_TIMEOUT) {
            status = AppDoWork(&bus, sessionId);
            continue;
        }

        if (status == AJ_OK) {
            switch (msg.msgId) {
            case PRX_MY_SIGNAL:
                AJ_Printf("Received my_signal\n");
                status = AJ_OK;
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
            connected = FALSE;
        }
    }
    AJ_Printf("clientlite EXIT %d\n", status);

    return status;
}

#ifdef AJ_MAIN
int main()
{
    return AJ_Main();
}
#endif

