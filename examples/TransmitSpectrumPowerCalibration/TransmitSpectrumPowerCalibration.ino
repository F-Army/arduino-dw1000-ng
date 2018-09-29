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

// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin

// data to send
byte data[] = { 0xC5, 0, 'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E', 0, 0 };


void setup() {
    // DEBUG monitoring
    Serial.begin(115200);
    Serial.println(F("### DW1000Ng-transmit-spectrum-power-test ###"));
    // initialize the driver
    DW1000Ng::begin(PIN_SS, PIN_IRQ, PIN_RST);
    Serial.println(F("DW1000Ng initialized ..."));
    // general configuration
    DW1000Ng::newConfiguration();

    /* Change according to test */
    DW1000Ng::setDeviceAddress(1);
    DW1000Ng::setNetworkId(10);
    DW1000Ng::setSFDMode(SFDMode::STANDARD_SFD);
    DW1000Ng::setChannel(Channel::CHANNEL_5);
    DW1000Ng::setDataRate(DataRate::RATE_850KBPS);
    DW1000Ng::setPulseFrequency(PulseFrequency::FREQ_16MHZ);
    DW1000Ng::setPreambleLength(PreambleLength::LEN_256);
    DW1000Ng::setPreambleCode(PreambleCode::CODE_3);
    DW1000Ng::useSmartPower(false);
    DW1000Ng::setTXPower(0x25456585);
    DW1000Ng::setTCPGDelay(0xC0);
    DW1000Ng::suppressFrameCheck(true);
    
    DW1000Ng::commitConfiguration();
    Serial.println(F("Committed configuration ..."));
    
    DW1000Ng::enableTransmitPowerSpectrumTestMode(124800); /* Approx 1ms long transmissions */
    DW1000Ng::setTransmitData(data);
    DW1000Ng::startTransmit();

    delay(120000); /* 2 minutes */

    /* used to stop transmission */
    DW1000Ng::softReset();
}

void loop() {
    
}
