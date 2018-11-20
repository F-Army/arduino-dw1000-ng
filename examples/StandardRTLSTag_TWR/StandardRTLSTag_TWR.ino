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

typedef struct RangeRequestResult {
    boolean success;
    uint16_t target_anchor;
} RangeRequestResult;


typedef struct RangeResult {
    boolean success;
    boolean next;
    uint16_t next_anchor;
    uint32_t new_blink_rate;
} RangeResult;

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

    DW1000Ng::setPreambleDetectionTimeout(16);
    DW1000Ng::setSfdDetectionTimeout(273);
    DW1000Ng::setReceiveFrameWaitTimeoutPeriod(3000);
    
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

void waitForTransmission() {
    while(!DW1000Ng::isTransmitDone()) {}
    DW1000Ng::clearTransmitStatus();
}

boolean receive() {
    DW1000Ng::startReceive();
    unsigned long timeout = micros();
    while(!DW1000Ng::isReceiveDone()) {
        if(DW1000Ng::isReceiveTimeout() ) {
            DW1000Ng::clearReceiveTimeoutStatus();
            Serial.println("to");
            return false;
        }
    }
    DW1000Ng::clearReceiveStatus();
    return true;
}

boolean isRangingInitiation(byte recv_data[], size_t recv_len) {
    return (recv_len > 17 && recv_data[15] == RANGING_INITIATION) ? true : false;
}

boolean nextRangingStep() {
    waitForTransmission();
    if(!receive()) return false;
    return true;
}

RangeResult range(uint16_t anchor, uint16_t replyDelayUs) {
    byte target_anchor[2];
    DW1000NgUtils::writeValueToBytes(target_anchor, anchor, 2);
    DW1000NgRTLS::transmitPoll(target_anchor);
    /* Start of poll control for range */
    if(!nextRangingStep()) return {false, false, 0, 0};
    size_t cont_len = DW1000Ng::getReceivedDataLength();
    byte cont_recv[cont_len];
    DW1000Ng::getReceivedData(cont_recv, cont_len);

    if (cont_len > 10 && cont_recv[9] == ACTIVITY_CONTROL && cont_recv[10] == RANGING_CONTINUE) {
        /* Received Response to poll */
        DW1000NgRTLS::handleRangingContinueEmbedded(cont_recv, replyDelayUs);
    } else {
        return {false, false, 0, 0};
    }

    if(!nextRangingStep()) return {false, false, 0, 0};

    size_t act_len = DW1000Ng::getReceivedDataLength();
    byte act_recv[act_len];
    DW1000Ng::getReceivedData(act_recv, act_len);

    if(act_len > 10 && act_recv[9] == ACTIVITY_CONTROL) {
        if (act_len > 12 && act_recv[10] == RANGING_CONFIRM) {
            return {true, true, DW1000NgUtils::bytesAsValue(&act_recv[11], 2), 0};
        } else if(act_len > 12 && act_recv[10] == ACTIVITY_FINISHED) {
            blink_rate = DW1000NgRTLS::handleActivityFinished(act_recv);
            return {true, false, 0, blink_rate};
        }
    } else {
        return {false, false, 0, 0};
    }
    /* end of ranging */
}

RangeRequestResult rangeRequest() {
    DW1000NgRTLS::transmitTwrShortBlink();
    
    if(!nextRangingStep()) return {false, 0};

    size_t init_len = DW1000Ng::getReceivedDataLength();
    byte init_recv[init_len];
    DW1000Ng::getReceivedData(init_recv, init_len);

    if(!isRangingInitiation(init_recv, init_len)) {
        return { false, 0};
    }

    DW1000Ng::setDeviceAddress(DW1000NgUtils::bytesAsValue(&init_recv[16], 2));
    return { true, DW1000NgUtils::bytesAsValue(&init_recv[13], 2) };
}

RangeInfrastructureResult rangeInfrastructure(uint16_t first_anchor) {
    RangeResult result = range(first_anchor, 3000);
    if(!result.success) return {false , 0};

    while(result.success && result.next) {
        result = range(result.next_anchor,3000);
        if(!result.success) return {false , 0};
    }

    if(result.success && result.new_blink_rate != 0) {
        return { true, result.new_blink_rate };
    } else {
        if(!result.success)
            return { false , 0 };
    }
};


void loop() {
    DW1000Ng::deepSleep();
    delay(blink_rate);
    DW1000Ng::spiWakeup();
    DW1000Ng::setEUI("AA:BB:CC:DD:EE:FF:00:00");


    RangeRequestResult request_result = rangeRequest();

    if(request_result.success) {
        
        RangeInfrastructureResult result = rangeInfrastructure(request_result.target_anchor);

        if(result.success)
            blink_rate = result.new_blink_rate;
    }
}
