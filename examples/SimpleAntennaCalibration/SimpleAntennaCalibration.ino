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
 * @file RangingAnchor.ino
 * Use this to test two-way ranging functionality with two
 * DWM1000:: This is the anchor component's code which computes range after
 * exchanging some messages. Addressing and frame filtering is currently done
 * in a custom way, as no MAC features are implemented yet.
 *
 * Complements the "RangingTag" example sketch.
 *
 * @todo
 *  - weighted average of ranging results based on signal quality
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

#define EXPECTED_RANGE 7.94 // Recommended value for default values, refer to chapter 8.3.1 of DW1000 User manual
#define EXPECTED_RANGE_EPSILON 0.05
#define ACCURACY_THRESHOLD 5
#define ANTENNA_DELAY_STEPS 1

// message flow state
volatile byte expectedMsgId = POLL;
// message sent/received state
volatile boolean sentAck = false;
volatile boolean receivedAck = false;
// protocol error state
boolean protocolFailed = false;

// Antenna calibration variables
int accuracyCounter = 0;
uint16_t antenna_delay = 16384;

// timestamps to remember
DWM1000Time timePollSent;
DWM1000Time timePollReceived;
DWM1000Time timePollAckSent;
DWM1000Time timePollAckReceived;
DWM1000Time timeRangeSent;
DWM1000Time timeRangeReceived;
// last computed range/time
DWM1000Time timeComputedRange;
// data buffer
#define LEN_DATA 16
byte data[LEN_DATA];
// watchdog and reset period
uint32_t lastActivity;
uint32_t resetPeriod = 250;
// reply times (same on both sides for symm. ranging)
uint16_t replyDelayTimeUS = 3000;
// ranging counter (per second)
uint16_t successRangingCount = 0;
uint32_t rangingCountPeriod = 0;
float samplingRate = 0;

void setup() {
    // DEBUG monitoring
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("### DWM1000-arduino-ranging-anchor ###"));
    // initialize the driver
    DWM1000::begin(PIN_IRQ, PIN_RST);
    DWM1000::select(PIN_SS);
    Serial.println(F("DWM1000 initialized ..."));
    // general configuration
    DWM1000::newConfiguration();
    DWM1000::useExtendedFrameLength(false);
	DWM1000::useSmartPower(true);
	DWM1000::suppressFrameCheck(false);
	DWM1000::setFrameFilter(false);
	DWM1000::interruptOnSent(true);
	DWM1000::interruptOnReceived(true);
	DWM1000::interruptOnReceiveFailed(true);
	DWM1000::interruptOnReceiveTimestampAvailable(false);
	DWM1000::interruptOnAutomaticAcknowledgeTrigger(true);
	DWM1000::setChannel(CHANNEL_5);
	DWM1000::setAntennaDelay(16384);
	DWM1000::enableMode(MODE_SHORTRANGE_LOWPRF_MEDIUMPREAMBLE);
    DWM1000::setDeviceAddress(1);
    DWM1000::setNetworkId(10);
    DWM1000::setAntennaDelay(antenna_delay);
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
    // anchor starts in receiving mode, awaiting a ranging poll message
   
    receiver();
    noteActivity();
    // for first time ranging frequency computation
    rangingCountPeriod = millis();
}

void noteActivity() {
    // update activity timestamp, so that we do not reach "resetPeriod"
    lastActivity = millis();
}

void resetInactive() {
    // anchor listens for POLL
    expectedMsgId = POLL;
    receiver();
    noteActivity();
}

void handleSent() {
    // status change on sent success
    sentAck = true;
}

void handleReceived() {
    // status change on received success
    receivedAck = true;
}

void transmitPollAck() {
    data[0] = POLL_ACK;
    DWM1000::setData(data, LEN_DATA);
    DWM1000::startTransmit();
}

void transmitRangeReport(float curRange) {
    data[0] = RANGE_REPORT;
    // write final ranging result
    memcpy(data + 1, &curRange, 4);
    DWM1000::setData(data, LEN_DATA);
    DWM1000::startTransmit();
}

void transmitRangeFailed() {
    data[0] = RANGE_FAILED;
    DWM1000::setData(data, LEN_DATA);
    DWM1000::startTransmit();
}

void receiver() {
    DWM1000::forceTRxOff();
    // so we don't need to restart the receiver manually
    DWM1000::startReceive();
}

