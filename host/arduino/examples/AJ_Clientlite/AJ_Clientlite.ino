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
#include <stdarg.h>
#include <stdio.h>


#define WIFI_UDP_WORKING 1

#ifdef WIFI_UDP_WORKING
#include <SPI.h>
#include <WiFi.h>
#else
#include <SPI.h>
#include <Ethernet.h>
#endif

#include <alljoyn.h>

#ifdef WIFI_UDP_WORKING
static char ssid[] = "YOUR-WIFI";
static char pass[] = "71DF437B55"; // hex password for the SSID
int wifiStatus = WL_IDLE_STATUS;
#else
#endif

#define CONNECT_TIMEOUT    (1000ul * 200)
#define UNMARSHAL_TIMEOUT  (1000ul * 5)
#define METHOD_TIMEOUT     (1000ul * 3)
/// globals
AJ_Status status = AJ_OK;
AJ_BusAttachment bus;
uint8_t connected = FALSE;
uint32_t sessionId = 0ul;
AJ_Status authStatus = AJ_ERR_NULL;
byte AJ_setup_done = 0;

/*
 * An application property to SET or GET
 */
int32_t g_iterCount = 0;

static const char DaemonName[] = "org.alljoyn";
static const char ServiceName[] = "org.alljoyn.alljoyn_test";
static const uint16_t ServicePort = 24;

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

AJ_Status SendPing(AJ_BusAttachment* bus, uint32_t sessionId, unsigned int num);

/*
 * Let the application do some work
 */
static void AppDoWork()
{
    Serial.println("AppDoWork");
    /*
     * This function is called if there are no messages to unmarshal
     */
    g_iterCount = g_iterCount + 1;
    SendPing(&bus, sessionId, g_iterCount);
}

AJ_Status SendPing(AJ_BusAttachment* bus, uint32_t sessionId, unsigned int num)
{
    Serial.println("SendPing");
    AJ_Status status;
    AJ_Message msg;
    char buf[80] = { 0 };

    sprintf(buf, "Ping String %u", num);

    status = AJ_MarshalMethodCall(bus, &msg, PRX_MY_PING, ServiceName, sessionId, 0, METHOD_TIMEOUT);
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

    status = AJ_MarshalMethodCall(bus, &msg, PRX_GET_PROP, ServiceName, sessionId, 0, METHOD_TIMEOUT);
    if (status == AJ_OK) {
        AJ_MarshalPropertyArgs(&msg, PRX_GET_INT);
        status = AJ_DeliverMsg(&msg);
    }
    return status;
}

AJ_Status SendSetProp(AJ_BusAttachment* bus, uint32_t sessionId, int32_t val)
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

void setup() {

    //Initialize serial and wait for port to open:
    Serial.begin(115200);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for Leonardo only
    }

    printf("hello, world.\n");

#ifdef WIFI_UDP_WORKING
    // check for the presence of the shield:
    if (WiFi.status() == WL_NO_SHIELD) {
        printf("WiFi shield not present\n");
        // don't continue:
        while (true) ;
    }

    // attempt to connect to Wifi network:
    while (wifiStatus != WL_CONNECTED) {
        printf("Attempting to connect to WPA SSID: %s\n", ssid);

        // Connect to WEP private network
        //    wifiStatus = WiFi.begin(ssid);
        wifiStatus = WiFi.begin(ssid, 0, pass);

        // wait 10 seconds for connection:
        delay(10000);

        // print your WiFi shield's IP address:
        IPAddress ip = WiFi.localIP();
        Serial.print("IP Address ");
        Serial.println(ip);
    }
#else
    byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
    // start the Ethernet connection:
    if (Ethernet.begin(mac) == 0) {
        printf("Failed to configure Ethernet using DHCP\n");
        // no point in carrying on, so do nothing forevermore:
        for (;;)
            ;
    }
#endif

    // you're connected now, so print out the data:
    printf("You're connected to the network\n");
    AJ_Initialize();
    AJ_RegisterObjects(NULL, ProxyObjects);
    AJ_setup_done = 1; // moved from one line above, I only want to do this once

}

int aj_main_loop()
{
    while (TRUE) {
        AJ_Message msg;

        if (!connected) {
            status = AJ_StartClient(&bus, DaemonName, CONNECT_TIMEOUT, ServiceName, ServicePort, &sessionId);
            if (status == AJ_OK) {
                printf("StartClient returned %d, sessionId=%u\n", status, sessionId);
                connected = TRUE;
                authStatus = AJ_OK;
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
        case AJ_REPLY_ID(PRX_SET_PROP):
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
                    printf("Get prop reply %d\n", g_iterCount);

                    if (status == AJ_OK) {
                        g_iterCount = g_iterCount + 1;
                        status = SendSetProp(&bus, sessionId, g_iterCount);
                    }
                }
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
        }
    }

    return 0;
}


void loop() {
    aj_main_loop();
}

