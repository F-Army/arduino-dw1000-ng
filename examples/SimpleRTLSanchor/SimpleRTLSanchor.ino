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

#include <SPI.h>
#include <DW1000Ng.hpp>
#include <DW1000NgConstants.hpp>
#include <DW1000NgConfiguration.hpp>
#include <DW1000NgRTLS.hpp>
#include <DW1000NgRanging.hpp>
#include <DW1000NgUtils.hpp>

// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin

uint32_t lastActivity;
uint32_t resetPeriod = 250;
uint16_t replyDelayTimeUS = 3000;

boolean sentAck = false;
boolean receivedAck = false;

byte srcAddr[2];

byte shortTagAddr[2] = {0x00, 0x04};

uint32_t pollRx;
uint32_t responseToPollTx;
uint32_t finalMessageRx;

uint32_t pollTx;
uint32_t responseToPollRx;
uint32_t finalMessageTx;

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
    Serial.begin(115200);
    DW1000Ng::initialize(PIN_SS, PIN_IRQ, PIN_RST);
    DW1000Ng::applyConfiguration(DEFAULT_CONFIG);
	DW1000Ng::applyInterruptConfiguration(DEFAULT_INTERRUPT_CONFIG);

    DW1000Ng::setAntennaDelay(16436);
    DW1000Ng::setEUI("AA:AA:AA:AA:AA:AA:AA:AA");
    DW1000Ng::setDeviceAddress(1);

    DW1000Ng::getDeviceAddress(srcAddr);


    DW1000Ng::attachSentHandler(handleSent);
    DW1000Ng::attachReceivedHandler(handleReceived);

    Serial.println("Configurations committed");

    receive();
}

void noteActivity() {
    // update activity timestamp, so that we do not reach "resetPeriod"
    lastActivity = millis();
}

void handleSent() {
    // status change on sent success
    sentAck = true;
    DW1000Ng::startReceive();
}

void handleReceived() {
    // status change on received success
    receivedAck = true;
}

void receive() {
    DW1000Ng::startReceive();
    noteActivity();
}

void loop() {
    /*
    if (!sentAck && !receivedAck) {
        // check if inactive
        if (millis() - lastActivity > resetPeriod) {
            Serial.println("Return to receive");
            receive();
        }
        return;
    }
*/
    if(sentAck) {
        sentAck = false;
        receive();
        Serial.println("Trasmesso");
    }

    if(receivedAck) {
        receivedAck = false;
        size_t DATA_LEN =  DW1000Ng::getReceivedDataLength();
        byte receivedData[DATA_LEN];
        DW1000Ng::getReceivedData(receivedData, DATA_LEN);
        frameType frame_type = DW1000Ng::getFrameType(receivedData);
        if(frame_type == frameType::BLINK) {
            Serial.println("Ricevuto blink");
            byte destAddr[8];
            memcpy(destAddr, &receivedData[2], 8);
            DW1000NgRanging::encodeRangingInitiation(srcAddr, addressType::SHORT, destAddr, addressType::LONG, shortTagAddr);
            DW1000Ng::startTransmit(TransmitMode::IMMEDIATE);
        } else if(frame_type == frameType::DATA) {
            rangingFrameType ranging_type = DW1000NgRanging::getRangingFrameType(receivedData);

            if(ranging_type == rangingFrameType::POLL) {
                /* Save Poll reception time */
                pollRx = static_cast<uint32_t>(DW1000Ng::getTransmitTimestamp());

                /* Send response to Poll */
                DW1000NgRanging::encodeResponseToPoll(srcAddr, addressType::SHORT, shortTagAddr, addressType::SHORT);
                DW1000Ng::startTransmit(TransmitMode::IMMEDIATE);
            } else if(ranging_type == rangingFrameType::FINAL_MESSAGE) {
                /* Save response to Poll timestamp and Final message receive timestamp */
                responseToPollTx = static_cast<uint32_t>(DW1000Ng::getTransmitTimestamp());
                finalMessageRx = static_cast<uint32_t>(DW1000Ng::getReceiveTimestamp());

                pollTx = DW1000NgUtils::bytesAsValue(&receivedData[10], 4);
                responseToPollRx = DW1000NgUtils::bytesAsValue(&receivedData[14], 4);
                finalMessageTx = DW1000NgUtils::bytesAsValue(&receivedData[18], 4);

                double range = DW1000NgRanging::computeRangeAsymmetric(pollTx, pollRx, responseToPollTx, responseToPollRx, finalMessageTx, finalMessageRx);
                range = DW1000NgRanging::correctRange(range);

                Serial.print("Range finished:"); Serial.println(range);
                
                ranging_confirm_settings_t rc = {
                    rangingConfirmActivity::FINISHED,
                    1
                };
                DW1000NgRanging::encodeRangingConfirm(srcAddr, addressType::SHORT, shortTagAddr, addressType::SHORT, rc);
                DW1000Ng::startTransmit(TransmitMode::IMMEDIATE);
            } else {
                receive();
            }
        } else {
            receive();
        }
    }
}

