/*
 * Copyright (c) 2018 by Michele Biondi <michelebiondi01@gmail.com>, Andrea Salvatori <andrea.salvatori92@gmail.com>
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>; Arduino-DW1000
 * Decawave DWM1000 library for arduino.
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
 * @file DWM1000.h
 * Arduino driver library (header file) for the Decawave DWM1000 UWB transceiver Module.
 */

#pragma once

#include <stdlib.h>
#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#include "DWM1000Constants.h"
#include "DWM1000Time.h"

class DWM1000Class {
public:
	/* ##### Init ################################################################ */
	/** 
	Initiates and starts a sessions with one or more DWM1000. If rst is not set or value 0xff, a soft resets (i.e. command
	triggered) are used and it is assumed that no reset line is wired.
	 
	@param[in] irq The interrupt line/pin that connects the Arduino.
	@param[in] rst The reset line/pin for hard resets of ICs that connect to the Arduino. Value 0xff means soft reset.
	*/
	static void begin(uint8_t irq, uint8_t rst = 0xff);
	
	/** 
	Selects a specific DWM1000 chip for communication. In case of a single DWM1000 chip in use
	this call only needs to be done once at start up, but is still mandatory. Other than a call
	to `reselect()` this function performs an initial setup of the now-selected chip.

	@param[in] ss The chip select line/pin that connects the to-be-selected chip with the
	Arduino.
	*/
	static void select(uint8_t ss);
	
	/** 
	(Re-)selects a specific DWM1000 chip for communication. In case of a single DWM1000 chip in use
	this call is not needed; only a call to `select()` has to be performed once at start up. Other 
	than a call to `select()` this function does not perform an initial setup of the (again-)selected 
	chips and assumes it to have a valid configuration loaded.

	@param[in] ss The chip select line/pin that connects the to-be-selected chip with the
	Arduino.
	*/
	static void reselect(uint8_t ss);
	
	/** 
	Tells the driver library that no communication to a DWM1000 will be required anymore.
	This basically just frees SPI and the previously used pins.
	*/
	static void end();
	
	/** 
	Enable debounce Clock, used to clock the LED blinking
	*/
	static void enableDebounceClock();

	/**
	Enable led blinking feature
	*/
	static void enableLedBlinking();

	/**
	Set GPIO mode
	*/
	static void setGPIOMode(uint8_t msgp, uint8_t mode);

	/**
	Enable deep sleep mode
	*/
	static void deepSleep();

	/**
	Wake-up from deep sleep by toggle chip select pin
	*/
	static void spiWakeup();

	/**
	Resets all connected or the currently selected DWM1000 chip. A hard reset of all chips
	is preferred, although a soft reset of the currently selected one is executed if no 
	reset pin has been specified (when using `begin(int)`, instead of `begin(int, int)`).
	*/
	static void reset();
	
	/** 
	Resets the currently selected DWM1000 chip programmatically (via corresponding commands).
	*/
	static void softReset();
	
	/* ##### Print device id, address, etc. ###################################### */
	/** 
	Generates a String representation of the device identifier of the chip. That usually 
	are the letters "DECA" plus the	version and revision numbers of the chip.

	@param[out] msgBuffer The String buffer to be filled with printable device information.
		Provide 128 bytes, this should be sufficient.
	*/
	static void getPrintableDeviceIdentifier(char msgBuffer[]);
	
	/** 
	Generates a String representation of the extended unique identifier (EUI) of the chip.

	@param[out] msgBuffer The String buffer to be filled with printable device information.
		Provide 128 bytes, this should be sufficient.
	*/
	static void getPrintableExtendedUniqueIdentifier(char msgBuffer[]);
	
	/** 
	Generates a String representation of the short address and network identifier currently
	defined for the respective chip.

	@param[out] msgBuffer The String buffer to be filled with printable device information.
		Provide 128 bytes, this should be sufficient.
	*/
	static void getPrintableNetworkIdAndShortAddress(char msgBuffer[]);
	
	/** 
	Generates a String representation of the main operational settings of the chip. This
	includes data rate, pulse repetition frequency, preamble and channel settings.

	@param[out] msgBuffer The String buffer to be filled with printable device information.
		Provide 128 bytes, this should be sufficient.
	*/
	static void getPrintableDeviceMode(char msgBuffer[]);
	
