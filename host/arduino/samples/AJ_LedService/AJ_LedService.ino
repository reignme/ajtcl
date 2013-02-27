


#include <SPI.h>
#include <WiFi.h>

#include <alljoyn.h>

/*
  Blink
 Turns on an LED on for one second, then off for one second, repeatedly.
 
 This example code is in the public domain.
 */

#include "due_led.h"
#include <stdint.h>



// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;

char ssid[] = "eric-wifi";     // the name of your network
int status = WL_IDLE_STATUS;     // the Wifi radio's status


void DUE_led_timed(uint32_t msec)
{
  printf("DUE_led\n");
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(msec);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(msec);               // wait for a second
}

void DUE_led(uint8_t on)
{
  printf("DUE_led(%u)\n", on);
  //if (on) {
  //  digitalWrite(led, ? HIGH : LOW);
 // }
  digitalWrite(led, on ? HIGH : LOW);   // turn the LED on (HIGH is the voltage level)
//  delay(msec);               // wait for a second
//  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
//  delay(msec);               // wait for a second
}


// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);

  Serial.begin(9600);
  while (!Serial);
  
  digitalWrite(led, LOW);
  
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
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


