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

char self_eui[24] = "AA:BB:CC:DD:EE:FF:00:00";

byte anchor_address[2];

// timestamps to remember
volatile uint64_t timePollSent;
volatile uint64_t timePollAckReceived;
volatile uint64_t timeRangeSent;
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

sleep_configuration_t SLEEP_CONFIG = {
    false,
    false,
    false,
    true,
    false,
    false,
    true,
    false,
    true,
    false,
    NULL
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
    DW1000Ng::applySleepConfiguration(SLEEP_CONFIG);
    DW1000Ng::enableFrameFiltering(TAG_FRAME_FILTER_CONFIG);
    
    DW1000Ng::setEUI(self_eui);

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
    transmitBlink();
    noteActivity();
}

void noteActivity() {
    // update activity timestamp, so that we do not reach "resetPeriod"
    lastActivity = millis();
}

void reset() {
    // tag returns to Idle and sends POLL
    DW1000Ng::forceTRxOff();
    transmitBlink();
    noteActivity();
}

void handleSent() {
    sentAck = true;
}

void handleReceived() {
    receivedAck = true;
}

void transmitBlink() {
    byte Blink[] = {BLINK, SEQ_NUMBER++, 0,0,0,0,0,0,0,0, NO_BATTERY_STATUS | NO_EX_ID, TAG_LISTENING_NOW};
    DW1000Ng::getEUI(&Blink[2]);
    DW1000Ng::setTransmitData(Blink, sizeof(Blink));
    DW1000Ng::startTransmit();
}

void transmitPoll() {
    byte Poll[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0 , RANGING_TAG_POLL};
    DW1000Ng::getNetworkId(&Poll[3]);
    memcpy(&Poll[5], anchor_address, 2);
    DW1000Ng::getDeviceAddress(&Poll[7]);
    DW1000Ng::setTransmitData(Poll, sizeof(Poll));
    DW1000Ng::startTransmit();
}


void transmitFinalMessage() {
    /* Calculation of future time */
    byte futureTimeBytes[LENGTH_TIMESTAMP];

	timeRangeSent = DW1000Ng::getSystemTimestamp();
	timeRangeSent += DW1000NgTime::microsecondsToUWBTime(replyDelayTimeUS);
    DW1000NgUtils::writeValueToBytes(futureTimeBytes, timeRangeSent, LENGTH_TIMESTAMP);
    DW1000Ng::setDelayedTRX(futureTimeBytes);
    timeRangeSent += DW1000Ng::getTxAntennaDelay();

    byte finalMessage[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0, RANGING_TAG_FINAL_RESPONSE_EMBEDDED, 
        0,0,0,0,0,0,0,0,0,0,0,0
    };

    DW1000Ng::getNetworkId(&finalMessage[3]);
    memcpy(&finalMessage[5], anchor_address, 2);
    DW1000Ng::getDeviceAddress(&finalMessage[7]);

    DW1000NgUtils::writeValueToBytes(finalMessage + 10, (uint32_t) timePollSent, 4);
    DW1000NgUtils::writeValueToBytes(finalMessage + 14, (uint32_t) timePollAckReceived, 4);
    DW1000NgUtils::writeValueToBytes(finalMessage + 18, (uint32_t) timeRangeSent, 4);
    DW1000Ng::setTransmitData(finalMessage, sizeof(finalMessage));
    DW1000Ng::startTransmit(TransmitMode::DELAYED);
}

void loop() {
    if (!sentAck && !receivedAck) {
        // check if inactive
        if (millis() - lastActivity > resetPeriod) {
            String tempString= "Time out! The lost anchor is:" ; tempString += (char)anchor_address[0] + (char)anchor_address[1];
            Serial.println(tempString);
            reset();
        }
        return;
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
        if(recv_data[9] == ACTIVITY_CONTROL) {
            if (recv_data[10] == RANGING_CONTINUE) {
                /* Received Response to poll */
                timePollSent = DW1000Ng::getTransmitTimestamp();
                timePollAckReceived = DW1000Ng::getReceiveTimestamp();
                transmitFinalMessage();
                String tempString= "Receiving messages from:" ; tempString += (char)anchor_address[0] + (char)anchor_address[1];
                tempString +=" and send it back.";
                Serial.println(tempString);
                noteActivity();
                return;
            } else if (recv_data[10] == RANGING_CONFIRM) {
                /* Received ranging confirm */
                memcpy(anchor_address, &recv_data[11], 2);
                transmitPoll();
                String tempString= "Sending messages to NEW Anchor:" ; tempString += (char) anchor_address[0] + (char)anchor_address[1];
                Serial.println(tempString);
                noteActivity();
                return;
            } else if(recv_data[10] == ACTIVITY_FINISHED) {
                resetPeriod = recv_data[11] + static_cast<uint32_t>(((recv_data[12] & 0x3F) << 8));
                byte multiplier = ((recv_data[12] & 0xC0) >> 6);
                if(multiplier  == 0x01) {
                    resetPeriod *= 25;
                } else if(multiplier == 0x02) {
                    resetPeriod *= 1000;
                }
                Serial.println("One Range is finishend, and the modules is going to sleep.");

                /* Sleep until next blink to save power */
                DW1000Ng::deepSleep();
                delay(resetPeriod);
                DW1000Ng::spiWakeup();
                DW1000Ng::setEUI(self_eui);

                transmitBlink();
                noteActivity();
                return;
            } else {
                reset();
                return;
            }
        }
        
        if(recv_data[15] == RANGING_INITIATION) {
            DW1000Ng::setDeviceAddress(DW1000NgUtils::bytesAsValue(&recv_data[16], 2));
            memcpy(anchor_address, &recv_data[13], 2);
            transmitPoll();
            String tempString= "Receiving messages from:" ; tempString += (char)anchor_address[0] + (char)anchor_address[1];
            tempString +=" and send it back.";
            Serial.println(tempString);
            noteActivity();
            return;
        }
    }
}