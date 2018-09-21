/*  
 *	Arduino-DWM1000 - Arduino library to use Decawave's DWM1000 module.
 *	Copyright (C) 2018  Michele Biondi <michelebiondi01@gmail.com>, Andrea Salvatori <andrea.salvatori92@gmail.com>
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <Arduino.h>
#include "DWM1000Configuration.hpp"
#include "DWM1000Registers.hpp"
#include "DWM1000Time.hpp"

class DWM1000Device {
public:

    /* Constructors */
    DWM1000Device(uint8_t ss, uint8_t irq, uint8_t rst, uint16_t shortAddress);
    DWM1000Device(uint8_t ss, uint8_t irq, uint8_t rst, uint16_t shortAddress, DWM1000Configuration config);
    DWM1000Device(uint8_t ss, uint8_t irq, uint8_t rst, uint16_t shortAddress, uint16_t panId);
    DWM1000Device(uint8_t ss, uint8_t irq, uint8_t rst, uint16_t shortAddress, uint16_t panId, DWM1000Configuration config);
    DWM1000Device(uint8_t ss, uint8_t irq, uint8_t rst, uint16_t shortAddress, uint16_t panId, char eui[]);
    DWM1000Device(uint8_t ss, uint8_t irq, uint8_t rst, uint16_t shortAddress, uint16_t panId, char eui[], DWM1000Configuration config);
    
    /* Destructors */
    ~DWM1000Device();

    /* Getters */
    uint8_t getSS();
    uint8_t getIRQ();
    uint8_t getRST();
    uint16_t getShortAddress();
    uint16_t getPanId();
    void getEUI(char *);
    DWM1000Configuration getConfiguration();

    DWM1000Time getLastSendTimestamp();
    DWM1000Time getLastReceiveTimestamp();

    void getReceiveData(byte data[], uint16_t n);
    void getReceiveData(String& data);

    
    /* Setters */
    void setShortAddress(uint16_t shortAddress);
    void setPanId(uint16_t panId);
    void setEUI(char eui[]);
    void setConfiguration(DWM1000Configuration config);

    void setErrorHandler(void (* handleError)(void));
	void setSentHandler(void (* handleSent)(void));
	void setReceivedHandler(void (* handleReceived)(void));
	void setReceiveFailedHandler(void (* handleReceiveFailed)(void));
	void setReceiveTimeoutHandler(void (* handleReceiveTimeout)(void));
	void setReceiveTimestampAvailableHandler(void (* handleReceiveTimestampAvailable)(void));

    /* Methods */
    void select();
    void end();

    void transmit(byte data[], uint16_t length);
    void transmit(const String& data);
    DWM1000Time transmitDelayed(byte data[], uint16_t length, uint16_t delayMicroSeconds);
    DWM1000Time transmitDelayed(const String& data, uint16_t delayMicroSeconds);

    void receive();
    void receiveDelayed(uint16_t delayMicroSeconds);

    void forceIdle();

    /*
    //TODO
    void sleep();
    void deepSleep();
    void reset();
    void softreset();
    void getBatteryVoltage();
    void getTemperature();
    */

private:

    static int SPICurrentDevice;
    
    uint8_t _ss;
    uint8_t _irq;
    uint8_t _rst;
    uint16_t _shortAddress;
    uint16_t _panId;
    char _eui[LEN_EUI];
    DWM1000Configuration _configuration;

    void (* _handleSent)(void);
	void (* _handleError)(void);
	void (* _handleReceived)(void);
	void (* _handleReceiveFailed)(void);
	void (* _handleReceiveTimeout)(void);
	void (* _handleReceiveTimestampAvailable)(void);

};