#include <SPI.h>
#include <DWM1000.hpp>

// connection pins
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = SS; // spi select pin

// data to send
String data = "DECAWAVE";


void setup() {
    // DEBUG monitoring
    Serial.begin(115200);
    Serial.println(F("### DWM1000-transmit-spectrum-power-test ###"));
    // initialize the driver
    DWM1000::begin(PIN_IRQ, PIN_RST);
    DWM1000::select(PIN_SS);
    Serial.println("DWM1000 initialized ...");
    // general configuration
    DWM1000::newConfiguration();
    DWM1000::setDeviceAddress(1);
    DWM1000::setNetworkId(10);
    DWM1000::setChannel(CHANNEL_5);
    DWM1000::enableMode(MODE_SHORTRANGE_HIGHPRF_MEDIUMPREAMBLE);
    DWM1000::setSFDMode(SFDMODE::STANDARD_SFD);
    DWM1000::setPreambleCode(9);
    DWM1000::useSmartPower(false);
    DWM1000::setTXPower(0x25456585);
    DWM1000::setTCPGDelay(0xC0);
    DWM1000::suppressFrameCheck(true);
    DWM1000::commitConfiguration();
    Serial.println(F("Committed configuration ..."));
    // DEBUG chip info and registers pretty printed
    char msg[128];
    DWM1000::getPrintableDeviceIdentifier(msg);
    Serial.print("Device ID: "); Serial.println(msg);
    DWM1000::getPrintableExtendedUniqueIdentifier(msg);
    Serial.print("Unique ID: "); Serial.println(msg);
    DWM1000::getPrintableNetworkIdAndShortAddress(msg);
    Serial.print("Network ID & Device Address: "); Serial.println(msg);
    DWM1000::getPrintableDeviceMode(msg);
    Serial.print("Device mode: "); Serial.println(msg);
    DWM1000::newTransmit();
    DWM1000::enableTransmitPowerSpectrumTestMode(124800);
    DWM1000::setData(data);
    DWM1000::startTransmit();

    delay(120000);

    DWM1000::softReset();
}

void loop() {
    
}

