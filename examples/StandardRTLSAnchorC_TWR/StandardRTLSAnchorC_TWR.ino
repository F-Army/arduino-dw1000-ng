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
 * StandardRTLSAnchorC_TWR.ino
 * 
 * This is an example slave anchor in a RTLS using two way ranging ISO/IEC 24730-62_2013 messages
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

volatile byte SEQ_NUMBER = 0;

// timestamps to remember
volatile uint64_t timePollSent;
volatile uint64_t timePollReceived;
volatile uint64_t timePollAckSent;
volatile uint64_t timePollAckReceived;
volatile uint64_t timeRangeSent;
volatile uint64_t timeRangeReceived;

volatile double distance;

// watchdog and reset period
volatile uint32_t lastActivity;
volatile uint32_t resetPeriod = 250;
// reply times (same on both sides for symm. ranging)
uint16_t replyDelayTimeUS = 3000;

byte target_eui[8];
byte tag_shortAddress[] = {0x05, 0x00};

byte main_anchor_address[] = {0x01, 0x00};

device_configuration_t DEFAULT_CONFIG = {
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
    false,
    false,
    false,
    false
};

frame_filtering_configuration_t ANCHOR_FRAME_FILTER_CONFIG = {
    false,
    false,
    true,
    false,
    false,
    false,
    false,
    false
};

void setup() {
    // DEBUG monitoring
    Serial.begin(115200);
    Serial.println(F("### arduino-DW1000Ng-ranging-anchor-C ###"));
    // initialize the driver
    DW1000Ng::initialize(PIN_SS, PIN_IRQ, PIN_RST);
    Serial.println(F("DW1000Ng initialized ..."));
    // general configuration
    DW1000Ng::applyConfiguration(DEFAULT_CONFIG);
	DW1000Ng::applyInterruptConfiguration(DEFAULT_INTERRUPT_CONFIG);
    DW1000Ng::enableFrameFiltering(ANCHOR_FRAME_FILTER_CONFIG);
    
    DW1000Ng::setEUI("AA:BB:CC:DD:EE:FF:00:03");

    DW1000Ng::setNetworkId(RTLS_APP_ID);
    DW1000Ng::setDeviceAddress(3);
	
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

void transmitResponseToPoll() {
    byte pollAck[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0, ACTIVITY_CONTROL, RANGING_CONTINUE, 0, 0};
    DW1000Ng::getNetworkId(&pollAck[3]);
    memcpy(&pollAck[5], tag_shortAddress, 2);
    DW1000Ng::getDeviceAddress(&pollAck[7]);
    DW1000Ng::setTransmitData(pollAck, sizeof(pollAck));
    DW1000Ng::startTransmit();
}

void transmitActivityFinished() {
    /* I send the new blink rate to the tag */
    byte MS_250 = 0xFA;
    byte rangingConfirm[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0, ACTIVITY_CONTROL, ACTIVITY_FINISHED, MS_250, 0x00};
    DW1000Ng::getNetworkId(&rangingConfirm[3]);
    memcpy(&rangingConfirm[5], tag_shortAddress, 2);
    DW1000Ng::getDeviceAddress(&rangingConfirm[7]);
    DW1000Ng::setTransmitData(rangingConfirm, sizeof(rangingConfirm));
    DW1000Ng::startTransmit();
}

void transmitRangeReport() {
    byte rangingReport[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0, 0x60, 0,0 };
    DW1000Ng::getNetworkId(&rangingReport[3]);
    memcpy(&rangingReport[5], main_anchor_address, 2);
    DW1000Ng::getDeviceAddress(&rangingReport[7]);
    DW1000NgUtils::writeValueToBytes(&rangingReport[10], static_cast<uint16_t>((distance*1000)), 2);
    DW1000Ng::setTransmitData(rangingReport, sizeof(rangingReport));
    DW1000Ng::startTransmit();
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
        // get message and parse
        size_t recv_len = DW1000Ng::getReceivedDataLength();
        byte recv_data[recv_len];
        DW1000Ng::getReceivedData(recv_data, recv_len);

        if (recv_data[9] == RANGING_TAG_POLL) {
            timePollReceived = DW1000Ng::getReceiveTimestamp();
            transmitResponseToPoll();
            noteActivity();
            return;
        } 
        
        if (recv_data[9] == RANGING_TAG_FINAL_RESPONSE_EMBEDDED) {

            timePollAckSent = DW1000Ng::getTransmitTimestamp();
            timeRangeReceived = DW1000Ng::getReceiveTimestamp();

            timePollSent = DW1000NgUtils::bytesAsValue(recv_data + 10, LENGTH_TIMESTAMP);
            timePollAckReceived = DW1000NgUtils::bytesAsValue(recv_data + 14, LENGTH_TIMESTAMP);
            timeRangeSent = DW1000NgUtils::bytesAsValue(recv_data + 18, LENGTH_TIMESTAMP);
            // (re-)compute range as two-way ranging is done
            distance = DW1000NgRanging::computeRangeAsymmetric(timePollSent,
                                                        timePollReceived, 
                                                        timePollAckSent, 
                                                        timePollAckReceived, 
                                                        timeRangeSent, 
                                                        timeRangeReceived);
            /* Apply bias correction */
            distance = DW1000NgRanging::correctRange(distance);
            
            String rangeString = "Range: "; rangeString += distance; rangeString += " m";
            rangeString += "\t RX power: "; rangeString += DW1000Ng::getReceivePower(); rangeString += " dBm";
            Serial.println(rangeString);
            
            transmitActivityFinished();
            noteActivity();
            transmitRangeReport();
            return;
        }
    }
}

