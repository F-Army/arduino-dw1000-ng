/*
 * MIT License
 * 
 * Copyright (c) 2018 Michele Biondi, Andrea Salvatori
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#pragma once

#include <Arduino.h>

#include "DW1000NgRegisters.hpp"
#include "DW1000NgTime.hpp"
#include "DW1000NgDeviceConfiguration.hpp"

typedef struct device_interrupt_map_t {
    boolean interruptOnSent;
    boolean interruptOnReceived;
    boolean interruptOnReceiveFailed;
    boolean interruptOnReceiveTimeout;
    boolean interruptOnReceiveTimestampAvailable;
    boolean interruptOnAutomaticAcknowledgeTrigger;
} device_interrupt_map_t;

class DW1000NgDevice {
public:

    /* CONSTRUCTORS */
    DW1000NgDevice(DW1000NgDeviceConfiguration config, device_interrupt_map_t int_map, uint8_t ss,  uint8_t irq, uint8_t rst);
    DW1000NgDevice(DW1000NgDeviceConfiguration config, device_interrupt_map_t int_map, uint8_t ss,  uint8_t irq);
    DW1000NgDevice(DW1000NgDeviceConfiguration config, uint8_t ss,  uint8_t irq, uint8_t rst);
    DW1000NgDevice(DW1000NgDeviceConfiguration config, uint8_t ss,  uint8_t irq);
    DW1000NgDevice(device_interrupt_map_t int_map, uint8_t ss,  uint8_t irq, uint8_t rst);
    DW1000NgDevice(device_interrupt_map_t int_map, uint8_t ss,  uint8_t irq);
    DW1000NgDevice(uint8_t ss,  uint8_t irq, uint8_t rst);
    DW1000NgDevice(uint8_t ss,  uint8_t irq);

    /* DESTRUCTORS */
    ~DW1000NgDevice();

    /* METHODS */
    void select();
    void end();

    void transmit(byte data[], uint16_t length);
    void transmit(const String& data);
    void transmitDelayed(byte data[], uint16_t length, uint16_t delayMicroSeconds);
    void transmitDelayed(const String& data, uint16_t delayMicroSeconds);
    DW1000NgTime retrieveLastTransmitTimestamp();

    void receive();
    void receiveDelayed(uint16_t delayMicroSeconds);
    void retrieveReceivedData(byte data[], uint16_t n);
    void retrieveReceivedData(String& data);
    DW1000NgTime retrieveLastReceiveTimestamp();

    void forceIdle();

    /* GETTERS AND SETTERS */
    DW1000NgDeviceConfiguration getConfiguration();
    void setConfiguration(DW1000NgDeviceConfiguration config);
    void setConfiguration(DeviceConfigurationProfile profile);

    void setShortAddress(uint16_t shortAddress);
    uint16_t getShortAddress();

    void setPanId(uint16_t panId);
    uint16_t getPanId();

    void setEUI(char eui[]);
    String getEUI();
    
    device_interrupt_map_t getInterruptMap();
    void setInterruptMap(device_interrupt_map_t int_map);

    void setErrorHandler(void (* handleError)(void));
	void setSentHandler(void (* handleSent)(void));
	void setReceivedHandler(void (* handleReceived)(void));
	void setReceiveFailedHandler(void (* handleReceiveFailed)(void));
	void setReceiveTimeoutHandler(void (* handleReceiveTimeout)(void));
	void setReceiveTimestampAvailableHandler(void (* handleReceiveTimestampAvailable)(void));

    /* //TODO
        void sleep();
        void deepSleep();
        void hardwareReset();
        void softwareReset();
        void getBatteryVoltage();
        void getTemperature();
    */

private:
    DW1000NgDeviceConfiguration _config;
    device_interrupt_map_t _interrupt_map;
    uint16_t _shortAddress;
    uint16_t _panId;
    char _eui[LEN_EUI];
};
