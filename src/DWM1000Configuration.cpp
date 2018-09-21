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

#include "DWM1000Configuration.hpp"
#include "DWM1000Constants.hpp"

DWM1000Configuration::DWM1000Configuration() {
    setDefaultProfile();
}

DWM1000Configuration::DWM1000Configuration(
        byte dataRate,
        byte pulseFrequency,
        byte preambleLength,
        byte channel,
        byte preambleCode,
        boolean interruptOnSent,
	    boolean interruptOnReceived,
	    boolean interruptOnReceiveFailed,
        boolean interruptOnReceiveTimeout,
	    boolean interruptOnReceiveTimestampAvailable,
	    boolean interruptOnAutomaticAcknowledgeTrigger,
        boolean frameCheck,
        boolean extendedFrameLength,
        boolean receiverAutoReenable,
        boolean nlos,
        boolean smartPower,
        uint16_t antennaDelay = 16384
) {
    setDatarate(dataRate);
    setPulseFrequency(pulseFrequency);
    setPreambleLength(preambleLength);
    setChannel(channel);
    setPreambleCode(preambleCode);
    setInterruptOnSent(interruptOnSent);
	setInterruptOnReceived(interruptOnReceived);
	setInterruptOnReceiveFailed(interruptOnReceiveFailed);
    setInterruptOnReceiveTimeout(interruptOnReceiveTimeout);
	setInterruptOnReceiveTimestampAvailable(interruptOnReceiveTimestampAvailable);
	setInterruptOnAutomaticAcknowledgeTrigger(interruptOnAutomaticAcknowledgeTrigger);
    setFrameCheck(frameCheck);
    setExtendedFrameLength(extendedFrameLength);
    setReceiverAutoReenable(receiverAutoReenable);
    setNlos(nlos);
    setSmartPower(smartPower);
    setAntennaDelay(antennaDelay);
}

DWM1000Configuration::DWM1000Configuration(
        byte dataRate,
        byte pulseFrequency,
        byte preambleLength,
        byte channel,
        byte preambleCode,
        boolean interruptOnSent,
	    boolean interruptOnReceived,
	    boolean interruptOnReceiveFailed,
        boolean interruptOnReceiveTimeout,
	    boolean interruptOnReceiveTimestampAvailable,
	    boolean interruptOnAutomaticAcknowledgeTrigger,
        boolean frameCheck,
        boolean extendedFrameLength,
        boolean receiverAutoReenable,
        boolean nlos,
        int32_t txPower,
        byte tcpgDelay,
        uint16_t antennaDelay = 16384
) {
    setDatarate(dataRate);
    setPulseFrequency(pulseFrequency);
    setPreambleLength(preambleLength);
    setChannel(channel);
    setPreambleCode(preambleCode);
    setInterruptOnSent(interruptOnSent);
	setInterruptOnReceived(interruptOnReceived);
	setInterruptOnReceiveFailed(interruptOnReceiveFailed);
    setInterruptOnReceiveTimeout(interruptOnReceiveTimeout);
	setInterruptOnReceiveTimestampAvailable(interruptOnReceiveTimestampAvailable);
	setInterruptOnAutomaticAcknowledgeTrigger(interruptOnAutomaticAcknowledgeTrigger);
    setFrameCheck(frameCheck);
    setExtendedFrameLength(extendedFrameLength);
    setReceiverAutoReenable(receiverAutoReenable);
    setNlos(nlos);
    setTxPower(txPower);
    setTcpgDelay(tcpgDelay);
    setAntennaDelay(antennaDelay);
}

DWM1000Configuration::~DWM1000Configuration() { }

