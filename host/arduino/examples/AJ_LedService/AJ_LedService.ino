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

#include <SPI.h>
#include <WiFi.h>

#include <alljoyn.h>

#include "due_led.h"
#include <stdint.h>

int led = 13;

char ssid[] = "yourNetwork";     // the name of your network
int status = WL_IDLE_STATUS;     // the Wifi radio's status


void DUE_led_timed(uint32_t msec)
{
    printf("DUE_led_timed\n");
    digitalWrite(led, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(msec);             // wait for a second
    digitalWrite(led, LOW);  // turn the LED off by making the voltage LOW
    delay(msec);             // wait for a second
}

void DUE_led(uint8_t on)
{
    printf("DUE_led(%u)\n", on);
    digitalWrite(led, on ? HIGH : LOW); // turn the LED on (HIGH is the voltage level)
}


// the setup routine runs once when you press reset:
void setup() {
    // initialize the digital pin as an output.
    pinMode(led, OUTPUT);

    Serial.begin(9600);
    while (!Serial) ;

    digitalWrite(led, LOW);

    if (WiFi.status() == WL_NO_SHIELD) {
        Serial.println("WiFi shield not present");
        // don't continue:
        while (true) ;
    }

    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to open SSID: ");
        Serial.println(ssid);
        status = WiFi.begin(ssid);

        // wait 10 seconds for connection:
        delay(10000);
    }

    IPAddress ip = WiFi.localIP();

    Serial.print("Connected: ");
    Serial.println(ip);
}

// the loop routine runs over and over again forever:
void loop() {
    printf("Hello\n");
    AJ_Main();
}


