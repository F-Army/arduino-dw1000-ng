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
const uint8_t PIN_SS = SS; // spi select pin

// reply times (same on both sides for symm. ranging)
uint16_t replyDelayTimeUS = 3000;

volatile uint32_t blink_rate = 200;

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
    DW1000Ng::initialize(PIN_SS);
    Serial.println("DW1000Ng initialized ...");
    // general configuration
    DW1000Ng::applyConfiguration(DEFAULT_CONFIG);
    DW1000Ng::enableFrameFiltering(TAG_FRAME_FILTER_CONFIG);
    
    DW1000Ng::setEUI("AA:BB:CC:DD:EE:FF:00:00");

    DW1000Ng::setNetworkId(RTLS_APP_ID);

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
    // anchor starts by transmitting a POLL message
    
}

void loop() {

    DW1000NgRTLS::transmitTwrShortBlink();

    while(!DW1000Ng::isTransmitDone()) {
        Serial.print("0");
    }
    DW1000Ng::clearTransmitStatus();

    DW1000Ng::startReceive();
    while(!DW1000Ng::isReceiveDone()) {
        Serial.print("1");
    }
    DW1000Ng::clearReceiveStatus();

    size_t init_len = DW1000Ng::getReceivedDataLength();
    byte init_recv[init_len];
    DW1000Ng::getReceivedData(init_recv, init_len);

    if(init_len > 17 && init_recv[15] == RANGING_INITIATION) {
        DW1000NgRTLS::handleRangingInitiation(init_recv);
    } else {
        Serial.print("No initiation");
        return;
    }

    while(!DW1000Ng::isTransmitDone()) {
        Serial.print("2");
    }
    DW1000Ng::clearTransmitStatus();

    DW1000Ng::startReceive();
    while(!DW1000Ng::isReceiveDone()) {
        Serial.print("3");
    }
    DW1000Ng::clearReceiveStatus();

    size_t cont_len = DW1000Ng::getReceivedDataLength();
    byte cont_recv[cont_len];
    DW1000Ng::getReceivedData(cont_recv, cont_len);

    if (cont_len > 10 && cont_recv[9] == ACTIVITY_CONTROL && cont_recv[10] == RANGING_CONTINUE) {
        /* Received Response to poll */
        DW1000NgRTLS::handleRangingContinueEmbedded(cont_recv, replyDelayTimeUS);
    } else {
        Serial.print("No continue");
        return;
    }

    while(!DW1000Ng::isTransmitDone()) {
        Serial.print("4");
    }
    DW1000Ng::clearTransmitStatus();

    DW1000Ng::startReceive();
    while(!DW1000Ng::isReceiveDone()) {
        Serial.print("5");
    }
    DW1000Ng::clearReceiveStatus();

    size_t act_len = DW1000Ng::getReceivedDataLength();
    byte act_recv[act_len];
    DW1000Ng::getReceivedData(act_recv, act_len);

    if(act_len > 10 && act_recv[9] == ACTIVITY_CONTROL) {
        if (act_len > 12 && act_recv[10] == RANGING_CONFIRM) {
            /* Received ranging confirm */
            DW1000NgRTLS::handleRangingConfirm(act_recv);
        } else if(act_len > 12 && act_recv[10] == ACTIVITY_FINISHED) {
            blink_rate = DW1000NgRTLS::handleActivityFinished(act_recv); 
        }
    } else {
        Serial.print("No act control");
    }

    /* Sleep until next blink to save power */
    DW1000Ng::deepSleep();
    delay(blink_rate);
    DW1000Ng::spiWakeup();
}