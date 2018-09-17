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
 * @file RangingTag.ino
 * Use this to test two-way ranging functionality with two DWM1000:: This is
 * the tag component's code which polls for range computation. Addressing and
 * frame filtering is currently done in a custom way, as no MAC features are
 * implemented yet.
 *
 * Complements the "RangingAnchor" example sketch.
 *
 * @todo
 *  - use enum instead of define
 *  - move strings to flash (less RAM consumption)
 */

#include <SPI.h>
#include <DWM1000.hpp>

// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin

// messages used in the ranging protocol
// TODO replace by enum
#define POLL 0
#define POLL_ACK 1
#define RANGE 2
#define RANGE_REPORT 3
#define RANGE_FAILED 255
// message flow state
volatile byte expectedMsgId = POLL_ACK;
// timestamps to remember
DWM1000Time timePollSent;
DWM1000Time timePollAckReceived;
DWM1000Time timeRangeSent;
// data buffer
#define LEN_DATA 16
byte data[LEN_DATA];
// watchdog and reset period
volatile uint32_t lastActivity;
uint32_t resetPeriod = 250;
// reply times (same on both sides for symm. ranging)
uint16_t replyDelayTimeUS = 3000;

void setup() {
    // DEBUG monitoring
    Serial.begin(115200);
    Serial.println(F("### DWM1000-arduino-ranging-tag ###"));
    // initialize the driver
    DWM1000::begin(PIN_IRQ, PIN_RST);
    DWM1000::select(PIN_SS);
    Serial.println("DWM1000 initialized ...");
    // general configuration
    DWM1000::newConfiguration();
    DWM1000::setDefaults();
    DWM1000::setDeviceAddress(2);
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
    // attach callback for (successfully) sent and received messages
    DWM1000::attachSentHandler(handleSent);
    DWM1000::attachReceivedHandler(handleReceived);
    // anchor starts by transmitting a POLL message
    transmitPoll();
}

void noteActivity() {
    // update activity timestamp, so that we do not reach "resetPeriod"
    lastActivity = millis();
}

void handleSent() {
    noteActivity();
    if(data[0] == POLL) {
        DWM1000::getTransmitTimestamp(timePollSent);
    } else if(data[0] == RANGE) {
        DWM1000::getTransmitTimestamp(timeRangeSent);
    }
    DWM1000::startReceive();
}

void handleReceived() {
    noteActivity();
    DWM1000::getData(data, LEN_DATA);
    if (data[0] != expectedMsgId) {
        // unexpected message, start over again
        //Serial.print("Received wrong message # "); Serial.println(msgId);
        transmitPoll();
        return;
    }
    if (data[0] == POLL_ACK) {
        DWM1000::getReceiveTimestamp(timePollAckReceived);
        transmitRange();
    } else if (data[0] == RANGE_REPORT) {
        float curRange;
        memcpy(&curRange, data + 1, 4);
        transmitPoll();
    } else if (data[0] == RANGE_FAILED) {
        transmitPoll();
    }
}

void transmitPoll() {
    data[0] = POLL;
    DWM1000::setData(data, LEN_DATA);
    DWM1000::startTransmit();
    expectedMsgId = POLL_ACK;
}

void transmitRange() {
    data[0] = RANGE;
    // delay sending the message and remember expected future sent timestamp
    timeRangeSent = DWM1000::setDelay(replyDelayTimeUS);
    timePollSent.getTimestamp(data + 1);
    timePollAckReceived.getTimestamp(data + 6);
    timeRangeSent.getTimestamp(data + 11);
    DWM1000::setData(data, LEN_DATA);
    DWM1000::startTransmit(TransmitMode::DELAYED);
    //Serial.print("Expect RANGE to be sent @ "); Serial.println(timeRangeSent.getAsFloat());
    expectedMsgId = RANGE_REPORT;
}

void loop() {
    // check if inactive
    if (millis() - lastActivity > resetPeriod) {
        DWM1000::forceTRxOff();
        transmitPoll();
    }
}

