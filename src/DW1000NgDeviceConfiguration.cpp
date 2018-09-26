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

namespace DW1000NgDeviceConfigurationProfiles {
    
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

DW1000NgDeviceConfiguration::DW1000NgDeviceConfiguration(boolean nlos) {
    if(nlos) {
        setProfile(DW1000NgDeviceConfigurationProfiles::NLOS_CONF);
    } else {
        setProfile(DW1000NgDeviceConfigurationProfiles::DEFAULT_CONFIGURATION);
    }
}

DW1000NgDeviceConfiguration::DW1000NgDeviceConfiguration(device_configuration_t conf) {
    setProfile(conf);
}

#if DW1000NGDEVICECONFIGURATION_H_PRINTABLE
    size_t DW1000NgDeviceConfiguration::printTo(Print& p) const {
        String message = "";
        message += "Extended Frame Length:"; message += _extendedFrameLength;
        message += "\nReceiver Auto reenable:"; message += _receiverAutoReenable;
        message += "\nSmart Power:"; message += _smartPower;
        message += "\nFrame check:"; message += _frameCheck;
        message += "\nNlos:"; message += _nlos;
        message += "\nSfd:"; 
        if(_sfd == SFDMode::STANDARD_SFD) {
            message += "Standard";
        } else {
            message += "Decawave";
        }
        message += "\nChannel:"; message += static_cast<byte>(_channel);  
        message += "\nDataRate:";
        if(_dataRate == DataRate::RATE_110KBPS) {
            message += "110Kbps";
        } else if(_dataRate == DataRate::RATE_850KBPS) {
            message += "850Kbps";
        } else {
            message += "6800Kbps";
        }
        message += "\nPRF:";
        if(_pulseFreq == PulseFrequency::FREQ_16MHZ) {
            message += "16Mhz";
        } else {
            message += "64Mhz";
        }
        message += "\nPreamble Length:";
        switch(_preambleLen) {
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
        message += "\nPreamble Code:"; message += static_cast<byte>(_preaCode);

        p.print(message);
        return message.length();      
    }
#endif

void DW1000NgDeviceConfiguration::setProfile(device_configuration_t profile) {
    _extendedFrameLength = profile.extendedFrameLength;
    _receiverAutoReenable = profile.receiverAutoReenable;
    _smartPower = profile.smartPower;
    _frameCheck = profile.frameCheck;
    _nlos = profile.nlos;
    _sfd = profile.sfd;
    _channel = profile.channel;
    _dataRate = profile.dataRate;
    _pulseFreq = profile.pulseFreq;
    _preambleLen = profile.preambleLen;
    _preaCode = profile.preaCode;
}