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
	void initialize(uint8_t ss, uint8_t irq, uint8_t rst = 0xff, SPIClass&spi = SPI);

	/** 
	Initiates and starts a sessions with a DW1000 without interrupt. If rst is not set or value 0xff, a soft resets (i.e. command
	triggered) are used and it is assumed that no reset line is wired.
	
	@param[in] ss  The SPI Selection pin used to identify the specific connection
	@param[in] rst The reset line/pin for hard resets of ICs that connect to the Arduino. Value 0xff means soft reset.
	*/
	void initializeNoInterrupt(uint8_t ss, uint8_t rst = 0xff);
	
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
	Applies the common sleep configuration and on-wake mode to the DW1000 for both DEEP_SLEEP and SLEEP modes.
	ONW_LLDO_BIT and ONW_LLDE_BIT are 1 to default.

	@param [in] config struct	The sleep/deepsleep configuration to apply to the DW1000
	*/
	void applySleepConfiguration(sleep_configuration_t sleep_config);

	/**
	Enter in DeepSleep. applySleepConfiguration must be called first.
	Either spi wakeup or pin wakeup must be enabled.
	-- In case of future implementation of Sleep mode, you must reset proper antenna delay with setTxAntennaDelay() after wakeUp event. --
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
	Gets the network identifier (a.k.a PAN id) set for the device

	@param[out] id the bytes that represent the PAN id (2 bytes)
	*/
	void getNetworkId(byte id[]);
	
	/** 
	(Re-)set the device address (i.e. short address) for the currently selected chip. This
	setting is important for certain MAC address filtering rules.

	@param[in] val An arbitrary numeric device address.
	*/
	void setDeviceAddress(uint16_t val);

	/**
	Gets the short address identifier set for the device

	@param[out] address the bytes that represent the short address of the device(2 bytes)
	*/
	void getDeviceAddress(byte address[]);
	
	/**
	Sets the device Extended Unique Identifier.
	This is a long identifier of the device.

	@param[in] eui A string containing the eui in its normal notation using columns.
	*/
	void setEUI(const char eui[]);

	/**
	Sets the device Extended Unique Identifier.
	This is a long identifier of the device.

	@param[in] eui The raw bytes of the eui.
	*/
	void setEUI(byte eui[]);
	
	/**
	Gets the device Extended Unique Identifier.

	@param[out] eui The 8 bytes of the EUI.
	*/
	void getEUI(byte eui[]);

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

	/** 
	Enables transmit power spectrum test mode that is used for Transmit Power regulatory testing 
	
	@param [in] repeat_interval the interval to repeat the transmission
	*/
	void enableTransmitPowerSpectrumTestMode(int32_t repeat_interval);
	
	/**
	Sets a delay for transmission and receive

	@param [in] futureTimeBytes the timestamp in bytes of the time of the transmission (in UWB time)
	*/
	void setDelayedTRX(byte futureTimeBytes[]);

	/**
	Sets the transmission bytes inside the tx buffer of the DW1000

	@param [in] data the bytes to transmit
	@param [in] n the length of the array of bytes
	*/
	void setTransmitData(byte data[], uint16_t n);

	/**
	Sets the transmission bytes inside the tx buffer of the DW1000 based on the input string

	@param [in] data the string to transmit
	*/
	void setTransmitData(const String& data);

	/**
	Gets the received bytes and stores them in a byte array

	@param [out] data The array of byte to store the data
	@param [out] n The length of the byte array
	*/
	void getReceivedData(byte data[], uint16_t n);

	/**
	Stores the received data inside a string

	param [out] data the string that will contain the data
	*/
	void getReceivedData(String& data);

	/**
	Calculates the length of the received data

	returns the length of the data
	*/
	uint16_t getReceivedDataLength();
	
	/**
	Calculates the latest transmission timestamp

	return the last transmission timestamp
	*/
	uint64_t getTransmitTimestamp();

	/**
	Calculates the latest receive timestamp

	return the last receive timestamp
	*/
	uint64_t getReceiveTimestamp();

	/**
	Calculates the current system timestamp inside the DW1000

	return the system timestamp
	*/
	uint64_t getSystemTimestamp();
	
	/* receive quality information. (RX_FSQUAL) - reg:0x12 */

	/**
	Gets the receive power of the device (last receive)

	returns the last receive power of the device
	*/
	float getReceivePower();

	/**
	Gets the power of the first path

	returns the first path power
	*/ 
	float getFirstPathPower();

	/**
	Gets the last receive quality

	returns last receive quality
	*/
	float getReceiveQuality();

	/**
	Sets both tx and rx antenna delay value

	@param [in] value the delay in UWB time
	*/
	void setAntennaDelay(uint16_t value);

	#if defined(__AVR__)
		/**
		Sets both tx and rx antenna delay value, and saves it in the EEPROM for future use

		@param [in] value the delay in UWB time
		@param [in] the EEPROM offset at which the delay is saved
		*/
		void setAndSaveAntennaDelay(uint16_t delay, uint8_t eeAddress = 0);

		/**
		Gets the saved antenna delay value from EEPROM

		returns the value of the delay saved in the EEPROM in UWB time

		@param [in] the EEPROM offset at which the delay is saved
		*/
		uint16_t getSavedAntennaDelay(uint8_t eeAddress = 0);

		/**
		Sets the saved antenna delay value from EEPROM as the configured delay

		@param [in] the EEPROM offset at which the delay is saved
		*/
		uint16_t setAntennaDelayFromEEPROM(uint8_t eeAddress = 0);
	#endif
	
	/**
	Sets the tx antenna delay value

	@param [in] value the delay in UWB time
	*/
	void setTxAntennaDelay(uint16_t value);

	/**
	Sets the rx antenna delay value

	@param [in] value the delay in UWB time
	*/
	void setRxAntennaDelay(uint16_t value);

	/**
	Gets the tx antenna delay value

	returns the value of the delay in UWB time
	*/
	uint16_t getTxAntennaDelay();

	/**
	Gets the rx antenna delay value

	returns the value of the delay in UWB time
	*/
	uint16_t getRxAntennaDelay();

	/**
	Sets the function for error event handling

	@param [in] handleError the target function
	*/
	void attachErrorHandler(void (* handleError)(void));
	
	/**
	Sets the function for end of transission event handling

	@param [in] handleSent the target function
	*/
	void attachSentHandler(void (* handleSent)(void));
	
	/**
	Sets the function for end of receive event handling

	@param [in] handleReceived the target function
	*/
	void attachReceivedHandler(void (* handleReceived)(void));
	
	/**
	Sets the function for receive error event handling

	@param [in] handleReceiveFailed the target function
	*/
	void attachReceiveFailedHandler(void (* handleReceiveFailed)(void));
	
	/**
	Sets the function for receive timeout event handling

	@param [in] handleReceiveTimeout the target function
	*/
	void attachReceiveTimeoutHandler(void (* handleReceiveTimeout)(void));
	
	/**
	Sets the function for receive timestamp availabe event handling

	@param [in] handleReceiveTimestampAvailable the target function
	*/
	void attachReceiveTimestampAvailableHandler(void (* handleReceiveTimestampAvailable)(void));
	
	/**
	Handles dw1000 events triggered by interrupt
	By default this is attached to the interrupt pin callback
	*/
	void interruptServiceRoutine();
	
	boolean isTransmitDone();

	void clearTransmitStatus();

	boolean isReceiveDone();

	void clearReceiveStatus();

	boolean isReceiveFailed();

	void clearReceiveFailedStatus();

	boolean isReceiveTimeout();

	void clearReceiveTimeoutStatus();

	/**
	Stops the transceiver immediately, this actually sets the device in Idle mode.
	*/
	void forceTRxOff();

	/**
	Sets the interrupt polarity

	By default this is set to true by the DW1000

	@param [in] val True here means active high
	*/
	void setInterruptPolarity(boolean val);

	/**
	Applies the target configuration to the DW1000

	@param [in] config the configuration to apply to the DW1000
	*/
	void applyConfiguration(device_configuration_t config);

	/**
	Enables the interrupts for the target events

	@param [in] interrupt_config the interrupt map to use
	*/
	void applyInterruptConfiguration(interrupt_configuration_t interrupt_config);

	/**
	Gets the current channel in use

	returns the current channel
	*/
	Channel getChannel();

	/**
	Gets the current PRF of the device

	returns the current PRF
	*/
	PulseFrequency getPulseFrequency();
	
	/**
	Sets the timeout for Raceive Frame.

	@param[in] Pac size based on current preamble lenght - 1
	*/
	void setPreambleDetectionTimeout(uint16_t pacSize);

	/**
	Sets the timeout for SFD detection.
	The recommended value is: PreambleLenght + SFD + 1.
	The default value is 4096+64+1
	
	@param[in] the sfd detection timeout 
	*/
	void setSfdDetectionTimeout(uint16_t preambleSymbols);

	/**
	Sets the timeout for Raceive Frame. Must be sets in idle mode.
	Allow the external microprocessor to enter a low power state awaiting a valid receive frame.

	@param[in] time in μs. units = ~1μs(1.026μs). 0 to disable 
	*/
	void setReceiveFrameWaitTimeoutPeriod(uint16_t timeMicroSeconds);

	/**
	Sets the device in receive mode

	@param [in] mode IMMEDIATE or DELAYED receive
	*/
	void startReceive(ReceiveMode mode = ReceiveMode::IMMEDIATE);
	
	/**
	Sets the device in transmission mode

	@param [in] mode IMMEDIATE or DELAYED transmission
	*/
	void startTransmit(TransmitMode mode = TransmitMode::IMMEDIATE);
		
	/**
	Gets the temperature inside the DW1000 Device

	returns The temperature 
	*/
	float getTemperature();

	/**
	Gets the voltage in input of the DW1000

	returns The input voltage
	*/
	float getBatteryVoltage();

	/**
	Gets both temperature and voltage with a single read

	@param [out] temp the temperature
	@param [out] vbat the input voltage
	*/ 
	void getTemperatureAndBatteryVoltage(float& temp, float& vbat);

	/**
	Enables the frame filtering functionality using the provided configuration.
	Messages must be formatted using 802.15.4-2011 format.

	@param [in] config frame filtering configuration
	*/
	void enableFrameFiltering(frame_filtering_configuration_t config);

	/**
	Disables the frame filtering functionality
	*/
	void disableFrameFiltering();
	
	/**
	WARNING: this just sets the relative bits inside the register.
	You must refer to the DW1000 User manual to activate it properly.
	*/
	void setDoubleBuffering(boolean val);

	/**
	Enables frames up to 1023 byte length

	@param [in] val true or false
	*/
	void useExtendedFrameLength(boolean val);
	
	/**
	Sets the time before the device enters receive after a transmission.
	Use 0 here to deactivate it.

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
