#include <Arduino.h>
#include "DW1000NgConfiguration.hpp"
#include "DW1000NgCompileOptions.hpp"

namespace DW1000NgConfiguration {
    device_configuration_t defaultConfig() {
        device_configuration_t conf = {
            false,
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

        return conf;
    }

    device_configuration_t defaultNlosConfig() {
        device_configuration_t conf = {
            false,
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

        return conf;
    }

    #if DW1000NGCONFIGURATION_H_PRINTABLE
    void printConfig(device_configuration_t config) {
        String printString = "\nFrame Filtering: "; printString += config.frameFiltering;
        printString += "\nExt frame length: "; printString += config.extendedFrameLength;
        printString += "\nReceiver auto reenable: "; printString += config.receiverAutoReenable;
        printString += "\nSmart power: "; printString += config.smartPower;
        printString += "\nFramecheck: "; printString += config.frameCheck;
        printString += "\nNlos: "; printString += config.nlos;
        printString += "\nSFDMode: "; printString += static_cast<byte>(config.sfd);
        printString += "\nChannel: "; printString += static_cast<byte>(config.channel);
        printString += "\nDataRate: "; printString += static_cast<byte>(config.dataRate);
        printString += "\nPulseFrequency: "; printString += static_cast<byte>(config.pulseFreq);
        printString += "\nPreamble Length: "; printString += static_cast<byte>(config.preambleLen);
        printString += "\nPreamble Code: "; printString += static_cast<byte>(config.preaCode);
        Serial.println(printString);
    }
    #endif
}