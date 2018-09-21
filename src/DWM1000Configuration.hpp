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

class DWM1000Configuration {
public:
    DWM1000Configuration();

    DWM1000Configuration(
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
    );

    DWM1000Configuration(
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
    );

    ~DWM1000Configuration();

    byte        getDatarate();
    byte        getPulseFrequency();
    byte        getPreambleLength();
    byte        getChannel();
    byte        getPreambleCode();
    boolean     getInterruptOnSent();
	boolean     getInterruptOnReceived();
	boolean     getInterruptOnReceiveFailed();
    boolean     getInterruptOnReceiveTimeout();
	boolean     getInterruptOnReceiveTimestampAvailable();
	boolean     getInterruptOnAutomaticAcknowledgeTrigger();
    boolean     getFrameCheck();
    boolean     getExtendedFrameLength();
    boolean     getReceiverAutoReenable();
    boolean     getNlos();
    boolean     getSmartPower();
    int32_t     getTxPower();
    byte        getTcpgDelay();
    uint16_t    getAntennaDelay();
    boolean     getAutoTxPower();
    boolean     getAutoTcpgDelay();

    void        setDatarate(byte dataRate);
    void        setPulseFrequency(byte pulseFrequency);
    void        setPreambleLength(byte preambleLength);
    void        setChannel(byte channel);
    void        setPreambleCode(byte preambleCode);
    void        setInterruptOnSent(boolean interruptOnSent);
	void        setInterruptOnReceived(boolean interruptOnReceived);
	void        setInterruptOnReceiveFailed(boolean interruptOnReceiveFailed);
    void        setInterruptOnReceiveTimeout(boolean interruptOnReceiveTimeout);
	void        setInterruptOnReceiveTimestampAvailable(boolean interruptOnReceiveTimestampAvailable);
	void        setInterruptOnAutomaticAcknowledgeTrigger(boolean interruptOnAutomaticAcknowledgeTrigger);
    void        setFrameCheck(boolean frameCheck);
    void        setExtendedFrameLength(boolean extendedFrameLength);
    void        setReceiverAutoReenable(boolean receiverAutoReenable);
    void        setNlos(boolean nlos);
    void        setSmartPower(boolean smartPower);
    void        setTxPower(int32_t txPower);
    void        setTcpgDelay(byte tcpgDelay);
    void        setAntennaDelay(uint16_t antennaDelay);

    void        setDefaultProfile();

private:
    byte        _dataRate;
    byte        _pulseFrequency;
    byte        _preambleLength,;
    byte        _channel;
    byte        _preambleCode;
    boolean     _interruptOnSent;
	boolean     _interruptOnReceived;
	boolean     _interruptOnReceiveFailed;
    boolean     _interruptOnReceiveTimeout;
	boolean     _interruptOnReceiveTimestampAvailable;
	boolean     _interruptOnAutomaticAcknowledgeTrigger;
    boolean     _frameCheck;
    boolean     _extendedFrameLength;
    boolean     _receiverAutoReenable;
    boolean     _nlos;
    boolean     _smartPower;
    int32_t     _txPower;
    byte        _tcpgDelay;
    uint16_t    _antennaDelay;

    boolean     _autoTXPower;
    boolean     _autoTCPGDelay;
};