	/* ##### Device address management, filters ################################## */
	/** 
	(Re-)set the network identifier which the selected chip should be associated with. This
	setting is important for certain MAC address filtering rules.

	@param[in] val An arbitrary numeric network identifier.
	*/
	
	
	static void setNetworkId(uint16_t val);
	
	/** 
	(Re-)set the device address (i.e. short address) for the currently selected chip. This
	setting is important for certain MAC address filtering rules.

	@param[in] val An arbitrary numeric device address.
	*/
	static void setDeviceAddress(uint16_t val);
	// TODO MAC and filters
	
	static void setEUI(char eui[]);
	static void setEUI(byte eui[]);
	
	/* ##### General device configuration ######################################## */
	/** 
	Specifies whether the DWM1000 chip should, again, turn on its receiver in case that the
	last reception failed. 

	This setting is enabled as part of `setDefaults()` if the device is
	in idle mode.

	@param[in] val `true` to enable, `false` to disable receiver auto-reenable.
	*/
	static void setReceiverAutoReenable(boolean val);
	
	/** 
	Specifies the interrupt polarity of the DWM1000 chip. 

	As part of `setDefaults()` if the device is in idle mode, interrupt polarity is set to 
	active high.

	@param[in] val `true` for active high interrupts, `false` for active low interrupts.
	*/
	static void setInterruptPolarity(boolean val);
	
	/** 
	Specifies whether to suppress any frame check measures while sending or receiving messages.
	If suppressed, no 2-byte checksum is appended to the message before sending and this 
	checksum is also not expected at receiver side. Note that when suppressing frame checks, 
	the error event handler	(attached via `attachReceiveErrorHandler()`) will not be triggered 
	if received data is corrupted.

	Frame checks are enabled as part of `setDefaults()` if the device is in idle mode.

	@param[in] val `true` to suppress frame check on sender and receiver side, `false` otherwise.
	*/
	static void suppressFrameCheck(boolean val);

	/* 
	Set internal helper for better tuning of values in NLOS applications 
	You should use PRF at 16 Mhz and preamble code length of 1024 or greater as stated by application notes.
	   
	With NLOS optimized values you have a higher chance of incorrect readings (~5%)
	For further information consult DW1000 Application notes APS006 - Part 2

	@param[in] val `true` to set nlos optimizations, `false` otherwise.
	*/
	static void setNLOS(boolean val);
	
	/** 
	Specifies the data transmission rate of the DWM1000 chip. One of the values
	- `TRX_RATE_110KBPS` (i.e. 110 kb/s)
	- `TRX_RATE_850KBPS` (i.e. 850 kb/s)
	- `TRX_RATE_6800KBPS` (i.e. 6.8 Mb/s)
	has to be provided.

	(see chapters 9.1/2/3/4 DW1000 user manual)

	See `setDefaults()` and `enableMode()` for additional information on data rate settings.

	WARNING: It resets SFD configuration to recommended in table 21 of DW1000 User manual.

	@param[in] rate The data transmission rate, encoded by the above defined constants.
	*/
	static void setDataRate(byte rate);
	
	/** 
	Specifies the pulse repetition frequency (PRF) of data transmissions with the DWM1000. Either
	- `TX_PULSE_FREQ_16MHZ` (i.e. 16 MHz)
	- `TX_PULSE_FREQ_64MHZ` (i.e. 64 MHz)
	has to be chosen.

	Note that the 16 MHz setting is more power efficient, while the 64 MHz setting requires more
	power, but also delivers slightly better transmission performance (i.e. on communication range and 
	timestamp accuracy) 
	(see chapters 9.1/2/3/4 DW1000 user manual).

	See `setDefaults()` and `enableMode()` for additional information on PRF settings.

	@param[in] freq The PRF, encoded by the above defined constants.
	*/
	static void setPulseFrequency(byte freq);

	static byte getPulseFrequency();
	static void setPreambleLength(byte prealen);

	static void setSFDMode(SFDMode mode);
	
