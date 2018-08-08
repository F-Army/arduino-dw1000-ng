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
 * @file BasicSender.ino
 * Use this to test simple sender/receiver functionality with two
 * DWM1000. Complements the "BasicReceiver" example sketch. 
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
boolean sent = false;
volatile boolean sentAck = false;
volatile unsigned long delaySent = 0;
int16_t sentNum = 0; // todo check int type
DWM1000Time sentTime;

void setup() {
  // DEBUG monitoring
  Serial.begin(9600);
  Serial.println(F("### DWM1000-arduino-sender-test ###"));
  // initialize the driver
  DWM1000.begin(PIN_IRQ, PIN_RST);
  DWM1000.select(PIN_SS);
  Serial.println(F("DWM1000 initialized ..."));
  // general configuration
  DWM1000.newConfiguration();
  DWM1000.setDefaults();
  DWM1000.setDeviceAddress(5);
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
  // attach callback for (successfully) sent messages
  DWM1000.attachSentHandler(handleSent);
  // start a transmission
  transmitter();
}

void handleSent() {
  // status change on sent success
  sentAck = true;
}

void transmitter() {
  // transmit some data
  Serial.print("Transmitting packet ... #"); Serial.println(sentNum);
  DWM1000.newTransmit();
  DWM1000.setDefaults();
  String msg = "Hello DWM1000, it's #"; msg += sentNum;
  DWM1000.setData(msg);
  // delay sending the message for the given amount
  DWM1000Time deltaTime = DWM1000Time(10, DWM1000Time::MILLISECONDS);
  DWM1000.setDelay(deltaTime);
  DWM1000.startTransmit();
  delaySent = millis();
}

void loop() {
  if (!sentAck) {
    return;
  }
  // continue on success confirmation
  // (we are here after the given amount of send delay time has passed)
  sentAck = false;
  // update and print some information about the sent message
  Serial.print("ARDUINO delay sent [ms] ... "); Serial.println(millis() - delaySent);
  DWM1000Time newSentTime;
  DWM1000.getTransmitTimestamp(newSentTime);
  Serial.print("Processed packet ... #"); Serial.println(sentNum);
  Serial.print("Sent timestamp ... "); Serial.println(newSentTime.getAsMicroSeconds());
  // note: delta is just for simple demo as not correct on system time counter wrap-around
  Serial.print("DWM1000 delta send time [ms] ... "); Serial.println((newSentTime.getAsMicroSeconds() - sentTime.getAsMicroSeconds()) * 1.0e-3);
  sentTime = newSentTime;
  sentNum++;
  // again, transmit some data
  transmitter();
}
