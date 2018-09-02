/**
 * Copyright (c) 2018 Michele 'an4cr0n' Biondi <michelebiondi01@gmail.com> 
 * Copyright (c) 2018 Andrea 'Sonic0' Salvatori <andrea.salvatori92@gmail.com>
 * Decawave DWM1000 driver for arduino.
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
 * @file TransmitSpectrumPowerCalibrationTest.ino
 * This sketch is used for regulatory power testing.
 */

#include <SPI.h>
#include <DWM1000.hpp>

// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin

// data to send
String data = "DECAWAVE";


void setup() {
    // DEBUG monitoring
    Serial.begin(115200);
    Serial.println(F("### DWM1000-transmit-spectrum-power-test ###"));
    // initialize the driver
    DWM1000::begin(PIN_IRQ, PIN_RST);
    DWM1000::select(PIN_SS);
    Serial.println("DWM1000 initialized ...");
    // general configuration
    DWM1000::newConfiguration();
    DWM1000::setDeviceAddress(1);
    DWM1000::setNetworkId(10);
    DWM1000::setChannel(CHANNEL_5);
    DWM1000::enableMode(MODE_SHORTRANGE_HIGHPRF_MEDIUMPREAMBLE);
    DWM1000::setSFDMode(SFDMode::STANDARD_SFD);
    DWM1000::setPreambleCode(PREAMBLE_CODE_64MHZ_9);
    DWM1000::useSmartPower(false);
    DWM1000::setTXPower(0x25456585); /* Change according to test */
    DWM1000::setTCPGDelay(0xC0); /* Change according to test */
    DWM1000::suppressFrameCheck(true);
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
    DWM1000::newTransmit();
    DWM1000::enableTransmitPowerSpectrumTestMode(124800); /* Approx 1ms long transmissions */
    DWM1000::setData(data);
    DWM1000::startTransmit();

    delay(120000); /* 2 minutes */

    /* used to stop transmission */
    DWM1000::softReset();
}

void loop() {
    
}

