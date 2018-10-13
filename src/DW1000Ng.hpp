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
	/* ##### Init ################################################################ */
	/** 
	Initiates and starts a sessions with one or more DW1000. If rst is not set or value 0xff, a soft resets (i.e. command
	triggered) are used and it is assumed that no reset line is wired.
	 
	@param[in] irq The interrupt line/pin that connects the Arduino.
	@param[in] rst The reset line/pin for hard resets of ICs that connect to the Arduino. Value 0xff means soft reset.
	*/
	void initialize(uint8_t ss, uint8_t irq, uint8_t rst = 0xff);
	
	/** 
	(Re-)selects a specific DW1000 chip for communication. In case of a single DW1000Ng chip in use
	this call is not needed; only a call to `select()` has to be performed once at start up. Other 
	than a call to `select()` this function does not perform an initial setup of the (again-)selected 
	chips and assumes it to have a valid configuration loaded.

	@param[in] ss The chip select line/pin that connects the to-be-selected chip with the
	Arduino.
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
	Set GPIO mode
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
	Resets all connected or the currently selected DW1000 chip. A hard reset of all chips
	is preferred, although a soft reset of the currently selected one is executed if no 
	reset pin has been specified (when using `begin(int)`, instead of `begin(int, int)`).
	*/
	void reset();
	
	/** 
	Resets the currently selected DW1000 chip programmatically (via corresponding commands).
	*/
	void softwareReset();
	
	/* ##### Device address management, filters ################################## */
	/** 
	(Re-)set the network identifier which the selected chip should be associated with. This
	setting is important for certain MAC address filtering rules.

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
	
	void setEUI(char eui[]);
	void setEUI(byte eui[]);

	void setTXPower(byte power[]);
	void setTXPower(int32_t power);
	void setTXPower(DriverAmplifierValue driver_amplifier, TransmitMixerValue mixer);
	void setTXPowerAuto();

	void setTCPGDelay(byte tcpg_delay);
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
	
	/**
	Sets the timeout for Raceive Frame.

	@param[in] Pac size based on current preamble lenght - 1
	*/
	void setPreambleDetectionTimeout(uint16_t pacSize);

	/**
	Sets the timeout for SFD detection.
	
	@param[in] PreambleLenght + SFD + 1. default value 4096+64+1 
	*/
	void setSfdDetectionTimeout(uint16_t preambleSymbols);

	/**
	Sets the timeout for Raceive Frame.
	Allow the external microprocessor to enter a low power state awaiting a valid receive frame.

	@param[in] time in μs. units = ~1μs(1.026μs) 
	*/
	void useReceiveFrameWaitTimeoutPeriod(uint16_t timeMicroSeconds);

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
	void enableFrameFiltering(frame_filtering_configuration_t config);
	void disableFrameFiltering();
	
	// note: not sure if going to be implemented for now
	void setDoubleBuffering(boolean val);
	// TODO is implemented, but needs testing
	void useExtendedFrameLength(boolean val);
	
	/**
	Sets the time before the device enters receive after a transmission.

	@param[in] time in μs. units = ~1μs(1.026μs) 
	*/
	void setWait4Response(uint32_t timeMicroSeconds);

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