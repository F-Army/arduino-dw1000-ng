/*  
 *	Arduino-DWM1000 - Arduino library to use Decawave's DWM1000 module.
 *	Copyright (C) 2018  Michele Biondi <michelebiondi01@gmail.com>, Andrea Salvatori <andrea.salvatori92@gmail.com>
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
 * DWM1000:: Complements the "BasicSender" example sketch.
 * 
 * @todo
 *  - move strings to flash (less RAM consumption)
 *  
 */

#include <SPI.h>
#include <DWM1000.hpp>

// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin

// DEBUG packet sent status and count
int16_t numReceived = 0; // todo check int type
String message;
volatile boolean receiveDone = false;
volatile boolean receiveError = false;

void setup() {
  // DEBUG monitoring
  Serial.begin(115200);
  Serial.println(F("### DWM1000-arduino-receiver-test ###"));
  // initialize the driver
  DWM1000::begin(PIN_IRQ, PIN_RST);
  DWM1000::select(PIN_SS);
  Serial.println(F("DWM1000 initialized ..."));
  // general configuration
  DWM1000::newConfiguration();
  DWM1000::setDefaults();
  DWM1000::setReceiverAutoReenable(true);
  DWM1000::setDeviceAddress(6);
  DWM1000::setNetworkId(10);
  DWM1000::commitConfiguration();
  Serial.println(F("Committed configuration ..."));
  // DEBUG chip info and registers pretty printed
  char msg[128];
  DWM1000::getPrintableDeviceIdentifier(msg);
  Serial.print("Device ID: "); Serial.println(msg);
  DWM1000::getPrintableExtendedUniqueIdentifier(msg);
  Serial.print("Unique ID: "); Serial.println(msg);
  DWM1000::getPrintableNetworkIdAndShortAddress(msg);
  Serial.print("Network ID & Device Address: "); Serial.println(msg);
  DWM1000::getPrintableDeviceMode(msg);
  Serial.print("Device mode: "); Serial.println(msg);
  // attach callback for (successfully) received messages
  DWM1000::attachReceivedHandler(handleReceived);
  DWM1000::attachReceiveFailedHandler(handleError);
  // start reception
  DWM1000::startReceive();
}

void handleReceived() {
  receiveDone = true;
  
}

void handleError() {
  receiveError = true;
  
}

void loop() {
  if(receiveError) {
    receiveError = false;
    Serial.println("Error receiving a message");
    DWM1000::getData(message);
    Serial.print("Error data is ... "); Serial.println(message);
  }
  if(receiveDone) {
    receiveDone = false;
    numReceived++;
    // get data as string
    DWM1000::getData(message);
    Serial.print("Received message ... #"); Serial.println(numReceived);
    Serial.print("Data is ... "); Serial.println(message);
    Serial.print("FP power is [dBm] ... "); Serial.println(DWM1000::getFirstPathPower());
    Serial.print("RX power is [dBm] ... "); Serial.println(DWM1000::getReceivePower());
    Serial.print("Signal quality is ... "); Serial.println(DWM1000::getReceiveQuality());
    DWM1000::startReceive();
  }
}
