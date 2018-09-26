/*  
 *	Arduino-DW1000Ng - Arduino library to use Decawave's DW1000Ng module.
 *	Copyright (C) 2018  Michele Biondi <michelebiondi01@gmail.com>, Andrea Salvatori <andrea.salvatori92@gmail.com>
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    DW1000Ng::setData(data);
    DW1000Ng::startTransmit();

    delay(120000); /* 2 minutes */

    /* used to stop transmission */
    DW1000Ng::softReset();
}

void loop() {
    
}