/*
 * RANGING ALGORITHMS
 * ------------------
 * Either of the below functions can be used for range computation (see line "CHOSEN
 * RANGING ALGORITHM" in the code).
 * - Asymmetric is more computation intense but least error prone
 * - Symmetric is less computation intense but more error prone to clock drifts
 *
 * The anchors and tags of this reference example use the same reply delay times, hence
 * are capable of symmetric ranging (and of asymmetric ranging anyway).
 */

void computeRangeAsymmetric() {
    // asymmetric two-way ranging (more computation intense, less error prone)
    DWM1000Time round1 = (timePollAckReceived - timePollSent).wrap();
    DWM1000Time reply1 = (timePollAckSent - timePollReceived).wrap();
    DWM1000Time round2 = (timeRangeReceived - timePollAckSent).wrap();
    DWM1000Time reply2 = (timeRangeSent - timePollAckReceived).wrap();
    DWM1000Time tof = (round1 * round2 - reply1 * reply2) / (round1 + round2 + reply1 + reply2);
    // set tof timestamp
    timeComputedRange.setTimestamp(tof);
}

void computeRangeSymmetric() {
    // symmetric two-way ranging (less computation intense, more error prone on clock drift)
    DWM1000Time tof = ((timePollAckReceived - timePollSent) - (timePollAckSent - timePollReceived) +
                      (timeRangeReceived - timePollAckSent) - (timeRangeSent - timePollAckReceived)) * 0.25f;
    // set tof timestamp
    timeComputedRange.setTimestamp(tof);
}

/*
 * END RANGING ALGORITHMS
 * ----------------------
 */

void loop() {
    int32_t curMillis = millis();
    if (!sentAck && !receivedAck) {
        // check if inactive
        if (curMillis - lastActivity > resetPeriod) {
            resetInactive();
        }
        return;
    }
    // continue on any success confirmation
    if (sentAck) {
        sentAck = false;
        byte msgId = data[0];
        if (msgId == POLL_ACK) {
            DWM1000::getTransmitTimestamp(timePollAckSent);
            noteActivity();
        }
        DWM1000::startReceive();
    }
    if (receivedAck) {
        receivedAck = false;
        // get message and parse
        DWM1000::getData(data, LEN_DATA);
        byte msgId = data[0];
        if (msgId != expectedMsgId) {
            // unexpected message, start over again (except if already POLL)
            protocolFailed = true;
        }
        if (msgId == POLL) {
            // on POLL we (re-)start, so no protocol failure
            protocolFailed = false;
            DWM1000::getReceiveTimestamp(timePollReceived);
            expectedMsgId = RANGE;
            transmitPollAck();
            noteActivity();
        }
        else if (msgId == RANGE) {
            DWM1000::getReceiveTimestamp(timeRangeReceived);
            expectedMsgId = POLL;
            if (!protocolFailed) {
                timePollSent.setTimestamp(data + 1);
                timePollAckReceived.setTimestamp(data + 6);
                timeRangeSent.setTimestamp(data + 11);
                // (re-)compute range as two-way ranging is done
                computeRangeAsymmetric(); // CHOSEN RANGING ALGORITHM
                float distance = timeComputedRange.getAsMeters();
                String rangeString = "Range: "; rangeString += distance; rangeString += " m";
                rangeString += "\t RX power: "; rangeString += DWM1000::getReceivePower(); rangeString += " dBm";
                rangeString += "\t Sampling: "; rangeString += samplingRate; rangeString += " Hz";
                Serial.println(rangeString);

                // Antenna delay script
                if(distance >= (EXPECTED_RANGE - EXPECTED_RANGE_EPSILON) && distance <= (EXPECTED_RANGE + EXPECTED_RANGE_EPSILON)) {
                    accuracyCounter++;
                } else {
                    accuracyCounter = 0;
                    antenna_delay += (distance > EXPECTED_RANGE) ? ANTENNA_DELAY_STEPS : -ANTENNA_DELAY_STEPS;
                    DWM1000::setAntennaDelay(antenna_delay);
                    DWM1000::commitConfiguration();
                }

                if(accuracyCounter == ACCURACY_THRESHOLD) {
                    Serial.print("Found Antenna Delay value (Divide by two if one antenna is set to 0): ");
                    Serial.println(antenna_delay);
                    delay(10000);
                }

                transmitRangeReport(timeComputedRange.getAsMicroSeconds());
                // update sampling rate (each second)                
                successRangingCount++;
                if (curMillis - rangingCountPeriod > 1000) {
                    samplingRate = (1000.0f * successRangingCount) / (curMillis - rangingCountPeriod);
                    rangingCountPeriod = curMillis;
                    successRangingCount = 0;
                }
            }
            else {
                transmitRangeFailed();
            }

            noteActivity();
        }
    }
}