	/*
	Specifies the channel for transmitting and receiving to and from a DWM1000 module. Once of this values
	- `CHANNEL_1` (i.e. 3494.4 MHz, bandwidth ~500 MHz)
	- `CHANNEL_2` (i.e. 3993.6 MHz, bandwidth ~500 MHz)
	- `CHANNEL_3` (i.e. 4492.8 MHz, bandwidth ~500 MHz)
	- `CHANNEL_4` (i.e. 3993.6 MHz, bandwidth ~1332 MHz)
	- `CHANNEL_5` (i.e. 6489.6 MHz, bandwidth ~500 MHz)
	- `CHANNEL_7` (i.e. 6489.6 MHz, bandwidth ~1082 MHz)
	has to be chosen.
	
	Channels 4 and 7 have a wider bandwidth. Operating at a wider bandwidth increases range but also increases power consumption.
	Channels 4 and 7 in reception have a maximum receive bandwidth of 900 MHz.
	The Operating range also varies depending on the channel centre frequency and channel bandwidth selected 
	(see chapters 9.1/2/3/4 DW1000 user manual)

	@param[in] channel The Number of channel, encoded by the above defined constants.
	 */
	static void setChannel(byte channel);

	static void setPreambleCode();
	static void setPreambleCode(byte preamble_code);
	static void useSmartPower(boolean smartPower);

	static void setTXPower(int32_t power);
	static void setTXPower(DriverAmplifierValue driver_amplifier, TransmitMixerValue mixer);
	
	
	/* transmit and receive configuration. */
	static DWM1000Time   setDelay(const DWM1000Time& delay);
	static void         receivePermanently(boolean val);
	static void         setData(byte data[], uint16_t n);
	static void         setData(const String& data);
	static void         getData(byte data[], uint16_t n);
	static void         getData(String& data);
	static uint16_t     getDataLength();
	static void         getTransmitTimestamp(DWM1000Time& time);
	static void         getReceiveTimestamp(DWM1000Time& time);
	static void         getSystemTimestamp(DWM1000Time& time);
	static void         getTransmitTimestamp(byte data[]);
	static void         getReceiveTimestamp(byte data[]);
	static void         getSystemTimestamp(byte data[]);
	
	/* receive quality information. (RX_FSQUAL) - reg:0x12 */
	static float getReceivePower();
	static float getFirstPathPower();
	static float getReceiveQuality();
	
	/* interrupt management. */
	static void interruptOnSent(boolean val);
	static void interruptOnReceived(boolean val);
	static void interruptOnReceiveFailed(boolean val);
	static void interruptOnReceiveTimeout(boolean val);
	static void interruptOnReceiveTimestampAvailable(boolean val);
	static void interruptOnAutomaticAcknowledgeTrigger(boolean val);

	/* Antenna delay calibration */
	static void setAntennaDelay(const uint16_t value);
	static uint16_t getAntennaDelay();

	/* callback handler management. */
	static void attachErrorHandler(void (* handleError)(void)) {
		_handleError = handleError;
	}
	
	static void attachSentHandler(void (* handleSent)(void)) {
		_handleSent = handleSent;
	}
	
	static void attachReceivedHandler(void (* handleReceived)(void)) {
		_handleReceived = handleReceived;
	}
	
	static void attachReceiveFailedHandler(void (* handleReceiveFailed)(void)) {
		_handleReceiveFailed = handleReceiveFailed;
	}
	
	static void attachReceiveTimeoutHandler(void (* handleReceiveTimeout)(void)) {
		_handleReceiveTimeout = handleReceiveTimeout;
	}
	
	static void attachReceiveTimestampAvailableHandler(void (* handleReceiveTimestampAvailable)(void)) {
		_handleReceiveTimestampAvailable = handleReceiveTimestampAvailable;
	}
	
	/* device state management. */
	// idle state
	static void idle();
	
	// general configuration state
	static void newConfiguration();

	static void commitConfiguration(TXPowerMode mode = TXPowerMode::AUTO_POWER);
	
	// reception state
	static void newReceive();
	static void startReceive();
	
	// transmission state
	static void newTransmit();
	static void startTransmit();
	
