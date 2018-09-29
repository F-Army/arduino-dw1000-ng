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
#include "DW1000NgConfiguration.hpp"
#include "DW1000NgConstants.hpp"

namespace {
    
    device_configuration_t DEFAULT_CONFIGURATION = {
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

    device_configuration_t NLOS_CONF = {
            false,
            true,
            true,
            true,
            true,
            SFDMode::STANDARD_SFD,
            Channel::CHANNEL_5,
            DataRate::RATE_850KBPS,
            PulseFrequency::FREQ_16MHZ,
            PreambleLength::LEN_256,
            PreambleCode::CODE_3
    };
}

DW1000NgConfiguration::DW1000NgConfiguration(ConfigurationProfile profile) {
    this->setConfiguration(profile);
}

DW1000NgConfiguration::DW1000NgConfiguration(device_configuration_t conf) {
    this->setConfiguration(conf);
}

#if DW1000NGCONFIGURATION_H_PRINTABLE
    size_t DW1000NgConfiguration::printTo(Print& p) const {
        String message = "";
        message += "Extended Frame Length:"; message += this->_config.extendedFrameLength;
        message += "\nReceiver Auto reenable:"; message += this->_config.receiverAutoReenable;
        message += "\nSmart Power:"; message += this->_config.smartPower;
        message += "\nFrame check:"; message += this->_config.frameCheck;
        message += "\nNlos:"; message += this->_config.nlos;
        message += "\nSfd:"; 
        if(this->_config.sfd == SFDMode::STANDARD_SFD) {
            message += "Standard";
        } else {
            message += "Decawave";
        }
        message += "\nChannel:"; message += static_cast<byte>(this->_config.channel);  
        message += "\nDataRate:";
        if(this->_config.dataRate == DataRate::RATE_110KBPS) {
            message += "110Kbps";
        } else if(this->_config.dataRate == DataRate::RATE_850KBPS) {
            message += "850Kbps";
        } else {
            message += "6800Kbps";
        }
        message += "\nPRF:";
        if(this->_config.pulseFreq == PulseFrequency::FREQ_16MHZ) {
            message += "16Mhz";
        } else {
            message += "64Mhz";
        }
        message += "\nPreamble Length:";
        switch(this->_config.preambleLen) {
            case PreambleLength::LEN_64:
                message += "64";
                break;
            case PreambleLength::LEN_128:
                message += "128";
                break;
            case PreambleLength::LEN_256:
                message += "256";
                break;
            case PreambleLength::LEN_512:
                message += "512";
                break;
            case PreambleLength::LEN_1024:
                message += "1024";
                break;
            case PreambleLength::LEN_1536:
                message += "1536";
                break;
            case PreambleLength::LEN_2048:
                message += "2048";
                break;
            case PreambleLength::LEN_4096:
                message += "4096";
                break;
        }
        message += "\nPreamble Code:"; message += static_cast<byte>(this->_config.preaCode);

        p.print(message);
        return message.length();      
    }
#endif

void DW1000NgConfiguration::setConfiguration(device_configuration_t config) {
    this->_config.extendedFrameLength = config.extendedFrameLength;
    this->_config.receiverAutoReenable = config.receiverAutoReenable;
    this->_config.smartPower = config.smartPower;
    this->_config.frameCheck = config.frameCheck;
    this->_config.nlos = config.nlos;
    this->_config.sfd = config.sfd;
    this->_config.channel = config.channel;
    this->_config.dataRate = config.dataRate;
    this->_config.pulseFreq = config.pulseFreq;
    this->_config.preambleLen = config.preambleLen;
    this->_config.preaCode = config.preaCode;
}

void DW1000NgConfiguration::setConfiguration(ConfigurationProfile profile) {
    if(profile == ConfigurationProfile::DEFAULT_PROFILE) {
        this->setConfiguration(DEFAULT_CONFIGURATION);
    } else if(profile == ConfigurationProfile::DEFAULT_NLOS_PROFILE) {
        this->setConfiguration(NLOS_CONF);
    } else {
        //TODO error handler
    }
}

device_configuration_t DW1000NgConfiguration::getConfiguration() {
    return this->_config;
}