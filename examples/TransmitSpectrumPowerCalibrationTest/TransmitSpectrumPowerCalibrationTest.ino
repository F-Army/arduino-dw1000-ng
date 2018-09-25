/*  
 *	Arduino-DWM1000 - Arduino library to use Decawave's DWM1000 module.
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
#include <DWM1000.hpp>

// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin

// data to send
byte data[] = { 0xC5, 0, 'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E', 0, 0 };


void setup() {
    // DEBUG monitoring
    Serial.begin(115200);
    Serial.println(F("### DWM1000-transmit-spectrum-power-test ###"));
    // initialize the driver
    DWM1000::begin(PIN_SS, PIN_IRQ, PIN_RST);
    Serial.println(F("DWM1000 initialized ..."));
    // general configuration
    DWM1000::newConfiguration();

    /* Change according to test */
    DWM1000::setDeviceAddress(1);
    DWM1000::setNetworkId(10);
    DWM1000::setSFDMode(SFDMode::STANDARD_SFD);
    DWM1000::setChannel(Channel::CHANNEL_5);
    DWM1000::setDataRate(DataRate::RATE_850KBPS);
    DWM1000::setPulseFrequency(PulseFrequency::FREQ_16MHZ);
    DWM1000::setPreambleLength(PreambleLength::LEN_256);
    DWM1000::setPreambleCode(PreambleCode::CODE_3);
    DWM1000::useSmartPower(false);
    DWM1000::setTXPower(0x25456585);
    DWM1000::setTCPGDelay(0xC0);
    DWM1000::suppressFrameCheck(true);
    
    DWM1000::commitConfiguration();
    Serial.println(F("Committed configuration ..."));
    
    DWM1000::enableTransmitPowerSpectrumTestMode(124800); /* Approx 1ms long transmissions */
    DWM1000::setData(data);
    DWM1000::startTransmit();

    delay(120000); /* 2 minutes */

    /* used to stop transmission */
    DWM1000::softReset();
}

void loop() {
    
}