	/* ##### Operation mode selection ############################################ */
	/** 
	Specifies the mode of operation for the DWM1000. Modes of operation are pre-defined
	combinations of data rate, pulse repetition frequency, preamble and channel settings
	that properly go together. If you simply want the chips to work, choosing a mode is 
	preferred over manual configuration.

	The following modes are pre-configured and one of them needs to be chosen:
	- `MODE_SHORTRANGE_LOWPRF_SHORTPREAMBLE` (basically this is 6,8 Mb/s data rate, 16 MHz PRF and short preambles)
	- `MODE_SHORTRANGE_HIGHPRF_SHORTPREAMBLE` (basically this is 6,8 Mb/s data rate, 64 MHz PRF and short preambles)
	- `MODE_SHORTRANGE_LOWPRF_MEDIUMPREAMBLE` (basically this is 6,8 Mb/s data rate, 16 MHz PRF and medium preambles)
	- `MODE_SHORTRANGE_HIGHPRF_MEDIUMPREAMBLE` (basically this is 6,8 Mb/s data rate, 64 MHz PRF and medium preambles)
	- `MODE_SHORTRANGE_LOWPRF_LONGPREAMBLE` (basically this is 6,8 Mb/s data rate, 16 MHz PRF and long preambles)
	- `MODE_SHORTRANGE_HIGHPRF_LONGPREAMBLE` (basically this is 6,8 Mb/s data rate, 64 MHz PRF and long preambles)

	- `MODE_MEDIUMRANGE_LOWPRF_SHORTPREAMBLE` (basically this is 850 kb/s data rate, 16 MHz PRF and short preambles)
	- `MODE_MEDIUMRANGE_HIGHPRF_SHORTPREAMBLE` (basically this is 850 kb/s data rate, 64 MHz PRF and short preambles)
	- `MODE_MEDIUMRANGE_LOWPRF_MEDIUMPREAMBLE` (basically this is 850 kb/s data rate, 16 MHz PRF and medium preambles)
	- `MODE_MEDIUMRANGE_HIGHPRF_MEDIUMPREAMBLE` (basically this is 850 kb/s data rate, 64 MHz PRF and medium preambles)
	- `MODE_MEDIUMRANGE_LOWPRF_LONGPREAMBLE` (basically this is 850 kb/s data rate, 16 MHz PRF and long preambles)
	- `MODE_MEDIUMRANGE_HIGHPRF_LONGPREAMBLE` (basically this is 850 kb/s data rate, 64 MHz PRF and long preambles)

	- `MODE_LONGRANGE_LOWPRF_SHORTPREAMBLE` (basically this is 110 kb/s data rate, 16 MHz PRF and short preambles)
	- `MODE_LONGRANGE_HIGHPRF_SHORTPREAMBLE` (basically this is 110 kb/s data rate, 64 MHz PRF and short preambles)

	Note that SHORTRANGE and SHORTPREAMBLE refers to the better power efficiency and improved transmission performance
	of 16 MHZ and 64 MHZ PRF respectively (see `setPulseFrequency()`).

	The default setting that is selected by `setDefaults()` is MODE_LONGRANGE_LOWPRF_SHORTPREAMBLE.

	@param[in] mode The mode of operation, encoded by the above defined constants.
	*/
	static void enableMode(const byte mode[]);
	
	/* use RX/TX specific and general default settings */
	static void setDefaults();
	
	/* host-initiated reading of temperature and battery voltage */
	static void getTemp(float& temp);
	static void getVbat(float& vbat);
	static void getTempAndVbat(float& temp, float& vbat);

	/* Debug helpers */
	static void getPrettyBytes(byte data[], char msgBuffer[], uint16_t n);
	static void getPrettyBytes(byte cmd, uint16_t offset, char msgBuffer[], uint16_t n);

//private:
	/* chip select, reset and interrupt pins. */
	static uint8_t _ss;
	static uint8_t _rst;
	static uint8_t _irq;
	
	/* callbacks. */
	static void (* _handleError)(void);
	static void (* _handleSent)(void);
	static void (* _handleReceived)(void);
	static void (* _handleReceiveFailed)(void);
	static void (* _handleReceiveTimeout)(void);
	static void (* _handleReceiveTimestampAvailable)(void);

	/* sfd mode */
	static void (* _currentSFDMode)(void); 
	
	/* register caches. */
	static byte _syscfg[LEN_SYS_CFG];
	static byte _sysctrl[LEN_SYS_CTRL];
	static byte _sysstatus[LEN_SYS_STATUS];
	static byte _txfctrl[LEN_TX_FCTRL];
	static byte _sysmask[LEN_SYS_MASK];
	static byte _chanctrl[LEN_CHAN_CTRL];
	
	/* device status monitoring */
	static byte _vmeas3v3;
	static byte _tmeas23C;

