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
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin

// message flow state
// message sent/received state
volatile boolean sentAck = false;
volatile boolean receivedAck = false;

byte SEQ_NUMBER = 0;

// timestamps to remember
volatile uint64_t timePollSent;
volatile uint64_t timePollAckReceived;
// watchdog and reset period
volatile uint32_t lastActivity;
volatile uint32_t resetPeriod = 250;
// reply times (same on both sides for symm. ranging)
uint16_t replyDelayTimeUS = 3000;

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

interrupt_configuration_t DEFAULT_INTERRUPT_CONFIG = {
    true,
    true,
    false,
    false,
    false,
    false
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
    DW1000Ng::initialize(PIN_SS, PIN_IRQ, PIN_RST);
    Serial.println("DW1000Ng initialized ...");
    // general configuration
    DW1000Ng::applyConfiguration(DEFAULT_CONFIG);
	DW1000Ng::applyInterruptConfiguration(DEFAULT_INTERRUPT_CONFIG);
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
    // attach callback for (successfully) sent and received messages
    DW1000Ng::attachSentHandler(handleSent);
    DW1000Ng::attachReceivedHandler(handleReceived);
    // anchor starts by transmitting a POLL message
    DW1000NgRTLS::transmitShortBlink();
    noteActivity();
}

void noteActivity() {
    // update activity timestamp, so that we do not reach "resetPeriod"
    lastActivity = millis();
}

void reset() {
    // tag returns to Idle and sends POLL
    DW1000Ng::forceTRxOff();
    DW1000NgRTLS::transmitShortBlink();
    noteActivity();
}

void handleSent() {
    sentAck = true;
}

void handleReceived() {
    receivedAck = true;
}

void loop() {
    if (!sentAck && !receivedAck) {
        // check if inactive
        if (millis() - lastActivity > resetPeriod) {
            reset();
        }
    }

    if (sentAck) {
        sentAck = false;
        DW1000Ng::startReceive();
    }

    if (receivedAck) {
        receivedAck = false;
        /* Parse received message */
        size_t recv_len = DW1000Ng::getReceivedDataLength();
        byte recv_data[recv_len];
        DW1000Ng::getReceivedData(recv_data, recv_len);

        /* RTLS standard message */
        if(recv_len > 10 && recv_data[9] == ACTIVITY_CONTROL) {
            if (recv_data[10] == RANGING_CONTINUE) {
                /* Received Response to poll */
                timePollSent = DW1000Ng::getTransmitTimestamp();
                timePollAckReceived = DW1000Ng::getReceiveTimestamp();
                DW1000NgRTLS::transmitFinalMessage(&recv_data[7], replyDelayTimeUS, timePollSent, timePollAckReceived);
                noteActivity();
            } else if (recv_len > 12 && recv_data[10] == RANGING_CONFIRM) {
                /* Received ranging confirm */
                DW1000NgRTLS::transmitPoll(&recv_data[11]);
                noteActivity();
            } else if(recv_len > 12 && recv_data[10] == ACTIVITY_FINISHED) {
                resetPeriod = recv_data[11] + static_cast<uint32_t>(((recv_data[12] & 0x3F) << 8));
                byte multiplier = ((recv_data[12] & 0xC0) >> 6);
                if(multiplier  == 0x01) {
                    resetPeriod *= 25;
                } else if(multiplier == 0x02) {
                    resetPeriod *= 1000;
                }

                /* Sleep until next blink to save power */
                DW1000Ng::deepSleep();
                delay(resetPeriod);
                DW1000Ng::spiWakeup();

                DW1000NgRTLS::transmitShortBlink();
                noteActivity();
            }
        } else if(recv_len > 17 && recv_data[15] == RANGING_INITIATION) {
            DW1000Ng::setDeviceAddress(DW1000NgUtils::bytesAsValue(&recv_data[16], 2));
            DW1000NgRTLS::transmitPoll(&recv_data[13]);
            noteActivity();
        }
        
        
    }
}