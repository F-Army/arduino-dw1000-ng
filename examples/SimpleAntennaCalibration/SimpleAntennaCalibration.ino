/*
 * MIT License
 * 
 * Copyright (c) 2018 Michele Biondi, Andrea Salvatori
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

/*
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>
 * Decawave DW1000 library for arduino.
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
 * DW1000Ng:: This is the anchor component's code which computes range after
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
#include <DW1000Ng.hpp>

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
DW1000NgTime timePollSent;
DW1000NgTime timePollReceived;
DW1000NgTime timePollAckSent;
DW1000NgTime timePollAckReceived;
DW1000NgTime timeRangeSent;
DW1000NgTime timeRangeReceived;
// last computed range/time
DW1000NgTime timeComputedRange;
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
    Serial.println(F("### DW1000Ng-arduino-ranging-anchor ###"));
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
    DW1000Ng::setPreambleLength(PreambleLength::LEN_256);
    DW1000Ng::setPreambleCode(PreambleCode::CODE_3);
    DW1000Ng::commitConfiguration();

    DW1000Ng::setDeviceAddress(1);
    DW1000Ng::setNetworkId(10);
    
	DW1000Ng::setAntennaDelay(antenna_delay);
    
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
    // attach callback for (successfully) sent and received messages
    DW1000Ng::attachSentHandler(handleSent);
    DW1000Ng::attachReceivedHandler(handleReceived);
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
    DW1000Ng::setTransmitData(data, LEN_DATA);
    DW1000Ng::startTransmit();
}

void transmitRangeReport(float curRange) {
    data[0] = RANGE_REPORT;
    // write final ranging result
    memcpy(data + 1, &curRange, 4);
    DW1000Ng::setTransmitData(data, LEN_DATA);
    DW1000Ng::startTransmit();
}

void transmitRangeFailed() {
    data[0] = RANGE_FAILED;
    DW1000Ng::setTransmitData(data, LEN_DATA);
    DW1000Ng::startTransmit();
}

void receiver() {
    DW1000Ng::forceTRxOff();
    // so we don't need to restart the receiver manually
    DW1000Ng::startReceive();
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
    DW1000NgTime round1 = (timePollAckReceived - timePollSent).wrap();
    DW1000NgTime reply1 = (timePollAckSent - timePollReceived).wrap();
    DW1000NgTime round2 = (timeRangeReceived - timePollAckSent).wrap();
    DW1000NgTime reply2 = (timeRangeSent - timePollAckReceived).wrap();
    DW1000NgTime tof = (round1 * round2 - reply1 * reply2) / (round1 + round2 + reply1 + reply2);
    // set tof timestamp
    timeComputedRange.setTimestamp(tof);
}

void computeRangeSymmetric() {
    // symmetric two-way ranging (less computation intense, more error prone on clock drift)
    DW1000NgTime tof = ((timePollAckReceived - timePollSent) - (timePollAckSent - timePollReceived) +
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
            DW1000Ng::getTransmitTimestamp(timePollAckSent);
            noteActivity();
        }
        DW1000Ng::startReceive();
    }
    if (receivedAck) {
        receivedAck = false;
        // get message and parse
        DW1000Ng::getData(data, LEN_DATA);
        byte msgId = data[0];
        if (msgId != expectedMsgId) {
            // unexpected message, start over again (except if already POLL)
            protocolFailed = true;
        }
        if (msgId == POLL) {
            // on POLL we (re-)start, so no protocol failure
            protocolFailed = false;
            DW1000Ng::getReceiveTimestamp(timePollReceived);
            expectedMsgId = RANGE;
            transmitPollAck();
            noteActivity();
        }
        else if (msgId == RANGE) {
            DW1000Ng::getReceiveTimestamp(timeRangeReceived);
            expectedMsgId = POLL;
            if (!protocolFailed) {
                timePollSent.setTimestamp(data + 1);
                timePollAckReceived.setTimestamp(data + 6);
                timeRangeSent.setTimestamp(data + 11);
                // (re-)compute range as two-way ranging is done
                computeRangeAsymmetric(); // CHOSEN RANGING ALGORITHM
                float distance = timeComputedRange.getAsMeters();
                String rangeString = "Range: "; rangeString += distance; rangeString += " m";
                rangeString += "\t RX power: "; rangeString += DW1000Ng::getReceivePower(); rangeString += " dBm";
                rangeString += "\t Sampling: "; rangeString += samplingRate; rangeString += " Hz";
                Serial.println(rangeString);

                // Antenna delay script
                if(distance >= (EXPECTED_RANGE - EXPECTED_RANGE_EPSILON) && distance <= (EXPECTED_RANGE + EXPECTED_RANGE_EPSILON)) {
                    accuracyCounter++;
                } else {
                    accuracyCounter = 0;
                    antenna_delay += (distance > EXPECTED_RANGE) ? ANTENNA_DELAY_STEPS : -ANTENNA_DELAY_STEPS;
                    DW1000Ng::setAntennaDelay(antenna_delay);
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