	/* PAN and short address. */
	static byte _networkAndAddress[LEN_PANADR];
	
	/* internal helper that guide tuning the chip. */
	static boolean    _smartPower;
	static byte       _extendedFrameLength;
	static byte       _preambleCode;
	static byte       _channel;
	static byte       _preambleLength;
	static byte       _pulseFrequency;
	static byte       _dataRate;
	static byte       _pacSize;
	static DWM1000Time _antennaDelay;
	
	/* internal helper to remember how to properly act. */
	static boolean _permanentReceive;
	static boolean _frameCheck;
	static boolean _nlos;
	
	/* whether RX or TX is active */
	static uint8_t _deviceMode;

	/* whether debounce clock is active */
	static boolean _debounceClockEnabled;

	/* Arduino interrupt handler */
	static void handleInterrupt();
	
	/* Allow MAC frame filtering */
	// TODO auto-acknowledge
	static void setFrameFilter(boolean val);
	static void setFrameFilterBehaveCoordinator(boolean val);
	static void setFrameFilterAllowBeacon(boolean val);
	//data type is used in the FC_1 0x41
	static void setFrameFilterAllowData(boolean val);
	static void setFrameFilterAllowAcknowledgement(boolean val);
	static void setFrameFilterAllowMAC(boolean val);
	//Reserved is used for the Blink message
	static void setFrameFilterAllowReserved(boolean val);
	
	// note: not sure if going to be implemented for now
	static void setDoubleBuffering(boolean val);
	// TODO is implemented, but needs testing
	static void useExtendedFrameLength(boolean val);
	// TODO is implemented, but needs testing
	static void waitForResponse(boolean val);

	// Helper functions for tune
	static void agctune1();
	static void agctune2();
	static void agctune3();
	static void drxtune0b();
	static void drxtune1a();
	static void drxtune1b();
	static void drxtune2();
	static void drxtune4H();
	static void ldecfg1();
	static void ldecfg2();
	static void lderepc();
	static void txpower();
	static void rfrxctrlh();
	static void rftxctrl();
	static void tcpgdelay();
	static void fspll();
	static void fsxtalt();
	
	/* function to write configurations to registers */
	static void writeConfiguration();

	/* tuning according to mode.(Very important) */
	static void tune(TXPowerMode mode);
	
	/* device status flags */
	static boolean isReceiveTimestampAvailable();
	static boolean isTransmitDone();
	static boolean isReceiveDone();
	static boolean isReceiveFailed();
	static boolean isReceiveTimeout();
	static boolean isClockProblem();
	
	/* interrupt state handling */
	static void clearInterrupts();
	static void clearAllStatus();
	static void clearReceiveStatus();
	static void clearReceiveTimestampAvailableStatus();
	static void clearTransmitStatus();
	
	/* internal helper to read/write system registers. */
	static void readSystemEventStatusRegister();
	static void readSystemConfigurationRegister();
	static void writeSystemConfigurationRegister();
	static void readNetworkIdAndDeviceAddress();
	static void writeNetworkIdAndDeviceAddress();
	static void readSystemEventMaskRegister();
	static void writeSystemEventMaskRegister();
	static void readChannelControlRegister();
	static void writeChannelControlRegister();
	static void readTransmitFrameControlRegister();
	static void writeTransmitFrameControlRegister();
	static void writeAntennaDelayRegisters();
	
	/* clock management. */
	static void enableClock(byte clock);
	
	/* LDE micro-code management. */
	static void manageLDE();
	
	/* timestamp correction. */
	static void correctTimestamp(DWM1000Time& timestamp);
	
	/* reading and writing bytes from and to DWM1000 module. */
	static void readBytes(byte cmd, uint16_t offset, byte data[], uint16_t n);
	static void readBytesOTP(uint16_t address, byte data[]);
	static void writeByte(byte cmd, uint16_t offset, byte data);
	static void writeBytes(byte cmd, uint16_t offset, byte data[], uint16_t n);

	/* SFDMode functions */
	static void useDecawaveSFD();
	static void useStandardSFD();
	static void useRecommendedSFD();

	/* SPI configs. */
	static const SPISettings _fastSPI;
	static const SPISettings _slowSPI;
	static const SPISettings* _currentSPI;
};

extern DWM1000Class DWM1000;
