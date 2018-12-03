/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/* 
 * StandardRTLSTag_TWR.ino
 * 
 * This is an example tag in a RTLS using two way ranging ISO/IEC 24730-62_2013 messages
 */

#include <SPI.h>
#include <DW1000Ng.hpp>
#include <DW1000NgUtils.hpp>
#include <DW1000NgTime.hpp>
#include <DW1000NgConstants.hpp>
#include <DW1000NgRanging.hpp>
#include <DW1000NgRTLS.hpp>

// connection pins
#if defined(ESP8266)
const uint8_t PIN_SS = 15;
#else
const uint8_t PIN_SS = SS; // spi select pin
const uint8_t PIN_RST = 9;
#endif

volatile uint32_t blink_rate = 200;

typedef struct RangeInfrastructureResult {
    boolean success;
    uint16_t new_blink_rate;
} RangeInfrastructureResult;

device_configuration_t DEFAULT_CONFIG = {
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

frame_filtering_configuration_t TAG_FRAME_FILTER_CONFIG = {
    false,
    false,
    true,
    false,
    false,
    false,
    false,
    false
};

void setup() {
    // DEBUG monitoring
    Serial.begin(115200);
    Serial.println(F("### DW1000Ng-arduino-ranging-tag ###"));
    // initialize the driver
    #if defined(ESP8266)
    DW1000Ng::initializeNoInterrupt(PIN_SS);
    #else
    DW1000Ng::initializeNoInterrupt(PIN_SS, PIN_RST);
    #endif
    Serial.println("DW1000Ng initialized ...");
    // general configuration
    DW1000Ng::applyConfiguration(DEFAULT_CONFIG);
    DW1000Ng::enableFrameFiltering(TAG_FRAME_FILTER_CONFIG);
    
    DW1000Ng::setEUI("AA:BB:CC:DD:EE:FF:00:00");

    DW1000Ng::setNetworkId(RTLS_APP_ID);

    DW1000Ng::setAntennaDelay(16436);

    DW1000Ng::setPreambleDetectionTimeout(15);
    DW1000Ng::setSfdDetectionTimeout(273);
    DW1000Ng::setReceiveFrameWaitTimeoutPeriod(2000);
    
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
}

RangeInfrastructureResult rangeInfrastructure(uint16_t first_anchor) {
    RangeResult result = DW1000NgRTLS::range(first_anchor, 1500);
    if(!result.success) return {false , 0};

    while(result.success && result.next) {
        result = DW1000NgRTLS::range(result.next_anchor, 1500);
        if(!result.success) return {false , 0};

        #if defined(ESP8266)
        yield();
        #endif
    }

    if(result.success && result.new_blink_rate != 0) {
        return { true, result.new_blink_rate };
    } else {
        if(!result.success)
            return { false , 0 };
    }
};

RangeInfrastructureResult localizeTWR() {
    RangeRequestResult request_result = DW1000NgRTLS::rangeRequest();

    if(request_result.success) {
        
        RangeInfrastructureResult result = rangeInfrastructure(request_result.target_anchor);

        if(result.success)
            return result;
    }
    return {false, 0};
}


void loop() {
    DW1000Ng::deepSleep();
    delay(blink_rate);
    DW1000Ng::spiWakeup();
    DW1000Ng::setEUI("AA:BB:CC:DD:EE:FF:00:00");

    RangeInfrastructureResult res = localizeTWR();
    if(res.success)
        blink_rate = res.new_blink_rate;
}
