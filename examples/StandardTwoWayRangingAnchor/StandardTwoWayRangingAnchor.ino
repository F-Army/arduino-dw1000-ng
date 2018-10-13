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
 * StandardTwoWayRangingAnchor.ino
 * 
 * This is an example anchor in a RTLS using two way ranging ISO/IEC 24730-62_2013 messages
 */

#include <SPI.h>
#include <DW1000Ng.hpp>
#include <DW1000NgUtils.hpp>
#include <DW1000NgRanging.hpp>
#include <DW1000NgRTLS.hpp>

// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin

// message sent/received state
volatile boolean sentAck = false;
volatile boolean receivedAck = false;

byte SEQ_NUMBER = 0;

// timestamps to remember
uint64_t timePollSent;
uint64_t timePollReceived;
uint64_t timePollAckSent;
uint64_t timePollAckReceived;
uint64_t timeRangeSent;
uint64_t timeRangeReceived;

// watchdog and reset period
uint32_t lastActivity;
uint32_t resetPeriod = 250;
// reply times (same on both sides for symm. ranging)
uint16_t replyDelayTimeUS = 3000;
// ranging counter (per second)
uint16_t successRangingCount = 0;
uint32_t rangingCountPeriod = 0;
float samplingRate = 0;

byte target_eui[8];
byte tag_shortAddress[] = {0x05, 0x00};

byte self_address[2];

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
    Serial.println(F("### DW1000Ng-arduino-ranging-anchor ###"));
    // initialize the driver
    DW1000Ng::initialize(PIN_SS, PIN_IRQ, PIN_RST);
    Serial.println(F("DW1000Ng initialized ..."));
    // general configuration
    DW1000Ng::applyConfiguration(DEFAULT_CONFIG);
	DW1000Ng::applyInterruptConfiguration(DEFAULT_INTERRUPT_CONFIG);
    
    DW1000Ng::setEUI("AA:BB:CC:DD:EE:FF:00:01");

    DW1000Ng::setNetworkId(RTLS_APP_ID);
    DW1000Ng::setDeviceAddress(1);

    DW1000Ng::getDeviceAddress(self_address);
	
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
    
    // anchor starts in receiving mode, awaiting a ranging poll message
    receive();
    // for first time ranging frequency computation
    rangingCountPeriod = millis();
}

void noteActivity() {
    // update activity timestamp, so that we do not reach "resetPeriod"
    lastActivity = millis();
}

void receive() {
    DW1000Ng::startReceive();
    noteActivity();
}
 
void resetInactive() {
    // anchor listens for POLL
    DW1000Ng::forceTRxOff();
    receive();
}

void handleSent() {
    // status change on sent success
    sentAck = true;
}

void handleReceived() {
    // status change on received success
    receivedAck = true;
}

void transmitRangingInitiation() {
    byte RangingInitiation[] = {DATA, SHORT_SRC_LONG_DEST, SEQ_NUMBER++, 0,0, 0,0,0,0,0,0,0,0,  0,0, RANGING_INITIATION, 0,0};
    DW1000Ng::getNetworkId(&RangingInitiation[3]);
    memcpy(&RangingInitiation[5], target_eui, 8);
    DW1000Ng::getDeviceAddress(&RangingInitiation[13]);
    memcpy(&RangingInitiation[16], tag_shortAddress, 2);
    DW1000Ng::setTransmitData(RangingInitiation, sizeof(RangingInitiation));
    DW1000Ng::startTransmit();
}

void transmitResponseToPoll() {
    byte pollAck[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0, ACTIVITY_CONTROL, RANGING_CONTINUE, 0, 0};
    DW1000Ng::getNetworkId(&pollAck[3]);
    memcpy(&pollAck[5], tag_shortAddress, 2);
    DW1000Ng::getDeviceAddress(&pollAck[7]);
    DW1000Ng::setTransmitData(pollAck, sizeof(pollAck));
    DW1000Ng::startTransmit();
}

void transmitRangingConfirm() {
    byte rangingConfirm[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0, ACTIVITY_CONTROL, RANGING_CONFIRM, 0x01, 0x00};
    DW1000Ng::getNetworkId(&rangingConfirm[3]);
    memcpy(&rangingConfirm[5], tag_shortAddress, 2);
    DW1000Ng::getDeviceAddress(&rangingConfirm[7]);
    DW1000Ng::setTransmitData(rangingConfirm, sizeof(rangingConfirm));
    DW1000Ng::startTransmit();
}
 
void loop() {
    int32_t curMillis = millis();
    if (!sentAck && !receivedAck) {
        // check if inactive
        if (curMillis - lastActivity > resetPeriod) {
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
        // get message and parse
        size_t recv_len = DW1000Ng::getReceivedDataLength();
        byte recv_data[recv_len];
        DW1000Ng::getReceivedData(recv_data, recv_len);

        if(recv_data[0] == BLINK) {
            /* Is blink */
            memcpy(target_eui, &recv_data[2], 8);
            transmitRangingInitiation();
            noteActivity();
        }

        if(DW1000NgRanging::isStandardRangingMessage(recv_data, recv_len)) {
            if (recv_data[9] == RANGING_TAG_POLL) {
                /* Software frame filter */
                if(memcmp(self_address, &recv_data[5], 2) != 0) {
                    DW1000Ng::startReceive();
                    return;
                }

                // on POLL we (re-)start, so no protocol failure
                timePollReceived = DW1000Ng::getReceiveTimestamp();
                transmitResponseToPoll();
                noteActivity();
            } else if (recv_data[9] == RANGING_TAG_FINAL_RESPONSE_EMBEDDED) {
                /* Software frame filter */
                if(memcmp(self_address, &recv_data[5], 2) != 0) {
                    DW1000Ng::startReceive();
                    return;
                }

                timePollAckSent = DW1000Ng::getTransmitTimestamp();
                timeRangeReceived = DW1000Ng::getReceiveTimestamp();

                timePollSent = DW1000NgUtils::bytesAsValue(recv_data + 10, LENGTH_TIMESTAMP);
                timePollAckReceived = DW1000NgUtils::bytesAsValue(recv_data + 14, LENGTH_TIMESTAMP);
                timeRangeSent = DW1000NgUtils::bytesAsValue(recv_data + 18, LENGTH_TIMESTAMP);
                // (re-)compute range as two-way ranging is done
                double distance = DW1000NgRanging::computeRangeAsymmetric(timePollSent,
                                                            timePollReceived, 
                                                            timePollAckSent, 
                                                            timePollAckReceived, 
                                                            timeRangeSent, 
                                                            timeRangeReceived);
                /* Apply bias correction */
                distance = DW1000NgRanging::correctRange(distance);
                
                String rangeString = "Range: "; rangeString += distance; rangeString += " m";
                rangeString += "\t RX power: "; rangeString += DW1000Ng::getReceivePower(); rangeString += " dBm";
                rangeString += "\t Sampling: "; rangeString += samplingRate; rangeString += " Hz";
                Serial.println(rangeString);
                
                transmitRangingConfirm();
                successRangingCount++;
                if (curMillis - rangingCountPeriod > 1000) {
                    samplingRate = (1000.0f * successRangingCount) / (curMillis - rangingCountPeriod);
                    rangingCountPeriod = curMillis;
                    successRangingCount = 0;
                }
                noteActivity();
            }
        }   
    }
}

