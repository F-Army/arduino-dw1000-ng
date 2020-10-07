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
 * @file SaveAntennaDelay.ino
 * This example shows how you can save the antenna delay in the EEPROM of your arduino.
 * When saved in EEPROM you can retrieve it after a reboot.
 * 
 */

#include <DW1000Ng.hpp>

// connection pins
#if defined(ESP8266)
const uint8_t PIN_RST = 5; // reset pin
const uint8_t PIN_IRQ = 4; // irq pin
const uint8_t PIN_SS = 15; // spi select pin
#else
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin
#endif


void setup() {
  // DEBUG monitoring
  Serial.begin(9600);
  // initialize the driver
  DW1000Ng::initialize(PIN_SS, PIN_IRQ, PIN_RST);
  Serial.println(F("DW1000Ng initialized ..."));
  // general configuration
  DW1000Ng::setDeviceAddress(5);
  DW1000Ng::setNetworkId(10);
  Serial.println(F("Committed configuration ..."));
  // First we genereate a random integer to set as antenna delay
  uint16_t randNumber = random(65535);
  DW1000Ng::setAndSaveAntennaDelay(randNumber);
  // Now the antenna delay is set to this number, and it is saved in the EEPROM
  Serial.println("Now in EEPROM: " + String(DW1000Ng::getSavedAntennaDelay()));
  Serial.println("TX delay: " + String(DW1000Ng::getTxAntennaDelay()));
  Serial.println("RX delay: " + String(DW1000Ng::getRxAntennaDelay()));
  // We simulate a reboot by resetting the antenna delay
  DW1000Ng::setAntennaDelay(0);
  Serial.println("TX delay: " + String(DW1000Ng::getTxAntennaDelay()));
  Serial.println("RX delay: " + String(DW1000Ng::getRxAntennaDelay()));
  // We can now restore it from EEPROM
  DW1000Ng::setAntennaDelayFromEEPROM();
  Serial.println("Restored from EEPROM");
  Serial.println("TX delay: " + String(DW1000Ng::getTxAntennaDelay()));
  Serial.println("RX delay: " + String(DW1000Ng::getRxAntennaDelay()));

  // You can also of course retrieve the value saved in EEPROM like this
  uint16_t delaySavedInEEPROM = DW1000Ng::getSavedAntennaDelay();
}

void loop() {
  
}