byte        DWM1000Configuration::getDatarate() { return _dataRate; }
byte        DWM1000Configuration::getPulseFrequency() { return _pulseFrequency; }
byte        DWM1000Configuration::getPreambleLength() { return _preambleLength; }
byte        DWM1000Configuration::getChannel() { return _channel; }
byte        DWM1000Configuration::getPreambleCode() { return _preambleCode; }
boolean     DWM1000Configuration::getInterruptOnSent() { return _interruptOnSent; }
boolean     DWM1000Configuration::getInterruptOnReceived() { return _interruptOnReceived; }
boolean     DWM1000Configuration::getInterruptOnReceiveFailed() { return _interruptOnReceiveFailed; }
boolean     DWM1000Configuration::getInterruptOnReceiveTimeout() { return _interruptOnReceiveTimeout; }
boolean     DWM1000Configuration::getInterruptOnReceiveTimestampAvailable() { return _interruptOnReceiveTimestampAvailable; }
boolean     DWM1000Configuration::getInterruptOnAutomaticAcknowledgeTrigger() { return _interruptOnAutomaticAcknowledgeTrigger; }
boolean     DWM1000Configuration::getFrameCheck() { return _frameCheck; }
boolean     DWM1000Configuration::getExtendedFrameLength() { return _extendedFrameLength; }
boolean     DWM1000Configuration::getReceiverAutoReenable() { return _receiverAutoReenable; }
boolean     DWM1000Configuration::getNlos() { return _nlos; }
boolean     DWM1000Configuration::getSmartPower() { return _smartPower; }
int32_t     DWM1000Configuration::getTxPower() { return _txPower; }
byte        DWM1000Configuration::getTcpgDelay() { return _tcpgDelay; }
uint16_t    DWM1000Configuration::getAntennaDelay() { return _antennaDelay; } 
boolean     DWM1000Configuration::getAutoTxPower() { return _txPower; }
boolean     DWM1000Configuration::getAutoTcpgDelay() { return _tcpgDelay; }

void        DWM1000Configuration::setDatarate(byte dataRate) { _dataRate = dataRate; }
void        DWM1000Configuration::setPulseFrequency(byte pulseFrequency) { _pulseFrequency = pulseFrequency; }
void        DWM1000Configuration::setPreambleLength(byte preambleLength) { _preambleLength = preambleLength; }
void        DWM1000Configuration::setChannel(byte channel) { _channel = channel; }
void        DWM1000Configuration::setPreambleCode(byte preambleCode) { _preambleCode = preambleCode; }
void        DWM1000Configuration::setInterruptOnSent(boolean interruptOnSent) { _interruptOnSent = interruptOnSent; }
void        DWM1000Configuration::setInterruptOnReceived(boolean interruptOnReceived) { _interruptOnReceived = interruptOnReceived; }
void        DWM1000Configuration::setInterruptOnReceiveFailed(boolean interruptOnReceiveFailed) { _interruptOnReceiveFailed = interruptOnReceiveFailed; }
void        DWM1000Configuration::setInterruptOnReceiveTimeout(boolean interruptOnReceiveTimeout) { _interruptOnReceiveTimeout = interruptOnReceiveTimeout; }
void        DWM1000Configuration::setInterruptOnReceiveTimestampAvailable(boolean interruptOnReceiveTimestampAvailable) { _interruptOnReceiveTimestampAvailable = interruptOnReceiveTimestampAvailable; }
void        DWM1000Configuration::setInterruptOnAutomaticAcknowledgeTrigger(boolean interruptOnAutomaticAcknowledgeTrigger) { _interruptOnAutomaticAcknowledgeTrigger = interruptOnAutomaticAcknowledgeTrigger; }
void        DWM1000Configuration::setFrameCheck(boolean frameCheck) { _frameCheck = frameCheck; }
void        DWM1000Configuration::setExtendedFrameLength(boolean extendedFrameLength) { _extendedFrameLength = extendedFrameLength; }
void        DWM1000Configuration::setReceiverAutoReenable(boolean receiverAutoReenable) { _receiverAutoReenable = receiverAutoReenable; }
void        DWM1000Configuration::setNlos(boolean nlos) { _nlos = nlos; }
void        DWM1000Configuration::setSmartPower(boolean smartPower) { _smartPower = smartPower; }
void        DWM1000Configuration::setTxPower(int32_t txPower) { _txPower = txPower; }
void        DWM1000Configuration::setTcpgDelay(byte tcpgDelay) { _tcpgDelay = tcpgDelay; }
void        DWM1000Configuration::setAntennaDelay(uint16_t antennaDelay) { _antennaDelay = antennaDelay; }

void        DWM1000Configuration::setDefaultProfile() {
    setDatarate(TRX_RATE_6800KBPS);
    setPulseFrequency(TX_PULSE_FREQ_16MHZ);
    setPreambleLength(TX_PREAMBLE_LEN_128);
    setChannel(CHANNEL_5);
    setPreambleCode(PREAMBLE_CODE_16MHZ_4);
    setInterruptOnSent(true);
	setInterruptOnReceived(true);
	setInterruptOnReceiveFailed(true);
    setInterruptOnReceiveTimeout(true);
	setInterruptOnReceiveTimestampAvailable(false);
	setInterruptOnAutomaticAcknowledgeTrigger(true);
    setFrameCheck(true);
    setExtendedFrameLength(false);
    setReceiverAutoReenable(true);
    setNlos(false);
    setSmartPower(true);
    setAntennaDelay(16384);
}






