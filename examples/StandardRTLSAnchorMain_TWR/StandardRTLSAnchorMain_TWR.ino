/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/* 
 * StandardRTLSAnchorMain_TWR.ino
 * 
 * This is an example master anchor in a RTLS using two way ranging ISO/IEC 24730-62_2013 messages
 */

#include <SPI.h>
#include <DW1000Ng.hpp>
#include <DW1000NgUtils.hpp>
#include <DW1000NgRanging.hpp>
#include <DW1000NgRTLS.hpp>

typedef struct Position {
    double x;
    double y;
} Position;

// connection pins
#if defined(ESP8266)
const uint8_t PIN_SS = 15;
#else
const uint8_t PIN_RST = 9;
const uint8_t PIN_SS = SS; // spi select pin
#endif

// timestamps to remember
volatile uint64_t timePollReceived;

Position position_self = {0,0};
Position position_B = {3,0};
Position position_C = {3,2.5};

volatile double range_self;
volatile double range_B;
volatile double range_C;

// watchdog and reset period
volatile uint32_t lastActivity;
volatile uint32_t resetPeriod = 250;

byte target_eui[8];
byte tag_shortAddress[] = {0x05, 0x00};

byte anchor_b[] = {0x02, 0x00};
byte anchor_c[] = {0x03, 0x00};

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
    true /* This allows blink frames */
};

void setup() {
    // DEBUG monitoring
    Serial.begin(115200);
    Serial.println(F("### DW1000Ng-arduino-ranging-anchorMain ###"));
    // initialize the driver
    DW1000Ng::initializeNoInterrupt(PIN_SS, PIN_RST);
    Serial.println(F("DW1000Ng initialized ..."));
    // general configuration
    DW1000Ng::applyConfiguration(DEFAULT_CONFIG);
    DW1000Ng::enableFrameFiltering(ANCHOR_FRAME_FILTER_CONFIG);
    
    DW1000Ng::setEUI("AA:BB:CC:DD:EE:FF:00:01");

    DW1000Ng::setPreambleDetectionTimeout(16);
    DW1000Ng::setSfdDetectionTimeout(273);
    DW1000Ng::setReceiveFrameWaitTimeoutPeriod(6000);

    DW1000Ng::setNetworkId(RTLS_APP_ID);
    DW1000Ng::setDeviceAddress(1);
	
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

/* using https://math.stackexchange.com/questions/884807/find-x-location-using-3-known-x-y-location-using-trilateration */
void calculatePosition(double &x, double &y) {

    /* This gives for granted that the z plane is the same for anchor and tags */
    double A = ( (-2*position_self.x) + (2*position_B.x) );
    double B = ( (-2*position_self.y) + (2*position_B.y) );
    double C = (range_self*range_self) - (range_B*range_B) - (position_self.x*position_self.x) + (position_B.x*position_B.x) - (position_self.y*position_self.y) + (position_B.y*position_B.y);
    double D = ( (-2*position_B.x) + (2*position_C.x) );
    double E = ( (-2*position_B.y) + (2*position_C.y) );
    double F = (range_B*range_B) - (range_C*range_C) - (position_B.x*position_B.x) + (position_C.x*position_C.x) - (position_B.y*position_B.y) + (position_C.y*position_C.y);

    x = (C*E-F*B) / (E*A-B*D);
    y = (C*D-A*F) / (B*D-A*E);
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
 
void loop() {
    if(!receive()) return;

    size_t recv_len = DW1000Ng::getReceivedDataLength();
    byte recv_data[recv_len];
    DW1000Ng::getReceivedData(recv_data, recv_len);


    if(recv_data[0] == BLINK) {
        /* Is blink */
        DW1000NgRTLS::transmitRangingInitiation(&recv_data[2], tag_shortAddress);
        waitForTransmission();
        if(!receive()) return;

        size_t poll_len = DW1000Ng::getReceivedDataLength();
        byte poll_data[poll_len];
        DW1000Ng::getReceivedData(poll_data, poll_len);

        if(poll_len > 9 && poll_data[9] == RANGING_TAG_POLL) {
            timePollReceived = DW1000Ng::getReceiveTimestamp();
            DW1000NgRTLS::transmitResponseToPoll(&poll_data[7]);
            waitForTransmission();
            uint64_t timeResponseToPoll = DW1000Ng::getTransmitTimestamp();

            if(!receive()) return;

            size_t rfinal_len = DW1000Ng::getReceivedDataLength();
            byte rfinal_data[rfinal_len];
            DW1000Ng::getReceivedData(rfinal_data, rfinal_len);
            if(rfinal_len > 18 && rfinal_data[9] == RANGING_TAG_FINAL_RESPONSE_EMBEDDED) {
                uint64_t timeFinalMessageReceive = DW1000Ng::getReceiveTimestamp();

                DW1000NgRTLS::transmitRangingConfirm(&rfinal_data[7], anchor_b);

                range_self = DW1000NgRanging::computeRangeAsymmetric(
                    DW1000NgUtils::bytesAsValue(rfinal_data + 10, LENGTH_TIMESTAMP), // Poll send time
                    timePollReceived, 
                    timeResponseToPoll, // Response to poll sent time
                    DW1000NgUtils::bytesAsValue(rfinal_data + 14, LENGTH_TIMESTAMP), // Response to Poll Received
                    DW1000NgUtils::bytesAsValue(rfinal_data + 18, LENGTH_TIMESTAMP), // Final Message send time
                    timeFinalMessageReceive // Final message receive time
                );

                range_self = DW1000NgRanging::correctRange(range_self);
    
                /* In case of wrong read due to bad device calibration */
                if(range_self <= 0) 
                    range_self = 0.001;

                String rangeString = "Range: "; rangeString += range_self; rangeString += " m";
                rangeString += "\t RX power: "; rangeString += DW1000Ng::getReceivePower(); rangeString += " dBm";
                Serial.println(rangeString);

            }
        }

    } else if(recv_data[9] == 0x60) {
        double range = static_cast<double>(DW1000NgUtils::bytesAsValue(&recv_data[10],2) / 1000.0);
        String rangeReportString = "Range from: "; rangeReportString += recv_data[7];
        rangeReportString += " = "; rangeReportString += range;
        Serial.println(rangeReportString);
        if(recv_data[7] == anchor_b[0] && recv_data[8] == anchor_b[1]) {
            range_B = range;
        } else if(recv_data[7] == anchor_c[0] && recv_data[8] == anchor_c[1]){
            range_C = range;
            double x,y;
            calculatePosition(x,y);
            String positioning = "Found position - x: ";
            positioning += x; positioning +=" y: ";
            positioning += y;
            Serial.println(positioning);
        }
    }
}