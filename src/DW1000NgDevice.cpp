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

#include "DW1000Ng.hpp"
#include "DW1000NgDevice.hpp"
#include "DW1000NgDeviceConfiguration.hpp"

device_interrupt_map_t default_interrupt_map {
    true,
    true,
    true,
    true,
    false,
    true
};

/* CONSTRUCTORS */

DW1000NgDevice::DW1000NgDevice(DW1000NgDeviceConfiguration config, device_interrupt_map_t int_map, uint8_t ss,  uint8_t irq, uint8_t rst) {
    _config.setConfiguration(config.getConfiguration());
    _interrupt_map = int_map;

    DW1000Ng::begin(ss, irq, rst);
    DW1000Ng::newConfiguration();
    DW1000Ng::setFrameFilter(false);
    DW1000Ng::interruptOnSent(_interrupt_map.interruptOnSent);
    DW1000Ng::interruptOnReceived(_interrupt_map.interruptOnReceived);
    DW1000Ng::interruptOnReceiveFailed(_interrupt_map.interruptOnReceiveFailed);
    DW1000Ng::interruptOnReceiveTimestampAvailable(_interrupt_map.interruptOnReceiveTimestampAvailable);
    DW1000Ng::interruptOnAutomaticAcknowledgeTrigger(_interrupt_map.interruptOnAutomaticAcknowledgeTrigger);
    DW1000Ng::useExtendedFrameLength(_config.getConfiguration().extendedFrameLength);
    DW1000Ng::useSmartPower(_config.getConfiguration().smartPower);
    DW1000Ng::suppressFrameCheck(!(_config.getConfiguration().frameCheck));
    DW1000Ng::setSFDMode(_config.getConfiguration().sfd);
    DW1000Ng::setChannel(_config.getConfiguration().channel);
    DW1000Ng::setDataRate(_config.getConfiguration().dataRate);
    DW1000Ng::setPulseFrequency(_config.getConfiguration().pulseFreq);
    DW1000Ng::setPreambleLength(_config.getConfiguration().preambleLen);
    DW1000Ng::setPreambleCode(_config.getConfiguration().preaCode);
    DW1000Ng::setNLOS(_config.getConfiguration().nlos);
    DW1000Ng::setReceiverAutoReenable(_config.getConfiguration().receiverAutoReenable);
    DW1000Ng::commitConfiguration();
}
DW1000NgDevice::DW1000NgDevice(DW1000NgDeviceConfiguration config, device_interrupt_map_t int_map, uint8_t ss,  uint8_t irq) : DW1000NgDevice(config, int_map, ss,  irq, 0xff) { }

DW1000NgDevice::DW1000NgDevice(DW1000NgDeviceConfiguration config, uint8_t ss,  uint8_t irq, uint8_t rst) : DW1000NgDevice(config, default_interrupt_map, ss,  irq, rst) { }
DW1000NgDevice::DW1000NgDevice(DW1000NgDeviceConfiguration config, uint8_t ss,  uint8_t irq) : DW1000NgDevice(config, ss, irq, 0xff) { }

DW1000NgDevice::DW1000NgDevice(device_interrupt_map_t int_map, uint8_t ss,  uint8_t irq, uint8_t rst) : DW1000NgDevice(_config, int_map, ss, irq, rst) { }
DW1000NgDevice::DW1000NgDevice(device_interrupt_map_t int_map, uint8_t ss,  uint8_t irq) : DW1000NgDevice(int_map, ss, irq, 0xff) { }

DW1000NgDevice::DW1000NgDevice(uint8_t ss,  uint8_t irq, uint8_t rst) : DW1000NgDevice(default_interrupt_map, ss, irq, rst) { }
DW1000NgDevice::DW1000NgDevice(uint8_t ss,  uint8_t irq) : DW1000NgDevice(ss, irq, 0xff) { }

/* DESTRUCTORS */
DW1000NgDevice::~DW1000NgDevice() { }

/* METHODS */
void select() { DW1000Ng::select(); }
void end() { DW1000Ng::end(); }
