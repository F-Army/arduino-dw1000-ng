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
#include "DW1000NgConstants.hpp"
#include "DW1000NgCompileOptions.hpp"

typedef struct device_configuration_t {
    boolean extendedFrameLength;
    boolean receiverAutoReenable;
    boolean smartPower;
    boolean frameCheck;
    boolean nlos;
    SFDMode sfd;
    Channel channel;
    DataRate dataRate;
    PulseFrequency pulseFreq;
    PreambleLength preambleLen;
    PreambleCode preaCode;
} device_configuration_t;

enum class ConfigurationProfile {
    DEFAULT_PROFILE,
    DEFAULT_NLOS_PROFILE
};

#if DW1000NGCONFIGURATION_H_PRINTABLE
class DW1000NgConfiguration : public Printable {
#else
class DW1000NgConfiguration {
#endif

public:
    DW1000NgConfiguration(ConfigurationProfile profile = ConfigurationProfile::DEFAULT_PROFILE);
    DW1000NgConfiguration(device_configuration_t conf);

#if DW1000NGCONFIGURATION_H_PRINTABLE
    size_t printTo(Print& p) const;
#endif

    void setConfiguration(device_configuration_t config);
    void setConfiguration(ConfigurationProfile profile);

    device_configuration_t getConfiguration();

private:
    device_configuration_t _config;
};