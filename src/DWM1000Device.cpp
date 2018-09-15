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

#include <Arduino.h>
#include "DWM1000.hpp"
#include "DWM1000Device.hpp"

DWM1000Device::DWM1000Device(uint8_t rst, uint8_t irq, uint8_t ss, uint16_t shortAddress) {
    _rst = rst;
    _irq = irq;
    _ss = ss;
    _shortAddress = shortAddress;
    _panId = 0xDECA;
}

DWM1000Device::DWM1000Device(uint8_t rst, uint8_t irq, uint8_t ss, uint16_t shortAddress, uint16_t panId) {
    _rst = rst;
    _irq = irq;
    _ss = ss;
    _shortAddress = shortAddress;
    _panId = panId; 
}

DWM1000Device::~DWM1000Device() { }

void DWM1000Device::init() {
    DWM1000::begin(_irq, _rst);
    DWM1000::select(_ss);
    DWM1000::newConfiguration();
    DWM1000::setDefaults();
    DWM1000::setDeviceAddress(_shortAddress);
    DWM1000::setNetworkId(_panId);
    DWM1000::commitConfiguration();
}

void DWM1000Device::init(uint16_t antennaDelay) {
    DWM1000::begin(_irq, _rst);
    DWM1000::select(_ss);
    DWM1000::newConfiguration();
    DWM1000::setDefaults();
    DWM1000::setAntennaDelay(antennaDelay);
    DWM1000::setDeviceAddress(_shortAddress);
    DWM1000::setNetworkId(_panId);
    DWM1000::commitConfiguration();
}

void DWM1000Device::setShortAddress(uint16_t shortAddress) {
    _shortAddress = shortAddress;
    DWM1000::setDeviceAddress(_shortAddress);
    DWM1000::commitConfiguration();
}

void DWM1000Device::setPanId(uint16_t panId) {
    _panId = panId;
    DWM1000::setNetworkId(_panId);
    DWM1000::commitConfiguration();
}

void DWM1000Device::setAntennaDelay(uint16_t antennaDelay) {
    _antennaDelay = antennaDelay;
    DWM1000::setAntennaDelay(_antennaDelay);
    DWM1000::commitConfiguration();
}

void DWM1000Device::setTransmitHandler(void (* handleSent)(void)) {
    DWM1000::attachSentHandler(handleSent);
}

void DWM1000Device::setReceiveHandler(void (* handleReceived)(void)) {
    DWM1000::attachReceivedHandler(handleReceived);
}

DWM1000Time DWM1000Device::setTransmitDelay(uint16_t delayUs) {
    return DWM1000::setDelay(delayUs);
}

void DWM1000Device::forceIdle() {
    DWM1000::forceTRxOff();
}

void DWM1000Device::receive() {
    DWM1000::newReceive();
    DWM1000::startReceive();
}


void DWM1000Device::transmit(byte data[], uint16_t dataLength) {
    DWM1000::setData(data, dataLength);
    DWM1000::startTransmit();
}

void DWM1000Device::transmitDelayed(byte data[], uint16_t dataLength) {
    DWM1000::setData(data, dataLength);
    DWM1000::startTransmit(TransmitMode::DELAYED);
}

uint16_t DWM1000Device::getShortAddress() {
    return _shortAddress;
}

uint16_t DWM1000Device::getPanId() {
    return _panId;
}

uint16_t DWM1000Device::getAntennaDelay() {
    return _antennaDelay;
}




