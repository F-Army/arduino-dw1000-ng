/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <DW1000Ng.hpp>

// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin

// data to send
byte data[] = { 0xC5, 0, 'D', 'E', 'C', 'A', 'W', 'A', 'V', 'E', 0, 0 };

device_configuration_t POWERTEST_CONFIG = {
    false,
    true,
    true,
    false,
    false,
    SFDMode::STANDARD_SFD,
    Channel::CHANNEL_5,
    DataRate::RATE_850KBPS,
    PulseFrequency::FREQ_16MHZ,
    PreambleLength::LEN_256,
    PreambleCode::CODE_3
};


void setup() {
    // DEBUG monitoring
    Serial.begin(115200);
    Serial.println(F("### DW1000Ng-transmit-spectrum-power-test ###"));
    // initialize the driver
    DW1000Ng::initialize(PIN_SS, PIN_IRQ, PIN_RST);
    Serial.println(F("DW1000Ng initialized ..."));
    // general configuration

    DW1000Ng::applyConfiguration(POWERTEST_CONFIG);

    /* Change according to test */
    DW1000Ng::setTXPower(0x25456585);
    DW1000Ng::setTCPGDelay(0xC0);

    Serial.println(F("Committed configuration ..."));
    Serial.println(F("Transmitting for calibration...."));
    
    DW1000Ng::enableTransmitPowerSpectrumTestMode(124800); /* Approx 1ms long transmissions */
    DW1000Ng::setTransmitData(data, sizeof(data));
    DW1000Ng::startTransmit();

    delay(120000); /* 2 minutes */

    Serial.println(F("End of transmission"));
    /* used to stop transmission */
    DW1000Ng::softwareReset();
}

void loop() {
    
}
