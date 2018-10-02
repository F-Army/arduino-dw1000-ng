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

/*
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>
 * Decawave DW1000Ng library for arduino.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file DW1000Ng.h
 * Arduino driver library (header file) for the Decawave DW1000Ng UWB transceiver Module.
 */

#pragma once

#include <stdlib.h>
#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#include "DW1000NgConstants.hpp"
#include "DW1000NgConfiguration.hpp"
#include "DW1000NgCompileOptions.hpp"

namespace DW1000Ng {
	/** 
	Initiates and starts a sessions with a DW1000. If rst is not set or value 0xff, a soft resets (i.e. command
	triggered) are used and it is assumed that no reset line is wired.
	
	@param[in] ss  The SPI Selection pin used to identify the specific connection
	@param[in] irq The interrupt line/pin that connects the Arduino.
	@param[in] rst The reset line/pin for hard resets of ICs that connect to the Arduino. Value 0xff means soft reset.
	*/
	void initialize(uint8_t ss, uint8_t irq, uint8_t rst = 0xff);
	
	/** 
	(Re-)selects a specific DW1000 chip for communication. Used in case you switched SPI to another device.
	*/
	void select();

	/** 
	Tells the driver library that no communication to a DW1000 will be required anymore.
	This basically just frees SPI and the previously used pins.
	*/
	void end();
	
	/** 
	Enable debounce Clock, used to clock the LED blinking
	*/
	void enableDebounceClock();

	/**
	Enable led blinking feature
	*/
	void enableLedBlinking();

	/**
	Set DW1000's GPIO pins mode
	*/
	void setGPIOMode(uint8_t msgp, uint8_t mode);

	/**
	Enable deep sleep mode
	*/
	void deepSleep();

	/**
	Wake-up from deep sleep by toggle chip select pin
	*/
	void spiWakeup();
	
	/**
	Resets all connected or the currently selected DW1000 chip.
	Uses hardware reset or in case the reset pin is not wired it falls back to software Reset. 
	*/
	void reset();
	
	/** 
	Resets the currently selected DW1000 chip programmatically (via corresponding commands).
	*/
	void softwareReset();
	
	/** 
	(Re-)set the network identifier which the selected chip should be associated with. This
	setting is important for certain MAC address filtering rules.
	This is also referred as PanId

	@param[in] val An arbitrary numeric network identifier.
	*/
	void setNetworkId(uint16_t val);
	
	/** 
	(Re-)set the device address (i.e. short address) for the currently selected chip. This
	setting is important for certain MAC address filtering rules.

	@param[in] val An arbitrary numeric device address.
	*/
	void setDeviceAddress(uint16_t val);
	// TODO MAC and filters
	
	/**
	Sets the device Extended Unique Identifier.
	This is a long identifier of the device.

	@param[in] eui A string containing the eui in its normal notation using columns.
	*/
	void setEUI(char eui[]);

	/**
	Sets the device Extended Unique Identifier.
	This is a long identifier of the device.

	@param[in] eui The raw bytes of the eui.
	*/
	void setEUI(byte eui[]);

	/**
	Sets the transmission power of the device.
	Be careful to respect your current country limitations.

	@param[in] power Bytes that represent the power
	*/
	void setTXPower(byte power[]);

	/**
	Sets the transmission power of the device.
	Be careful to respect your current country limitations.

	@param[in] power  Bytes (written as a 32-bit number) that represent the power
	*/
	void setTXPower(int32_t power);

	/**
	Sets the transmission power of the device.
	Be careful to respect your current country limitations.

	@param[in] driver_amplifier Base power amplifier
	@param[in] mixer Mixer power
	*/
	void setTXPower(DriverAmplifierValue driver_amplifier, TransmitMixerValue mixer);

	/**
	Automatically sets power in respect to the current device settings.
	This should be guaranteed to set power under -41.3 dBm / MHz (legal limit in most countries).
	*/
	void setTXPowerAuto();

	/**
	Sets the pulse generator delay value.
	You should use the setTCPGDelayAuto() function.
	*/
	void setTCPGDelay(byte tcpg_delay);

	/**
	Automatically sets pulse generator delay value
	*/
	void setTCPGDelayAuto();

	/* Used for Transmit Power regulatory testing */
	void enableTransmitPowerSpectrumTestMode(int32_t repeat_interval);
	
	/* transmit and receive configuration. */
	void         setDelayedTRX(byte futureTimeBytes[]);
	void         setTransmitData(byte data[], uint16_t n);
	void         setTransmitData(const String& data);
	void         getReceivedData(byte data[], uint16_t n);
	void         getReceivedData(String& data);
	uint16_t     getReceivedDataLength();
	
