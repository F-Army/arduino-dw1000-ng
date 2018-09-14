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
 * @file MessagePingPong.ino
 * Use this to test two-way communication functionality with two
 * DWM1000:: Both Arduinos use this sketch, but one node configured
 * as initiator/sender of the (first) ping message and the other 
 * being configured as receiver/answerer of the (first) ping message.
 *
 * Configure each node by setting their "trxToggle" attribute to either
 * "SENDER" or "RECEIVER".
 * 
 * @todo
 * - add in SENDER mode timeout if no pong received then resend ping
 */

#include "require_cpp11.hpp"
#include <SPI.h>
#include <DWM1000.hpp>

// connection pins
constexpr uint8_t PIN_RST = 9; // reset pin
constexpr uint8_t PIN_IRQ = 2; // irq pin
constexpr uint8_t PIN_SS = SS; // spi select pin

// toggle state
enum class TransmissionState : uint8_t {
  SENDER,
  RECEIVER
};
// toggle and message RX/TX
// NOTE: the other Arduino needs to be configured with RECEIVER
//       (or SENDER respectively)
TransmissionState trxToggle = TransmissionState::RECEIVER;
volatile boolean trxAck = false;
volatile boolean rxError = false;
String msg;

void setup() {
  // DEBUG monitoring
  Serial.begin(9600);
  Serial.println(F("### DWM1000-arduino-ping-pong-test ###"));
  // initialize the driver
  DWM1000::begin(PIN_IRQ, PIN_RST);
  DWM1000::select(PIN_SS);
  Serial.println(F("DWM1000 initialized ..."));
  // general configuration
  DWM1000::newConfiguration();
  DWM1000::setDefaults();
  DWM1000::setDeviceAddress(1);
  DWM1000::setNetworkId(10);
  DWM1000::commitConfiguration();
  Serial.println(F("Committed configuration ..."));
  // DEBUG chip info and registers pretty printed
  char msgInfo[128];
  DWM1000::getPrintableDeviceIdentifier(msgInfo);
  Serial.print(F("Device ID: ")); Serial.println(msgInfo);
  DWM1000::getPrintableExtendedUniqueIdentifier(msgInfo);
  Serial.print(F("Unique ID: ")); Serial.println(msgInfo);
  DWM1000::getPrintableNetworkIdAndShortAddress(msgInfo);
  Serial.print(F("Network ID & Device Address: ")); Serial.println(msgInfo);
  DWM1000::getPrintableDeviceMode(msgInfo);
  Serial.print(F("Device mode: ")); Serial.println(msgInfo);
  // attach callback for (successfully) sent and received messages
  DWM1000::attachSentHandler(handleSent);
  DWM1000::attachReceivedHandler(handleReceived);
  DWM1000::attachReceiveFailedHandler(handleReceiveFailed);
  // sender starts by sending a PING message, receiver starts listening
  if (trxToggle == TransmissionState::SENDER) {
    msg = "Ping ...";
    receiver();
    transmit();
  } else {
    msg = "... and Pong";
    receiver();
  }
}

void handleSent() {
  // status change on sent success
  trxAck = true;
}

void handleReceived() {
  // status change on received success
  trxAck = true;
}

void handleReceiveFailed() {
  // error flag
  rxError = true;
}

void transmit() {
  DWM1000::setData(msg);
  DWM1000::startTransmit();
}

void receiver() {
  DWM1000::newReceive();
  // so we don't need to restart the receiver manually
  DWM1000::receivePermanently(true);
  DWM1000::startReceive();
}

void loop() {
  if (rxError) {
    Serial.println(F("Failed to properly receive message."));
    rxError = false;
    return;
  }
  if (!trxAck) {
    return;
  }
  // continue on any success confirmation
  trxAck = false;
  // a sender will be a receiver and vice versa
  trxToggle = (trxToggle == TransmissionState::SENDER) ? TransmissionState::RECEIVER : TransmissionState::SENDER;
  if (trxToggle == TransmissionState::SENDER) {
    // formerly a receiver
    String rxMsg;
    DWM1000::getData(rxMsg);
    Serial.print(F("Received: ")); Serial.println(rxMsg);
    transmit();
  } else {
    Serial.print(F("Transmitted: ")); Serial.println(msg);
  }
}
