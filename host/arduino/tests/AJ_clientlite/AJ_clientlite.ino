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


#undef WIFI_UDP_WORKING

#include <SPI.h>
#ifdef WIFI_UDP_WORKING
#include <WiFi.h>
#else
#include <Ethernet.h>
#endif

#include <alljoyn.h>

#ifdef WIFI_UDP_WORKING
static char ssid[] = "YOUR-WIFI";
static char pass[] = "71DF437B55"; // hex password for the SSID
int wifiStatus = WL_IDLE_STATUS;
#else
#endif

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
}

int AJ_Main(void);


void loop() {
    AJ_Main();
}

