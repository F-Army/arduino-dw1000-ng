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
 * @file BasicConnectivityTest.ino
 * Use this to test connectivity with your DWM1000 from Arduino.
 * It performs an arbitrary setup of the chip and prints some information.
 * 
 * @todo
 *  - move strings to flash (less RAM consumption)
 *  - make real check of connection (e.g. change some values on DWM1000 and verify)
 */

#include <SPI.h>
#include <DWM1000.hpp>

// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin

void setup() {
  // DEBUG monitoring
  Serial.begin(9600);
  // initialize the driver
  DWM1000.begin(PIN_IRQ, PIN_RST);
  DWM1000.select(PIN_SS);
  Serial.println(F("DWM1000 initialized ..."));
  // general configuration
  DWM1000.newConfiguration();
  DWM1000.setDeviceAddress(5);
  DWM1000.setNetworkId(10);
  DWM1000.commitConfiguration();
  Serial.println(F("Committed configuration ..."));
  // wait a bit
  delay(1000);
}

void loop() {
  // DEBUG chip info and registers pretty printed
  char msg[128];
  DWM1000.getPrintableDeviceIdentifier(msg);
  Serial.print("Device ID: "); Serial.println(msg);
  DWM1000.getPrintableExtendedUniqueIdentifier(msg);
  Serial.print("Unique ID: "); Serial.println(msg);
  DWM1000.getPrintableNetworkIdAndShortAddress(msg);
  Serial.print("Network ID & Device Address: "); Serial.println(msg);
  DWM1000.getPrintableDeviceMode(msg);
  Serial.print("Device mode: "); Serial.println(msg);
  // wait a bit
  delay(10000);
}
