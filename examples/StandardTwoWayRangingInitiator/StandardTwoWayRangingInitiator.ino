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
 * @file RangingTag.ino
 * Use this to test two-way ranging functionality with two DW1000Ng:: This is
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
#include <DW1000Ng.hpp>
#include <DW1000NgUtils.hpp>
#include <DW1000NgTime.hpp>
#include <DW1000NgConstants.hpp>
#include <DW1000NgRanging.hpp>
#include <DW1000NgRTLS.hpp>

// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin

// message flow state
// message sent/received state
volatile boolean sentAck = false;
volatile boolean receivedAck = false;
// timestamps to remember
uint64_t timePollSent;
uint64_t timePollAckReceived;
uint64_t timeRangeSent;
// watchdog and reset period
uint32_t lastActivity;
uint32_t resetPeriod = 250;
// reply times (same on both sides for symm. ranging)
uint16_t replyDelayTimeUS = 3000;

device_configuration_t DEFAULT_CONFIG = {
    false,
    false,
    true,
    true,
    true,
    false,
    SFDMode::STANDARD_SFD,
    Channel::CHANNEL_5,
    DataRate::RATE_850KBPS,
    PulseFrequency::FREQ_16MHZ,
    PreambleLength::LEN_256,
    PreambleCode::CODE_3
};

interrupt_configuration_t DEFAULT_INTERRUPT_CONFIG = {
    true,
    true,
    true,
    false,
    true
};

void setup() {
    // DEBUG monitoring
    Serial.begin(115200);
    Serial.println(F("### DW1000Ng-arduino-ranging-tag ###"));
    // initialize the driver
    DW1000Ng::initialize(PIN_SS, PIN_IRQ, PIN_RST);
    Serial.println("DW1000Ng initialized ...");
    // general configuration
    DW1000Ng::applyConfiguration(DEFAULT_CONFIG);
	DW1000Ng::applyInterruptConfiguration(DEFAULT_INTERRUPT_CONFIG);
    
    DW1000Ng::setEUI("FF:FF:FF:FF:FF:FF:FF:FF");

    
    DW1000Ng::setAntennaDelay(16436);
    
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
    // anchor starts by transmitting a POLL message
    transmitBlink();
    noteActivity();
}

void noteActivity() {
    // update activity timestamp, so that we do not reach "resetPeriod"
    lastActivity = millis();
}

void resetInactive() {
    // tag returns to Idle and sends POLL
    DW1000Ng::forceTRxOff();
    transmitBlink();
    noteActivity();
}

void handleSent() {
    sentAck = true;
}

void handleReceived() {
    receivedAck = true;
}

void transmitBlink() {
    byte Blink[] = {BLINK, 1, 0,0,0,0,0,0,0,0, 0x43, 0x02};
    DW1000Ng::getEUI(&Blink[2]);
    DW1000Ng::setTransmitData(Blink, sizeof(Blink));
    DW1000Ng::startTransmit();
}

void transmitPoll() {
    byte Poll[] = {DATA, SHORT_SRC_AND_DEST, 0x01, RTLS_APP_ID_LOW, RTLS_APP_ID_HIGH, 0x01, 0x00, 0,0 , 0x21};
    DW1000Ng::getDeviceAddress(&Poll[7]);
    DW1000Ng::setTransmitData(Poll, sizeof(Poll));
    DW1000Ng::startTransmit();
}


void transmitFinalMessage() {
    /* Calculation of future time */
    byte futureTimeBytes[LENGTH_TIMESTAMP];

	timeRangeSent = DW1000Ng::getSystemTimestamp();
	timeRangeSent += DW1000NgTime::microsecondsToUWBTime(replyDelayTimeUS);
    DW1000NgUtils::writeValueToBytes(futureTimeBytes, timeRangeSent, LENGTH_TIMESTAMP);
    DW1000Ng::setDelayedTRX(futureTimeBytes);
    timeRangeSent += DW1000Ng::getTxAntennaDelay();

    byte finalMessage[] = {DATA, SHORT_SRC_AND_DEST, 0x01, RTLS_APP_ID_LOW, RTLS_APP_ID_HIGH, 0x01, 0x00, 0,0, 0x23, 
        0,0,0,0,0,0,0,0,0,0,0,0
    };

    DW1000Ng::getDeviceAddress(&finalMessage[7]);

    DW1000NgUtils::writeValueToBytes(finalMessage + 10, (uint32_t) timePollSent, 4);
    DW1000NgUtils::writeValueToBytes(finalMessage + 14, (uint32_t) timePollAckReceived, 4);
    DW1000NgUtils::writeValueToBytes(finalMessage + 18, (uint32_t) timeRangeSent, 4);
    DW1000Ng::setTransmitData(finalMessage, sizeof(finalMessage));
    DW1000Ng::startTransmit(TransmitMode::DELAYED);
}

void loop() {
    if (!sentAck && !receivedAck) {
        // check if inactive
        if (millis() - lastActivity > resetPeriod) {
            resetInactive();
        }
        return;
    }

    if (sentAck) {
        sentAck = false;
        DW1000Ng::startReceive();
    }

    if (receivedAck) {
        receivedAck = false;
        /* Parse received message */
        size_t recv_len = DW1000Ng::getReceivedDataLength();
        byte recv_data[recv_len];
        DW1000Ng::getReceivedData(recv_data, recv_len);
        
        if(DW1000NgRanging::isStandardRangingMessage(recv_data, recv_len)) {
            /* RTLS standard message */
            if(recv_data[15] == 0x20) {
                DW1000Ng::setDeviceAddress(DW1000NgUtils::bytesAsValue(&recv_data[16], 2));
                transmitPoll();
                noteActivity();
            }

            if (recv_data[9] == 0x10 && recv_data[10] == 0x02) {
                /* Received Response to poll */
                timePollSent = DW1000Ng::getTransmitTimestamp();
                timePollAckReceived = DW1000Ng::getReceiveTimestamp();
                transmitFinalMessage();
                noteActivity();
            } else if (recv_data[9] == 0x10 && recv_data[10] == 0x01) {
                /* Received ranging confirm */
                transmitPoll();
                noteActivity();
            }
        }
    }
}