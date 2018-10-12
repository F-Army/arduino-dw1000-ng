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
 * @file BasicReceiver.ino
 * Use this to test simple sender/receiver functionality with two
 * DW1000:: Complements the "BasicSender" example sketch.
 * 
 * @todo
 *  - move strings to flash (less RAM consumption)
 *  
 */

#include <SPI.h>
#include <DW1000Ng.hpp>
#include <DW1000NgRTLS.hpp>
#include <DW1000NgRanging.hpp>

// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin

// DEBUG packet sent status and count
volatile boolean received = false;
volatile boolean error = false;
volatile int16_t numReceived = 0; // todo check int type
String message;

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
  Serial.begin(9600);
  Serial.println(F("### DW1000Ng-arduino-receiver-test ###"));
  // initialize the driver
  DW1000Ng::initialize(PIN_SS, PIN_IRQ, PIN_RST);
  Serial.println(F("DW1000Ng initialized ..."));

  DW1000Ng::applyConfiguration(DEFAULT_CONFIG);
	DW1000Ng::applyInterruptConfiguration(DEFAULT_INTERRUPT_CONFIG);

  DW1000Ng::setDeviceAddress(6);
  DW1000Ng::setNetworkId(10);

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

    uint16_t received_len = DW1000Ng::getReceivedDataLength();
    byte received[received_len];
    // get data as string
    DW1000Ng::getReceivedData(received, received_len);
    data_offset_t offsets = DW1000Ng::getDataOffsets(received);
    Serial.print("SRC OFFSET = "); Serial.println(offsets.source_offset);
    Serial.print("APP OFFSET = "); Serial.println(offsets.application_offset);
    switch(DW1000NgRanging::getRangingFrameType(received)) {
      case rangingFrameType::RANGING_INITIATION:
        Serial.println("Received ranging Initiation");
        break;
      case rangingFrameType::FINAL_MESSAGE:
        Serial.println("Received final range message");
        break;
       case rangingFrameType::POLL:
        Serial.println("Poll received");
        break;
       case rangingFrameType::RESPONSE_TO_POLL:
        Serial.println("Response to Poll Received");
        break;
      default:
        Serial.print("Davide Verdecchia");
    }
    switch(DW1000Ng::getFrameType(received)) {
      case frameType::BLINK:
        Serial.print("Received Blink Frame #");
        break;
       case frameType::DATA:
        Serial.print("Received Data Frame #");
        break;
       case frameType::OTHER:
        Serial.print("Received Unknown Frame #");
        break;
       default:
        Serial.print("Something went wrong #");
    }
      Serial.println(numReceived);
    String received_data;
    for(auto i=0; i < received_len; i++) {
      received_data += received[i];
      received_data += " ";
    }
    Serial.println(received_data);
    DW1000Ng::startReceive();
  }
  if (error) {
    error = false;
    Serial.println("Error receiving a message");
    DW1000Ng::getReceivedData(message);
    Serial.print("Error data is ... "); Serial.println(message);
  }
}