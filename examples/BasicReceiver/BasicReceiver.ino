/*  
 *	Arduino-DW1000Ng - Arduino library to use Decawave's DW1000Ng module.
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
 * Decawave DW1000Ng library for arduino.
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
 * DW1000Ng:: Complements the "BasicSender" example sketch.
 * 
 * @todo
 *  - move strings to flash (less RAM consumption)
 *  
 */

#include <SPI.h>
#include <DW1000Ng.hpp>

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
  Serial.println(F("### DW1000Ng-arduino-receiver-test ###"));
  // initialize the driver
  DW1000Ng::begin(PIN_SS, PIN_IRQ, PIN_RST);
  Serial.println(F("DW1000Ng initialized ..."));
  // general configuration
  DW1000Ng::newConfiguration();
  DW1000Ng::useExtendedFrameLength(false);
	DW1000Ng::useSmartPower(true);
	DW1000Ng::suppressFrameCheck(false);
	DW1000Ng::setFrameFilter(false);
	DW1000Ng::interruptOnSent(true);
	DW1000Ng::interruptOnReceived(true);
	DW1000Ng::interruptOnReceiveFailed(true);
	DW1000Ng::interruptOnReceiveTimestampAvailable(false);
	DW1000Ng::interruptOnAutomaticAcknowledgeTrigger(true);
  DW1000Ng::setSFDMode(SFDMode::STANDARD_SFD);
	DW1000Ng::setChannel(Channel::CHANNEL_5);
  DW1000Ng::setDataRate(DataRate::RATE_850KBPS);
  DW1000Ng::setPulseFrequency(PulseFrequency::FREQ_16MHZ);
  DW1000Ng::setPreambleLength(PreambleLength::LEN_128);
  DW1000Ng::setPreambleCode(PreambleCode::CODE_3);
  DW1000Ng::setReceiverAutoReenable(true);
  DW1000Ng::commitConfiguration();

  DW1000Ng::setDeviceAddress(6);
  DW1000Ng::setNetworkId(10);

  DW1000Ng::setAntennaDelay(16384);
  Serial.println(F("Committed configuration ..."));
  // DEBUG chip info and registers pretty printed
  char msg[128];
  DW1000Ng::getPrintableDeviceIdentifier(msg);
  Serial.print("Device ID: "); Serial.println(msg);
  DW1000Ng::getPrintableExtendedUniqueIdentifier(msg);
  Serial.print("Unique ID: "); Serial.println(msg);
  DW1000Ng::getPrintableNetworkIdAndShortAddress(msg);
  Serial.print("Network ID & Device Address: "); Serial.println(msg);
  DW1000Ng::getPrintableDeviceMode(msg);
  Serial.print("Device mode: "); Serial.println(msg);
  // attach callback for (successfully) received messages
  DW1000Ng::attachReceivedHandler(handleReceived);
  DW1000Ng::attachReceiveFailedHandler(handleError);
  DW1000Ng::attachErrorHandler(handleError);
  // start reception
  DW1000Ng::startReceive();
}

void handleReceived() {
  // status change on reception success
  received = true;
}

void handleError() {
  error = true;
}

void loop() {
  // enter on confirmation of ISR status change (successfully received)
  if (received) {
    received = false;
    numReceived++;
    // get data as string
    DW1000Ng::getData(message);
    Serial.print("Received message ... #"); Serial.println(numReceived);
    Serial.print("Data is ... "); Serial.println(message);
    Serial.print("RX power is [dBm] ... "); Serial.println(DW1000Ng::getReceivePower());
    Serial.print("Signal quality is ... "); Serial.println(DW1000Ng::getReceiveQuality());
    DW1000Ng::startReceive();
  }
  if (error) {
    error = false;
    Serial.println("Error receiving a message");
    DW1000Ng::getData(message);
    Serial.print("Error data is ... "); Serial.println(message);
  }
}