	uint64_t     getTransmitTimestamp();
	uint64_t     getReceiveTimestamp();
	uint64_t     getSystemTimestamp();
	
	/* receive quality information. (RX_FSQUAL) - reg:0x12 */
	float getReceivePower();
	float getFirstPathPower();
	float getReceiveQuality();

	/* Antenna delay calibration */
	void setAntennaDelay(uint16_t value);
	void setTxAntennaDelay(uint16_t value);
	void setRxAntennaDelay(uint16_t value);

	uint16_t getTxAntennaDelay();
	uint16_t getRxAntennaDelay();

	/* callback handler management. */
	void attachErrorHandler(void (* handleError)(void));
	
	void attachSentHandler(void (* handleSent)(void));
	
	void attachReceivedHandler(void (* handleReceived)(void));
	
	void attachReceiveFailedHandler(void (* handleReceiveFailed)(void));
	
	void attachReceiveTimeoutHandler(void (* handleReceiveTimeout)(void));
	
	void attachReceiveTimestampAvailableHandler(void (* handleReceiveTimestampAvailable)(void));

	void pollForEvents();
	
	/* device state management. */
	// force idle state
	void forceTRxOff();

	void setInterruptPolarity(boolean val);

	void applyConfiguration(device_configuration_t config);
	void applyInterruptConfiguration(interrupt_configuration_t interrupt_config);

	/* Configuration Getters */

	Channel getChannel();

	PulseFrequency getPulseFrequency();
	
	// reception state
	void startReceive(ReceiveMode mode = ReceiveMode::IMMEDIATE);
	
	// transmission state
	void startTransmit(TransmitMode mode = TransmitMode::IMMEDIATE);
	
	/* host-initiated reading of temperature and battery voltage */
	void getTemperature(float& temp);
	void getBatteryVoltage(float& vbat);
	void getTemperatureAndBatteryVoltage(float& temp, float& vbat);

	/* Allow MAC frame filtering */
	// TODO auto-acknowledge
	void setFrameFilter(boolean val);
	void setFrameFilterBehaveCoordinator(boolean val);
	void setFrameFilterAllowBeacon(boolean val);
	//data type is used in the FC_1 0x41
	void setFrameFilterAllowData(boolean val);
	void setFrameFilterAllowAcknowledgement(boolean val);
	void setFrameFilterAllowMAC(boolean val);
	//Reserved is used for the Blink message
	void setFrameFilterAllowReserved(boolean val);
	
	// note: not sure if going to be implemented for now
	void setDoubleBuffering(boolean val);
	// TODO is implemented, but needs testing
	void useExtendedFrameLength(boolean val);
	// TODO is implemented, but needs testing
	void waitForResponse(boolean val);

	#if DW1000NG_PRINTABLE

	/* ##### Print device id, address, etc. ###################################### */
	/** 
	Generates a String representation of the device identifier of the chip. That usually 
	are the letters "DECA" plus the	version and revision numbers of the chip.

	@param[out] msgBuffer The String buffer to be filled with printable device information.
		Provide 128 bytes, this should be sufficient.
	*/
	void getPrintableDeviceIdentifier(char msgBuffer[]);
	
	/** 
	Generates a String representation of the extended unique identifier (EUI) of the chip.

	@param[out] msgBuffer The String buffer to be filled with printable device information.
		Provide 128 bytes, this should be sufficient.
	*/
	void getPrintableExtendedUniqueIdentifier(char msgBuffer[]);
	
	/** 
	Generates a String representation of the short address and network identifier currently
	defined for the respective chip.

	@param[out] msgBuffer The String buffer to be filled with printable device information.
		Provide 128 bytes, this should be sufficient.
	*/
	void getPrintableNetworkIdAndShortAddress(char msgBuffer[]);
	
	/** 
	Generates a String representation of the main operational settings of the chip. This
	includes data rate, pulse repetition frequency, preamble and channel settings.

	@param[out] msgBuffer The String buffer to be filled with printable device information.
		Provide 128 bytes, this should be sufficient.
	*/
	void getPrintableDeviceMode(char msgBuffer[]);
	#endif

	#if DW1000NG_DEBUG
	void getPrettyBytes(byte data[], char msgBuffer[], uint16_t n);
    void getPrettyBytes(byte cmd, uint16_t offset, char msgBuffer[], uint16_t n);
	#endif
};