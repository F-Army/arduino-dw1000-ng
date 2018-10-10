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

rangingFrameType expectedResponse = rangingFrameType::RANGING_INITIATION;
rangingFrameType lastAction;

boolean sentAck = false;
boolean receivedAck = false;

byte short_address[2];

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
    DW1000Ng::setEUI("DE:AD:BE:EF:BE:EF:DE:AD");

    DW1000Ng::attachSentHandler(handleSent);
    DW1000Ng::attachReceivedHandler(handleReceived);

    Serial.println("Configurations committed");

    blink();
    noteActivity();
    DW1000Ng::startReceive();
}

void noteActivity() {
    // update activity timestamp, so that we do not reach "resetPeriod"
    lastActivity = millis();
}

void handleSent() {
    // status change on sent success
    sentAck = true;
}

void handleReceived() {
    // status change on received success
    receivedAck = true;
}

void blink() {
    DW1000Ng::encodeMinimalBlink();
    DW1000Ng::startTransmit(TransmitMode::IMMEDIATE);
}

void returnToBlink() {
    DW1000Ng::forceTRxOff();
    blink();
    rangingFrameType expectedResponse = rangingFrameType::RANGING_INITIATION;
    noteActivity();
    DW1000Ng::startReceive();
}

void loop() {
    if (!sentAck && !receivedAck) {
        // check if inactive
        if (millis() - lastActivity > resetPeriod) {
            Serial.println("Return to blink");
            returnToBlink();
        }
        return;
    }

    if(sentAck) {
        sentAck = false;
        DW1000Ng::startReceive();
    }

    if(receivedAck) {
        receivedAck = false;
        size_t DATA_LEN =  DW1000Ng::getReceivedDataLength();
        byte receivedData[DATA_LEN];
        DW1000Ng::getReceivedData(receivedData, DATA_LEN);
        frameType frame_type = DW1000Ng::getFrameType(receivedData);
        if(frame_type != frameType::DATA) {
            returnToBlink();
        } else {
            rangingFrameType ranging_frame_type = DW1000NgRanging::getRangingFrameType(receivedData);
            if(ranging_frame_type == expectedResponse) {
                data_offset_t offsets = DW1000Ng::getDataOffsets(receivedData);
                if(ranging_frame_type == rangingFrameType::RANGING_INITIATION) {
                    DW1000Ng::setDeviceAddress((uint16_t) DW1000NgUtils::bytesAsValue(&receivedData[offsets.application_offset], 2));
                    DW1000Ng::getDeviceAddress(short_address);
                    DW1000NgRanging::encodePoll(short_address, addressType::SHORT, &receivedData[offsets.source_offset], offsets.application_offset - offsets.source_offset == 8 ? addressType::LONG : addressType::SHORT);
                    DW1000Ng::startTransmit(TransmitMode::IMMEDIATE);
                    expectedResponse = rangingFrameType::RESPONSE_TO_POLL;
                    lastAction = rangingFrameType::POLL;
                } else if(ranging_frame_type == rangingFrameType::RESPONSE_TO_POLL) {
                    data_offset_t offsets = DW1000Ng::getDataOffsets(receivedData);
                    DW1000NgRanging::encodeFinalMessage(short_address, addressType::SHORT, &receivedData[offsets.source_offset], offsets.application_offset - offsets.source_offset == 8 ? addressType::LONG : addressType::SHORT, replyDelayTimeUS);
                    DW1000Ng::startTransmit(TransmitMode::DELAYED);
                    lastAction = rangingFrameType::FINAL_MESSAGE;
                }

                noteActivity();
            } else if(lastAction == rangingFrameType::FINAL_MESSAGE) {
                if(ranging_frame_type == rangingFrameType::RANGING_CONFIRM) {
                    returnToBlink();
                } else {
                    returnToBlink();
                }
            } else {
                returnToBlink();
            }
        }
    }

}