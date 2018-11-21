/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/* 
 * StandardRTLSAnchorB_TWR.ino
 * 
 * This is an example slave anchor in a RTLS using two way ranging ISO/IEC 24730-62_2013 messages
 */

#include <SPI.h>
#include <DW1000Ng.hpp>
#include <DW1000NgUtils.hpp>
#include <DW1000NgRanging.hpp>
#include <DW1000NgRTLS.hpp>

typedef struct ContinueRangeResult {
    boolean success;
    double range;
} ContinueRangeResult;

// connection pins
#if defined(ESP8266)
const uint8_t PIN_SS = 15;
#else
const uint8_t PIN_RST = 9;
const uint8_t PIN_SS = SS; // spi select pin
#endif

byte main_anchor_address[] = {0x01, 0x00};
uint16_t next_anchor = 3;

double range_self;

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

frame_filtering_configuration_t ANCHOR_FRAME_FILTER_CONFIG = {
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
    Serial.println(F("### arduino-DW1000Ng-ranging-anchor-B ###"));
    // initialize the driver
    DW1000Ng::initializeNoInterrupt(PIN_SS, PIN_RST);
    Serial.println(F("DW1000Ng initialized ..."));
    // general configuration
    DW1000Ng::applyConfiguration(DEFAULT_CONFIG);
    DW1000Ng::enableFrameFiltering(ANCHOR_FRAME_FILTER_CONFIG);
    
    DW1000Ng::setEUI("AA:BB:CC:DD:EE:FF:00:02");

    DW1000Ng::setPreambleDetectionTimeout(16);
    DW1000Ng::setSfdDetectionTimeout(273);
    DW1000Ng::setReceiveFrameWaitTimeoutPeriod(6000);

    DW1000Ng::setNetworkId(RTLS_APP_ID);
    DW1000Ng::setDeviceAddress(2);
	
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
  
}

void transmitRangeReport() {
    byte rangingReport[] = {DATA, SHORT_SRC_AND_DEST, DW1000NgRTLS::increaseSequenceNumber(), 0,0, 0,0, 0,0, 0x60, 0,0 };
    DW1000Ng::getNetworkId(&rangingReport[3]);
    memcpy(&rangingReport[5], main_anchor_address, 2);
    DW1000Ng::getDeviceAddress(&rangingReport[7]);
    DW1000NgUtils::writeValueToBytes(&rangingReport[10], static_cast<uint16_t>((range_self*1000)), 2);
    DW1000Ng::setTransmitData(rangingReport, sizeof(rangingReport));
    DW1000Ng::startTransmit();
}
 
boolean receive() {
    DW1000Ng::startReceive();
    while(!DW1000Ng::isReceiveDone()) {
        if(DW1000Ng::isReceiveTimeout() ) {
            DW1000Ng::clearReceiveTimeoutStatus();
            return false;
        }
    }
    DW1000Ng::clearReceiveStatus();
    return true;
}

void waitForTransmission() {
    while(!DW1000Ng::isTransmitDone()) {}
    DW1000Ng::clearTransmitStatus();
}

ContinueRangeResult continueRange(NextActivity next, uint16_t value) {
    double range;
    if(!receive()) return {false, 0};

    size_t poll_len = DW1000Ng::getReceivedDataLength();
    byte poll_data[poll_len];
    DW1000Ng::getReceivedData(poll_data, poll_len);

    if(poll_len > 9 && poll_data[9] == RANGING_TAG_POLL) {
        uint64_t timePollReceived = DW1000Ng::getReceiveTimestamp();
        DW1000NgRTLS::transmitResponseToPoll(&poll_data[7]);
        waitForTransmission();
        uint64_t timeResponseToPoll = DW1000Ng::getTransmitTimestamp();

        if(!receive()) return {false, 0};

        size_t rfinal_len = DW1000Ng::getReceivedDataLength();
        byte rfinal_data[rfinal_len];
        DW1000Ng::getReceivedData(rfinal_data, rfinal_len);
        if(rfinal_len > 18 && rfinal_data[9] == RANGING_TAG_FINAL_RESPONSE_EMBEDDED) {
            uint64_t timeFinalMessageReceive = DW1000Ng::getReceiveTimestamp();

            byte finishValue[2];
            DW1000NgUtils::writeValueToBytes(finishValue, value, 2);

            if(next == NextActivity::RANGING_CONFIRM)
                DW1000NgRTLS::transmitRangingConfirm(&rfinal_data[7], finishValue);
            else
                DW1000NgRTLS::transmitActivityFinished(&rfinal_data[7], finishValue);

            range = DW1000NgRanging::computeRangeAsymmetric(
                DW1000NgUtils::bytesAsValue(rfinal_data + 10, LENGTH_TIMESTAMP), // Poll send time
                timePollReceived, 
                timeResponseToPoll, // Response to poll sent time
                DW1000NgUtils::bytesAsValue(rfinal_data + 14, LENGTH_TIMESTAMP), // Response to Poll Received
                DW1000NgUtils::bytesAsValue(rfinal_data + 18, LENGTH_TIMESTAMP), // Final Message send time
                timeFinalMessageReceive // Final message receive time
            );

            range = DW1000NgRanging::correctRange(range);

            /* In case of wrong read due to bad device calibration */
            if(range <= 0) 
                range = 0.000001;

            return {true, range};
        }
    }
}
 
void loop() {     
        ContinueRangeResult result = continueRange(NextActivity::RANGING_CONFIRM, next_anchor);
        if(result.success) {
            range_self = result.range;
            waitForTransmission();
            transmitRangeReport();

            String rangeString = "Range: "; rangeString += range_self; rangeString += " m";
            rangeString += "\t RX power: "; rangeString += DW1000Ng::getReceivePower(); rangeString += " dBm";
            Serial.println(rangeString);
        }
}
