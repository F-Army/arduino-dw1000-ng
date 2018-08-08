/*
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>
 * Decawave DWM1000 library for arduino.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file BasicReceiver.ino
 * Use this to test simple sender/receiver functionality with two
 * DWM1000. Complements the "BasicSender" example sketch.
 * 
 * @todo
 *  - move strings to flash (less RAM consumption)
 *  
 */

#include <SPI.h>
#include <DWM1000.h>

// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin

// DEBUG packet sent status and count
volatile boolean received = false;
volatile boolean error = false;
volatile int16_t numReceived = 0; // todo check int type
String message;

void setup() {
  // DEBUG monitoring
  Serial.begin(9600);
  Serial.println(F("### DWM1000-arduino-receiver-test ###"));
  // initialize the driver
  DWM1000.begin(PIN_IRQ, PIN_RST);
  DWM1000.select(PIN_SS);
  Serial.println(F("DWM1000 initialized ..."));
  // general configuration
  DWM1000.newConfiguration();
  DWM1000.setDefaults();
  DWM1000.setDeviceAddress(6);
  DWM1000.setNetworkId(10);
  DWM1000.commitConfiguration();
  Serial.println(F("Committed configuration ..."));
  // DEBUG chip info and registers pretty printed
  char msg[128];
  DWM1000.getPrintableDeviceIdentifier(msg);
  Serial.print("Device ID: "); Serial.println(msg);
  DWM1000.getPrintableExtendedUniqueIdentifier(msg);
  Serial.print("Unique ID: "); Serial.println(msg);
  DWM1000.getPrintableNetworkIdAndShortAddress(msg);
  Serial.print("Network ID & Device Address: "); Serial.println(msg);
  DWM1000.getPrintableDeviceMode(msg);
  Serial.print("Device mode: "); Serial.println(msg);
  // attach callback for (successfully) received messages
  DWM1000.attachReceivedHandler(handleReceived);
  DWM1000.attachReceiveFailedHandler(handleError);
  DWM1000.attachErrorHandler(handleError);
  // start reception
  receiver();
}

void handleReceived() {
  // status change on reception success
  received = true;
}

void handleError() {
  error = true;
}

void receiver() {
  DWM1000.newReceive();
  DWM1000.setDefaults();
  // so we don't need to restart the receiver manually
  DWM1000.receivePermanently(true);
  DWM1000.startReceive();
}

void loop() {
  // enter on confirmation of ISR status change (successfully received)
  if (received) {
    numReceived++;
    // get data as string
    DWM1000.getData(message);
    Serial.print("Received message ... #"); Serial.println(numReceived);
    Serial.print("Data is ... "); Serial.println(message);
    Serial.print("FP power is [dBm] ... "); Serial.println(DWM1000.getFirstPathPower());
    Serial.print("RX power is [dBm] ... "); Serial.println(DWM1000.getReceivePower());
    Serial.print("Signal quality is ... "); Serial.println(DWM1000.getReceiveQuality());
    received = false;
  }
  if (error) {
    Serial.println("Error receiving a message");
    error = false;
    DWM1000.getData(message);
    Serial.print("Error data is ... "); Serial.println(message);
  }
}
