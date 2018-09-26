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

#include <Arduino.h>
#include "DW1000NgDeviceConfiguration.hpp"
#include "DW1000NgConstants.hpp"


DW1000NgDeviceConfiguration::DW1000NgDeviceConfiguration() {
    setProfile(&DW1000NgDeviceConfigurationProfiles::DEFAULT_CONF);
}

DW1000NgDeviceConfiguration::DW1000NgDeviceConfiguration(boolean nlos) {
    setProfile(nlos ? &DW1000NgDeviceConfigurationProfiles::NLOS_CONF : &DW1000NgDeviceConfigurationProfiles::DEFAULT_CONF);
}

DW1000NgDeviceConfiguration::DW1000NgDeviceConfiguration(device_configuration_t* conf) {
    setProfile(conf);
}

void DW1000NgDeviceConfiguration::setProfile(device_configuration_t* profile) {
    _extendedFrameLength = profile->extendedFrameLength;
    _receiverAutoReenable = profile->receiverAutoReenable;
    _smartPower = profile->smartPower;
    _frameCheck = profile->frameCheck;
    _nlos = profile->nlos;
    _sfd = profile->sfd;
    _channel = profile->channel;
    _dataRate = profile->dataRate;
    _pulseFreq = profile->pulseFreq;
    _preambleLen = profile->preambleLen;
    _preaCode = profile->preaCode;
}