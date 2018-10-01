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
 * @file DW1000Ng.cpp
 * Arduino driver library (source file) for the Decawave DW1000Ng UWB transceiver Module.
 */

#include <stdio.h>
#include <stdlib.h>
#include <SPI.h>
#include <string.h>
#include "DW1000NgUtils.hpp"
#include "DW1000NgConstants.hpp"
#include "DW1000NgRegisters.hpp"
#include "DW1000Ng.hpp"
#include "DW1000NgTime.hpp"

namespace DW1000Ng {
	
	/* anonymous namespace to host private-like variables and methods */
	namespace {

		/* ########################### PRIVATE VARIABLES ################################# */

		/* SPI select pin and interrupt pin*/
		uint8_t _ss = 0xff;
		uint8_t _irq = 0xff;
		uint8_t _rst = 0xff;

		/* IRQ callbacks */
		void (* _handleSent)(void)                      = nullptr;
		void (* _handleError)(void)                     = nullptr;
		void (* _handleReceived)(void)                  = nullptr;
		void (* _handleReceiveFailed)(void)             = nullptr;
		void (* _handleReceiveTimeout)(void)            = nullptr;
		void (* _handleReceiveTimestampAvailable)(void) = nullptr;

		/* registers */
		byte       _syscfg[LEN_SYS_CFG];
		byte       _sysctrl[LEN_SYS_CTRL];
		byte       _sysstatus[LEN_SYS_STATUS];
		byte       _txfctrl[LEN_TX_FCTRL];
		byte       _sysmask[LEN_SYS_MASK];
		byte       _chanctrl[LEN_CHAN_CTRL];
		byte       _networkAndAddress[LEN_PANADR];

		/* Temperature and Voltage monitoring */
		byte _vmeas3v3 = 0;
		byte _tmeas23C = 0;

		/* Driver Internal State Trackers */
		byte        	_extendedFrameLength;
		PacSize        	_pacSize;
		PulseFrequency	_pulseFrequency;
		DataRate        _dataRate;
		PreambleLength	_preambleLength;
		PreambleCode	_preambleCode;
		Channel        	_channel;
		boolean     	_smartPower;
		boolean     	_frameCheck;
		boolean     	_debounceClockEnabled = false;
		boolean     	_nlos = false;
		boolean			_standardSFD = true;
		boolean     	_autoTXPower = true;
		boolean     	_autoTCPGDelay = true;
		uint16_t		_antennaTxDelay = 0;
		uint16_t		_antennaRxDelay = 0;

		/* SPI relative variables */
		const SPISettings  _fastSPI = SPISettings(16000000L, MSBFIRST, SPI_MODE0);
		const SPISettings  _slowSPI = SPISettings(2000000L, MSBFIRST, SPI_MODE0);
		const SPISettings* _currentSPI = &_fastSPI;

		/* ############################# PRIVATE METHODS ################################### */
		/*
		* Write bytes to the DW1000Ng. Single bytes can be written to registers via sub-addressing.
		* @param cmd
		* 		The register address (see Chapter 7 in the DW1000Ng user manual).
		* @param offset
		*		The offset to select register sub-parts for writing, or 0x00 to disable
		* 		sub-adressing.
		* @param data
		*		The data array to be written.
		* @param data_size
		*		The number of bytes to be written (take care not to go out of bounds of
		* 		the register).
		*/
		// TODO offset really bigger than byte?

		void _writeBytesToRegister(byte cmd, uint16_t offset, byte data[], uint16_t data_size) {
			byte header[3];
			uint8_t  headerLen = 1;
			uint16_t  i = 0;
			
			// TODO proper error handling: address out of bounds
			// build SPI header
			if(offset == NO_SUB) {
				header[0] = WRITE | cmd;
			} else {
				header[0] = WRITE_SUB | cmd;
				if(offset < 128) {
					header[1] = (byte)offset;
					headerLen++;
				} else {
					header[1] = RW_SUB_EXT | (byte)offset;
					header[2] = (byte)(offset >> 7);
					headerLen += 2;
				}
			}
			SPI.beginTransaction(*_currentSPI);
			digitalWrite(_ss, LOW);
			for(i = 0; i < headerLen; i++) {
				SPI.transfer(header[i]); // send header
			}
			for(i = 0; i < data_size; i++) {
				SPI.transfer(data[i]); // write values
			}
			delayMicroseconds(5);
			digitalWrite(_ss, HIGH);
			SPI.endTransaction();
		}

		void _writeToRegister(byte cmd, uint16_t offset, uint32_t data, uint16_t data_size) { 
			byte dataBytes[data_size];
			DW1000NgUtils::writeValueToBytes(dataBytes, data, data_size);
			_writeBytesToRegister(cmd, offset, dataBytes, data_size);
		}

		// Helper to set a single register
		void _writeByte(byte cmd, uint16_t offset, byte data) {
			_writeBytesToRegister(cmd, offset, &data, 1);
		}
		
		/*
		* Read bytes from the DW1000Ng. Number of bytes depend on register length.
		* @param cmd
		* 		The register address (see Chapter 7 in the DW1000Ng user manual).
		* @param data
		*		The data array to be read into.
		* @param n
		*		The number of bytes expected to be received.
		*/
		// TODO incomplete doc
		void _readBytes(byte cmd, uint16_t offset, byte data[], uint16_t n) {
			byte header[3];
			uint8_t headerLen = 1;
			uint16_t i = 0;
			
			// build SPI header
			if(offset == NO_SUB) {
				header[0] = READ | cmd;
			} else {
				header[0] = READ_SUB | cmd;
				if(offset < 128) {
					header[1] = (byte)offset;
					headerLen++;
				} else {
					header[1] = RW_SUB_EXT | (byte)offset;
					header[2] = (byte)(offset >> 7);
					headerLen += 2;
				}
			}
			SPI.beginTransaction(*_currentSPI);
			digitalWrite(_ss, LOW);
			for(i = 0; i < headerLen; i++) {
				SPI.transfer(header[i]); // send header
			}
			for(i = 0; i < n; i++) {
				data[i] = SPI.transfer(0x00); // read values
			}
			delayMicroseconds(5);
			digitalWrite(_ss, HIGH);
			SPI.endTransaction();
		}

		// always 4 bytes
		// TODO why always 4 bytes? can be different, see p. 58 table 10 otp memory map
		void _readBytesOTP(uint16_t address, byte data[]) {
			byte addressBytes[LEN_OTP_ADDR];
			
			// p60 - 6.3.3 Reading a value from OTP memory
			// bytes of address
			addressBytes[0] = (address & 0xFF);
			addressBytes[1] = ((address >> 8) & 0xFF);
			// set address
			_writeBytesToRegister(OTP_IF, OTP_ADDR_SUB, addressBytes, LEN_OTP_ADDR);
			// switch into read mode
			_writeByte(OTP_IF, OTP_CTRL_SUB, 0x03); // OTPRDEN | OTPREAD
			_writeByte(OTP_IF, OTP_CTRL_SUB, 0x01); // OTPRDEN
			// read value/block - 4 bytes
			_readBytes(OTP_IF, OTP_RDAT_SUB, data, LEN_OTP_RDAT);
			// end read mode
			_writeByte(OTP_IF, OTP_CTRL_SUB, 0x00);
		}
		
		/* Steps used to get Temp and Voltage */
		void _vbatAndTempSteps() {
			byte step1 = 0x80; _writeBytesToRegister(RF_CONF, 0x11, &step1, 1);
			byte step2 = 0x0A; _writeBytesToRegister(RF_CONF, 0x12, &step2, 1);
			byte step3 = 0x0F; _writeBytesToRegister(RF_CONF, 0x12, &step3, 1);
			byte step4 = 0x01; _writeBytesToRegister(TX_CAL, NO_SUB, &step4, 1);
			byte step5 = 0x00; _writeBytesToRegister(TX_CAL, NO_SUB, &step5, 1);
		}

		/* AGC_TUNE1 - reg:0x23, sub-reg:0x04, table 24 */
		void _agctune1() {
			byte agctune1[LEN_AGC_TUNE1];
			if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
				DW1000NgUtils::writeValueToBytes(agctune1, 0x8870, LEN_AGC_TUNE1);
			} else if(_pulseFrequency == PulseFrequency::FREQ_64MHZ) {
				DW1000NgUtils::writeValueToBytes(agctune1, 0x889B, LEN_AGC_TUNE1);
			} else {
				// TODO proper error/warning handling
			}
			_writeBytesToRegister(AGC_TUNE, AGC_TUNE1_SUB, agctune1, LEN_AGC_TUNE1);
		}

		/* AGC_TUNE2 - reg:0x23, sub-reg:0x0C, table 25 */
		void _agctune2() {
			byte agctune2[LEN_AGC_TUNE2];
			DW1000NgUtils::writeValueToBytes(agctune2, 0x2502A907L, LEN_AGC_TUNE2);
			_writeBytesToRegister(AGC_TUNE, AGC_TUNE2_SUB, agctune2, LEN_AGC_TUNE2);
		}

		/* AGC_TUNE3 - reg:0x23, sub-reg:0x12, table 26 */
		void _agctune3() {
			byte agctune3[LEN_AGC_TUNE3];
			DW1000NgUtils::writeValueToBytes(agctune3, 0x0035, LEN_AGC_TUNE3);
			_writeBytesToRegister(AGC_TUNE, AGC_TUNE3_SUB, agctune3, LEN_AGC_TUNE3);
		}

		/* DRX_TUNE0b - reg:0x27, sub-reg:0x02, table 30 */
		void _drxtune0b() {
			byte drxtune0b[LEN_DRX_TUNE0b];
			if(_dataRate == DataRate::RATE_110KBPS) {
				if(!_standardSFD) {
					DW1000NgUtils::writeValueToBytes(drxtune0b, 0x0016, LEN_DRX_TUNE0b);
				} else {
					DW1000NgUtils::writeValueToBytes(drxtune0b, 0x000A, LEN_DRX_TUNE0b);
				}
			} else if(_dataRate == DataRate::RATE_850KBPS) {
				if(!_standardSFD) {
					DW1000NgUtils::writeValueToBytes(drxtune0b, 0x0006, LEN_DRX_TUNE0b);
				} else {
					DW1000NgUtils::writeValueToBytes(drxtune0b, 0x0001, LEN_DRX_TUNE0b);
				}
			} else if(_dataRate == DataRate::RATE_6800KBPS) {
				if(!_standardSFD) {
					DW1000NgUtils::writeValueToBytes(drxtune0b, 0x0002, LEN_DRX_TUNE0b);
				} else {
					DW1000NgUtils::writeValueToBytes(drxtune0b, 0x0001, LEN_DRX_TUNE0b);
				}
			} else {
				// TODO proper error/warning handling
			}
			_writeBytesToRegister(DRX_TUNE, DRX_TUNE0b_SUB, drxtune0b, LEN_DRX_TUNE0b);
		}

		/* DRX_TUNE1a - reg:0x27, sub-reg:0x04, table 31 */
		void _drxtune1a() {
			byte drxtune1a[LEN_DRX_TUNE1a];
			if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
				DW1000NgUtils::writeValueToBytes(drxtune1a, 0x0087, LEN_DRX_TUNE1a);
			} else if(_pulseFrequency == PulseFrequency::FREQ_64MHZ) {
				DW1000NgUtils::writeValueToBytes(drxtune1a, 0x008D, LEN_DRX_TUNE1a);
			} else {
				// TODO proper error/warning handling
			}
			_writeBytesToRegister(DRX_TUNE, DRX_TUNE1a_SUB, drxtune1a, LEN_DRX_TUNE1a);
		}

		/* DRX_TUNE1b - reg:0x27, sub-reg:0x06, table 32 */
		void _drxtune1b() {
			byte drxtune1b[LEN_DRX_TUNE1b];
			if(_preambleLength == PreambleLength::LEN_1536 || _preambleLength == PreambleLength::LEN_2048 ||
				_preambleLength == PreambleLength::LEN_4096) {
				if(_dataRate == DataRate::RATE_110KBPS) {
					DW1000NgUtils::writeValueToBytes(drxtune1b, 0x0064, LEN_DRX_TUNE1b);
				} else {
					// TODO proper error/warning handling
				}
			} else if(_preambleLength != PreambleLength::LEN_64) {
				if(_dataRate == DataRate::RATE_850KBPS || _dataRate == DataRate::RATE_6800KBPS) {
					DW1000NgUtils::writeValueToBytes(drxtune1b, 0x0020, LEN_DRX_TUNE1b);
				} else {
					// TODO proper error/warning handling
				}
			} else {
				if(_dataRate == DataRate::RATE_6800KBPS) {
					DW1000NgUtils::writeValueToBytes(drxtune1b, 0x0010, LEN_DRX_TUNE1b);
				} else {
					// TODO proper error/warning handling
				}
			}
			_writeBytesToRegister(DRX_TUNE, DRX_TUNE1b_SUB, drxtune1b, LEN_DRX_TUNE1b);
		}

		/* DRX_TUNE2 - reg:0x27, sub-reg:0x08, table 33 */
		void _drxtune2() {
			byte drxtune2[LEN_DRX_TUNE2];	
			if(_pacSize == PacSize::SIZE_8) {
				if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
					DW1000NgUtils::writeValueToBytes(drxtune2, 0x311A002DL, LEN_DRX_TUNE2);
				} else if(_pulseFrequency == PulseFrequency::FREQ_64MHZ) {
					DW1000NgUtils::writeValueToBytes(drxtune2, 0x313B006BL, LEN_DRX_TUNE2);
				} else {
					// TODO proper error/warning handling
				}
			} else if(_pacSize == PacSize::SIZE_16) {
				if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
					DW1000NgUtils::writeValueToBytes(drxtune2, 0x331A0052L, LEN_DRX_TUNE2);
				} else if(_pulseFrequency == PulseFrequency::FREQ_64MHZ) {
					DW1000NgUtils::writeValueToBytes(drxtune2, 0x333B00BEL, LEN_DRX_TUNE2);
				} else {
					// TODO proper error/warning handling
				}
			} else if(_pacSize == PacSize::SIZE_32) {
				if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
					DW1000NgUtils::writeValueToBytes(drxtune2, 0x351A009AL, LEN_DRX_TUNE2);
				} else if(_pulseFrequency == PulseFrequency::FREQ_64MHZ) {
					DW1000NgUtils::writeValueToBytes(drxtune2, 0x353B015EL, LEN_DRX_TUNE2);
				} else {
					// TODO proper error/warning handling
				}
			} else if(_pacSize == PacSize::SIZE_64) {
				if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
					DW1000NgUtils::writeValueToBytes(drxtune2, 0x371A011DL, LEN_DRX_TUNE2);
				} else if(_pulseFrequency == PulseFrequency::FREQ_64MHZ) {
					DW1000NgUtils::writeValueToBytes(drxtune2, 0x373B0296L, LEN_DRX_TUNE2);
				} else {
					// TODO proper error/warning handling
				}
			} else {
				// TODO proper error/warning handling
			}
			_writeBytesToRegister(DRX_TUNE, DRX_TUNE2_SUB, drxtune2, LEN_DRX_TUNE2);
		}

		/* DRX_TUNE4H - reg:0x27, sub-reg:0x26, table 34 */
		void _drxtune4H() {
			byte drxtune4H[LEN_DRX_TUNE4H];
			if(_preambleLength == PreambleLength::LEN_64) {
				DW1000NgUtils::writeValueToBytes(drxtune4H, 0x0010, LEN_DRX_TUNE4H);
			} else {
				DW1000NgUtils::writeValueToBytes(drxtune4H, 0x0028, LEN_DRX_TUNE4H);
			}
			_writeBytesToRegister(DRX_TUNE, DRX_TUNE4H_SUB, drxtune4H, LEN_DRX_TUNE4H);
		}

		/* LDE_CFG1 - reg 0x2E, sub-reg:0x0806 */
		void _ldecfg1() {
			byte ldecfg1[LEN_LDE_CFG1];
			_nlos == true ? DW1000NgUtils::writeValueToBytes(ldecfg1, 0x7, LEN_LDE_CFG1) : DW1000NgUtils::writeValueToBytes(ldecfg1, 0xD, LEN_LDE_CFG1);
			_writeBytesToRegister(LDE_IF, LDE_CFG1_SUB, ldecfg1, LEN_LDE_CFG1);
		}

		/* LDE_CFG2 - reg 0x2E, sub-reg:0x1806, table 50 */
		void _ldecfg2() {
			byte ldecfg2[LEN_LDE_CFG2];	
			if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
				_nlos == true ? DW1000NgUtils::writeValueToBytes(ldecfg2, 0x0003, LEN_LDE_CFG2) : DW1000NgUtils::writeValueToBytes(ldecfg2, 0x1607, LEN_LDE_CFG2);
			} else if(_pulseFrequency == PulseFrequency::FREQ_64MHZ) {
				DW1000NgUtils::writeValueToBytes(ldecfg2, 0x0607, LEN_LDE_CFG2);
			} else {
				// TODO proper error/warning handling
			}
			_writeBytesToRegister(LDE_IF, LDE_CFG2_SUB, ldecfg2, LEN_LDE_CFG2);
		}

		/* LDE_REPC - reg 0x2E, sub-reg:0x2804, table 51 */
		void _lderepc() {
			byte lderepc[LEN_LDE_REPC];
			if(_preambleCode == PreambleCode::CODE_1 || _preambleCode == PreambleCode::CODE_2) {
				if(_dataRate == DataRate::RATE_110KBPS) {
					DW1000NgUtils::writeValueToBytes(lderepc, ((0x5998 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DW1000NgUtils::writeValueToBytes(lderepc, 0x5998, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PreambleCode::CODE_3 || _preambleCode == PreambleCode::CODE_8) {
				if(_dataRate == DataRate::RATE_110KBPS) {
					DW1000NgUtils::writeValueToBytes(lderepc, ((0x51EA >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DW1000NgUtils::writeValueToBytes(lderepc, 0x51EA, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PreambleCode::CODE_4) {
				if(_dataRate == DataRate::RATE_110KBPS) {
					DW1000NgUtils::writeValueToBytes(lderepc, ((0x428E >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DW1000NgUtils::writeValueToBytes(lderepc, 0x428E, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PreambleCode::CODE_5) {
				if(_dataRate == DataRate::RATE_110KBPS) {
					DW1000NgUtils::writeValueToBytes(lderepc, ((0x451E >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DW1000NgUtils::writeValueToBytes(lderepc, 0x451E, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PreambleCode::CODE_6) {
				if(_dataRate == DataRate::RATE_110KBPS) {
					DW1000NgUtils::writeValueToBytes(lderepc, ((0x2E14 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DW1000NgUtils::writeValueToBytes(lderepc, 0x2E14, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PreambleCode::CODE_7) {
				if(_dataRate == DataRate::RATE_110KBPS) {
					DW1000NgUtils::writeValueToBytes(lderepc, ((0x8000 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DW1000NgUtils::writeValueToBytes(lderepc, 0x8000, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PreambleCode::CODE_9) {
				if(_dataRate == DataRate::RATE_110KBPS) {
					DW1000NgUtils::writeValueToBytes(lderepc, ((0x28F4 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DW1000NgUtils::writeValueToBytes(lderepc, 0x28F4, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PreambleCode::CODE_10 || _preambleCode == PreambleCode::CODE_17) {
				if(_dataRate == DataRate::RATE_110KBPS) {
					DW1000NgUtils::writeValueToBytes(lderepc, ((0x3332 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DW1000NgUtils::writeValueToBytes(lderepc, 0x3332, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PreambleCode::CODE_11) {
				if(_dataRate == DataRate::RATE_110KBPS) {
					DW1000NgUtils::writeValueToBytes(lderepc, ((0x3AE0 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DW1000NgUtils::writeValueToBytes(lderepc, 0x3AE0, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PreambleCode::CODE_12) {
				if(_dataRate == DataRate::RATE_110KBPS) {
					DW1000NgUtils::writeValueToBytes(lderepc, ((0x3D70 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DW1000NgUtils::writeValueToBytes(lderepc, 0x3D70, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PreambleCode::CODE_18 || _preambleCode == PreambleCode::CODE_19) {
				if(_dataRate == DataRate::RATE_110KBPS) {
					DW1000NgUtils::writeValueToBytes(lderepc, ((0x35C2 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DW1000NgUtils::writeValueToBytes(lderepc, 0x35C2, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PreambleCode::CODE_20) {
				if(_dataRate == DataRate::RATE_110KBPS) {
					DW1000NgUtils::writeValueToBytes(lderepc, ((0x47AE >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DW1000NgUtils::writeValueToBytes(lderepc, 0x47AE, LEN_LDE_REPC);
				}
			} else {
				// TODO proper error/warning handling
			}
			
			_writeBytesToRegister(LDE_IF, LDE_REPC_SUB, lderepc, LEN_LDE_REPC);
		}

		/* TX_POWER (enabled smart transmit power control) - reg:0x1E, tables 19-20
		* These values are based on a typical IC and an assumed IC to antenna loss of 1.5 dB with a 0 dBi antenna */
		void _txpowertune() {
			byte txpower[LEN_TX_POWER];
			if(_channel == Channel::CHANNEL_1 || _channel == Channel::CHANNEL_2) {
				if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
					if(_smartPower) {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x1B153555L, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x15355575L, LEN_TX_POWER);
						#endif
					} else {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x55555555L, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x75757575L, LEN_TX_POWER);
						#endif
					}
				} else if(_pulseFrequency == PulseFrequency::FREQ_64MHZ) {
					if(_smartPower) {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x0D072747L, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x07274767L, LEN_TX_POWER);
						#endif
					} else {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x47474747L, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x67676767L, LEN_TX_POWER);
						#endif
					}
				} else {
					// TODO proper error/warning handling
				}
			} else if(_channel == Channel::CHANNEL_3) {
				if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
					if(_smartPower) {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x150F2F4FL, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x0F2F4F6FL, LEN_TX_POWER);
						#endif
					} else {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x4F4F4F4FL, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x6F6F6F6FL, LEN_TX_POWER);
						#endif
					}
				} else if(_pulseFrequency == PulseFrequency::FREQ_64MHZ) {
					if(_smartPower) {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x0B2B4B6BL, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x2B4B6B8BL, LEN_TX_POWER);
						#endif
					} else {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x6B6B6B6BL, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x8B8B8B8BL, LEN_TX_POWER);
						#endif
					}
				} else {
					// TODO proper error/warning handling
				}
			} else if(_channel == Channel::CHANNEL_4) {
				if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
					if(_smartPower) {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x1F1F1F3FL, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x1F1F3F5FL, LEN_TX_POWER);
						#endif
					} else {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x3F3F3F3FL, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x5F5F5F5FL, LEN_TX_POWER);
						#endif
					}
				} else if(_pulseFrequency == PulseFrequency::FREQ_64MHZ) {
					if(_smartPower) {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x1A3A5A7AL, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x3A5A7A9AL, LEN_TX_POWER);
						#endif
					} else {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x7A7A7A7AL, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x9A9A9A9AL, LEN_TX_POWER);
						#endif
					}
				} else {
					// TODO proper error/warning handling
				}
			} else if(_channel == Channel::CHANNEL_5) {
				if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
					if(_smartPower) {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x140E0828L, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x0E082848L, LEN_TX_POWER);
						#endif
					} else {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x28282828L, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x48484848L, LEN_TX_POWER);
						#endif
					}
				} else if(_pulseFrequency == PulseFrequency::FREQ_64MHZ) {
					if(_smartPower) {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x05254565L, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x25456585L, LEN_TX_POWER);
						#endif
					} else {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x65656565L, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x85858585L, LEN_TX_POWER);
						#endif
					}
				} else {
					// TODO proper error/warning handling
				}
			} else if(_channel == Channel::CHANNEL_7) {
				if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
					if(_smartPower) {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x12325272L, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x32527292L, LEN_TX_POWER);
						#endif
					} else {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x72727272L, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x92929292L, LEN_TX_POWER);
						#endif
					}
				} else if(_pulseFrequency == PulseFrequency::FREQ_64MHZ) {
					if(_smartPower) {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0x315191B1L, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0x5171B1D1L, LEN_TX_POWER);
						#endif
					} else {
						#if DWM1000_OPTIMIZED
						DW1000NgUtils::writeValueToBytes(txpower, 0xB1B1B1B1L, LEN_TX_POWER);
						#else
						DW1000NgUtils::writeValueToBytes(txpower, 0xD1D1D1D1L, LEN_TX_POWER);
						#endif
					}
				} else {
					// TODO proper error/warning handling
				}
			} else {
				// TODO proper error/warning handling
			}
			_writeBytesToRegister(TX_POWER, NO_SUB, txpower, LEN_TX_POWER);
		}

		/* RF_RXCTRLH - reg:0x28, sub-reg:0x0B, table 37 */
		void _rfrxctrlh() {
			byte rfrxctrlh[LEN_RF_RXCTRLH];
			if(_channel != Channel::CHANNEL_4 && _channel != Channel::CHANNEL_7) {
				DW1000NgUtils::writeValueToBytes(rfrxctrlh, 0xD8, LEN_RF_RXCTRLH);
			} else {
				DW1000NgUtils::writeValueToBytes(rfrxctrlh, 0xBC, LEN_RF_RXCTRLH);
			}
			_writeBytesToRegister(RF_CONF, RF_RXCTRLH_SUB, rfrxctrlh, LEN_RF_RXCTRLH);
		}

		/* RX_TXCTRL - reg:0x28, sub-reg:0x0C */
		void _rftxctrl() {
			byte rftxctrl[LEN_RF_TXCTRL];
			if(_channel == Channel::CHANNEL_1) {
				DW1000NgUtils::writeValueToBytes(rftxctrl, 0x00005C40L, LEN_RF_TXCTRL);
			} else if(_channel == Channel::CHANNEL_2) {
				DW1000NgUtils::writeValueToBytes(rftxctrl, 0x00045CA0L, LEN_RF_TXCTRL);
			} else if(_channel == Channel::CHANNEL_3) {
				DW1000NgUtils::writeValueToBytes(rftxctrl, 0x00086CC0L, LEN_RF_TXCTRL);
			} else if(_channel == Channel::CHANNEL_4) {
				DW1000NgUtils::writeValueToBytes(rftxctrl, 0x00045C80L, LEN_RF_TXCTRL);
			} else if(_channel == Channel::CHANNEL_5) {
				DW1000NgUtils::writeValueToBytes(rftxctrl, 0x001E3FE0L, LEN_RF_TXCTRL);
			} else if(_channel == Channel::CHANNEL_7) {
				DW1000NgUtils::writeValueToBytes(rftxctrl, 0x001E7DE0L, LEN_RF_TXCTRL);
			} else {
				// TODO proper error/warning handling
			}
			_writeBytesToRegister(RF_CONF, RF_TXCTRL_SUB, rftxctrl, LEN_RF_TXCTRL);
		}

		/* TC_PGDELAY - reg:0x2A, sub-reg:0x0B, table 40 */
		void _tcpgdelaytune() {
			byte tcpgdelay[LEN_TC_PGDELAY];	
			if(_channel == Channel::CHANNEL_1) {
				DW1000NgUtils::writeValueToBytes(tcpgdelay, 0xC9, LEN_TC_PGDELAY);
			} else if(_channel == Channel::CHANNEL_2) {
				DW1000NgUtils::writeValueToBytes(tcpgdelay, 0xC2, LEN_TC_PGDELAY);
			} else if(_channel == Channel::CHANNEL_3) {
				DW1000NgUtils::writeValueToBytes(tcpgdelay, 0xC5, LEN_TC_PGDELAY);
			} else if(_channel == Channel::CHANNEL_4) {
				DW1000NgUtils::writeValueToBytes(tcpgdelay, 0x95, LEN_TC_PGDELAY);
			} else if(_channel == Channel::CHANNEL_5) {
				DW1000NgUtils::writeValueToBytes(tcpgdelay, 0xC0, LEN_TC_PGDELAY);
			} else if(_channel == Channel::CHANNEL_7) {
				DW1000NgUtils::writeValueToBytes(tcpgdelay, 0x93, LEN_TC_PGDELAY);
			} else {
				// TODO proper error/warning handling
			}
			_writeBytesToRegister(TX_CAL, TC_PGDELAY_SUB, tcpgdelay, LEN_TC_PGDELAY);
		}

		// FS_PLLCFG and FS_PLLTUNE - reg:0x2B, sub-reg:0x07-0x0B, tables 43-44
		void _fspll() {
			byte fspllcfg[LEN_FS_PLLCFG];
			byte fsplltune[LEN_FS_PLLTUNE];
			if(_channel == Channel::CHANNEL_1) {
				DW1000NgUtils::writeValueToBytes(fspllcfg, 0x09000407L, LEN_FS_PLLCFG);
				DW1000NgUtils::writeValueToBytes(fsplltune, 0x1E, LEN_FS_PLLTUNE);
			} else if(_channel == Channel::CHANNEL_2 || _channel == Channel::CHANNEL_4) {
				DW1000NgUtils::writeValueToBytes(fspllcfg, 0x08400508L, LEN_FS_PLLCFG);
				DW1000NgUtils::writeValueToBytes(fsplltune, 0x26, LEN_FS_PLLTUNE);
			} else if(_channel == Channel::CHANNEL_3) {
				DW1000NgUtils::writeValueToBytes(fspllcfg, 0x08401009L, LEN_FS_PLLCFG);
				DW1000NgUtils::writeValueToBytes(fsplltune, 0x56, LEN_FS_PLLTUNE);
			} else if(_channel == Channel::CHANNEL_5 || _channel == Channel::CHANNEL_7) {
				DW1000NgUtils::writeValueToBytes(fspllcfg, 0x0800041DL, LEN_FS_PLLCFG);
				DW1000NgUtils::writeValueToBytes(fsplltune, 0xBE, LEN_FS_PLLTUNE);
			} else {
				// TODO proper error/warning handling
			}
			_writeBytesToRegister(FS_CTRL, FS_PLLTUNE_SUB, fsplltune, LEN_FS_PLLTUNE);
			_writeBytesToRegister(FS_CTRL, FS_PLLCFG_SUB, fspllcfg, LEN_FS_PLLCFG);
		}

		/* Crystal calibration from OTP (if available)
		* FS_XTALT - reg:0x2B, sub-reg:0x0E
		* OTP(one-time-programmable) memory map - table 10 */
		void _fsxtalt() {
			byte fsxtalt[LEN_FS_XTALT];
			byte buf_otp[4];
			_readBytesOTP(0x01E, buf_otp); //0x01E -> byte[0]=XTAL_Trim
			if (buf_otp[0] == 0) {
				// No trim value available from OTP, use midrange value of 0x10
				DW1000NgUtils::writeValueToBytes(fsxtalt, ((0x10 & 0x1F) | 0x60), LEN_FS_XTALT);
			} else {
				DW1000NgUtils::writeValueToBytes(fsxtalt, ((buf_otp[0] & 0x1F) | 0x60), LEN_FS_XTALT);
			}
			// write configuration back to chip
			_writeBytesToRegister(FS_CTRL, FS_XTALT_SUB, fsxtalt, LEN_FS_XTALT);
		}

		void _tune() {
			// these registers are going to be tuned/configured
			_agctune1();
			_agctune2();
			_agctune3();
			_drxtune0b();
			_drxtune1a();
			_drxtune1b();
			_drxtune2();
			_drxtune4H();
			_ldecfg1();
			_ldecfg2();
			_lderepc(); 
			if(_autoTXPower) _txpowertune();
			_rfrxctrlh();
			_rftxctrl();
			if(_autoTCPGDelay) _tcpgdelaytune();
			_fspll();
			_fsxtalt();
		}

		void _writeNetworkIdAndDeviceAddress() {
			_writeBytesToRegister(PANADR, NO_SUB, _networkAndAddress, LEN_PANADR);
		}

		void _writeSystemConfigurationRegister() {
			_writeBytesToRegister(SYS_CFG, NO_SUB, _syscfg, LEN_SYS_CFG);
		}

		void _writeChannelControlRegister() {
			_writeBytesToRegister(CHAN_CTRL, NO_SUB, _chanctrl, LEN_CHAN_CTRL);
		}

		void _writeTransmitFrameControlRegister() {
			_writeBytesToRegister(TX_FCTRL, NO_SUB, _txfctrl, LEN_TX_FCTRL);
		}

		void _writeSystemEventMaskRegister() {
			_writeBytesToRegister(SYS_MASK, NO_SUB, _sysmask, LEN_SYS_MASK);
		}

		void _writeAntennaDelayRegisters() {
			byte antennaTxDelayBytes[2];
			byte antennaRxDelayBytes[2];
			DW1000NgUtils::writeValueToBytes(antennaTxDelayBytes, _antennaTxDelay, LEN_TX_ANTD);
			DW1000NgUtils::writeValueToBytes(antennaRxDelayBytes, _antennaRxDelay, LEN_LDE_RXANTD);
			_writeBytesToRegister(TX_ANTD, NO_SUB, antennaTxDelayBytes, LEN_TX_ANTD);
			_writeBytesToRegister(LDE_IF, LDE_RXANTD_SUB, antennaRxDelayBytes, LEN_LDE_RXANTD);
		}

		void _writeConfiguration() {
			// write all configurations back to device
			_writeSystemConfigurationRegister();
			_writeChannelControlRegister();
			_writeTransmitFrameControlRegister();
		}

		void _setFrameFilter(boolean val) {
			DW1000NgUtils::setBit(_syscfg, LEN_SYS_CFG, FFEN_BIT, val);
		}

		void _useExtendedFrameLength(boolean val) {
			_extendedFrameLength = (val ? FRAME_LENGTH_EXTENDED : FRAME_LENGTH_NORMAL);
			_syscfg[2] &= 0xFC;
			_syscfg[2] |= _extendedFrameLength;
		}

		void _setReceiverAutoReenable(boolean val) {
			DW1000NgUtils::setBit(_syscfg, LEN_SYS_CFG, RXAUTR_BIT, val);
		}

		void _useFrameCheck(boolean val) {
			_frameCheck = val;
		}

		void _setNlosOptimization(boolean val) {
			_nlos = val;
			if(_nlos) {
				_ldecfg1();
				_ldecfg2();
			}
		}

		void _useSmartPower(boolean smartPower) {
			_smartPower = smartPower;
			DW1000NgUtils::setBit(_syscfg, LEN_SYS_CFG, DIS_STXP_BIT, !smartPower);
			_writeSystemConfigurationRegister();
			if(_autoTXPower)
				_txpowertune();
		}

		void _setSFDMode(SFDMode mode) {
			switch(mode) {
				case SFDMode::STANDARD_SFD:
					DW1000NgUtils::setBit(_chanctrl, LEN_CHAN_CTRL, DWSFD_BIT, false);
					DW1000NgUtils::setBit(_chanctrl, LEN_CHAN_CTRL, TNSSFD_BIT, false);
					DW1000NgUtils::setBit(_chanctrl, LEN_CHAN_CTRL, RNSSFD_BIT, false);
					_standardSFD = true;
					break;
				case SFDMode::DECAWAVE_SFD:
					DW1000NgUtils::setBit(_chanctrl, LEN_CHAN_CTRL, DWSFD_BIT, true);
					DW1000NgUtils::setBit(_chanctrl, LEN_CHAN_CTRL, TNSSFD_BIT, true);
					DW1000NgUtils::setBit(_chanctrl, LEN_CHAN_CTRL, RNSSFD_BIT, true);
					_standardSFD = false;
					break;
				default:
					return; //TODO Proper error handling
			}
		}

		void _setChannel(Channel channel) {
			byte chan = static_cast<byte>(channel);
			chan &= 0xF;
			_chanctrl[0] = ((chan | (chan << 4)) & 0xFF);

			_channel = channel;
		}

		void _setDataRate(DataRate data_rate) {
			byte rate = static_cast<byte>(data_rate);
			rate &= 0x03;
			_txfctrl[1] &= 0x83;
			_txfctrl[1] |= (byte)((rate << 5) & 0xFF);
			// special 110kbps flag
			if(data_rate == DataRate::RATE_110KBPS) {
				DW1000NgUtils::setBit(_syscfg, LEN_SYS_CFG, RXM110K_BIT, true);
			} else {
				DW1000NgUtils::setBit(_syscfg, LEN_SYS_CFG, RXM110K_BIT, false);
			}
			_dataRate = data_rate;
		}

		void _setPulseFrequency(PulseFrequency frequency) {
			byte freq = static_cast<byte>(frequency);
			freq &= 0x03;
			_txfctrl[2] &= 0xFC;
			_txfctrl[2] |= (byte)(freq & 0xFF);
			_chanctrl[2] &= 0xF3;
			_chanctrl[2] |= (byte)((freq << 2) & 0xFF);

			_pulseFrequency = frequency;
		}

		void _setPreambleLength(PreambleLength preamble_length) {
			byte prealen = static_cast<byte>(preamble_length);
			prealen &= 0x0F;
			_txfctrl[2] &= 0xC3;
			_txfctrl[2] |= (byte)((prealen << 2) & 0xFF);
			
			switch(preamble_length) {
				case PreambleLength::LEN_64:
					_pacSize = PacSize::SIZE_8;
					break;
				case PreambleLength::LEN_128:
					_pacSize = PacSize::SIZE_8;
					break;
				case PreambleLength::LEN_256:
					_pacSize = PacSize::SIZE_16;
					break;
				case PreambleLength::LEN_512:
					_pacSize = PacSize::SIZE_16;
					break;
				case PreambleLength::LEN_1024:
					_pacSize = PacSize::SIZE_32;
					break;
				default:
					_pacSize = PacSize::SIZE_64; // In case of 1536, 2048 or 4096 preamble length.
			}
			
			_preambleLength = preamble_length;
		}

		void _setPreambleCode(PreambleCode preamble_code) {
			byte preacode = static_cast<byte>(preamble_code);
			preacode &= 0x1F;
			_chanctrl[2] &= 0x3F;
			_chanctrl[2] |= ((preacode << 6) & 0xFF);
			_chanctrl[3] = 0x00;
			_chanctrl[3] = ((((preacode >> 2) & 0x07) | (preacode << 3)) & 0xFF);

			_preambleCode = preamble_code;
		}

		boolean _checkPreambleCodeValidity() {
			byte preacode = static_cast<byte>(_preambleCode);
			if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
				for (auto i = 0; i < 2; i++) {
					if(preacode == preamble_validity_matrix_PRF16[(int) _channel][i])
						return true;
				}
				return false;
			} else if (_pulseFrequency == PulseFrequency::FREQ_64MHZ) {
				for(auto i = 0; i < 4; i++) {
					if(preacode == preamble_validity_matrix_PRF64[(int) _channel][i])
						return true;
				}
				return false;
			} else {
				return false; //TODO Proper error handling
			}
		}

		void _setValidPreambleCode() {
			PreambleCode preamble_code;

			switch(_channel) {
				case Channel::CHANNEL_1:
					preamble_code = _pulseFrequency == PulseFrequency::FREQ_16MHZ ? PreambleCode::CODE_2 : PreambleCode::CODE_10;
					break;
				case Channel::CHANNEL_3:
					preamble_code = _pulseFrequency == PulseFrequency::FREQ_16MHZ ? PreambleCode::CODE_6 : PreambleCode::CODE_10;
					break;
				case Channel::CHANNEL_4:
				case Channel::CHANNEL_7:
					preamble_code = _pulseFrequency == PulseFrequency::FREQ_16MHZ ? PreambleCode::CODE_8 : PreambleCode::CODE_18;
					break;
				case Channel::CHANNEL_2:
				case Channel::CHANNEL_5:
					preamble_code = _pulseFrequency == PulseFrequency::FREQ_16MHZ ? PreambleCode::CODE_3 : PreambleCode::CODE_10;
					break;
				default:
					return; //TODO Proper Error Handling
			}
			byte preacode = static_cast<byte>(preamble_code);
			preacode &= 0x1F;
			_chanctrl[2] &= 0x3F;
			_chanctrl[2] |= ((preacode << 6) & 0xFF);
			_chanctrl[3] = 0x00;
			_chanctrl[3] = ((((preacode >> 2) & 0x07) | (preacode << 3)) & 0xFF);

			_preambleCode = preamble_code;
		}

		void _setNonStandardSFDLength() {
			switch(_dataRate) {
				case DataRate::RATE_6800KBPS:
					_writeByte(USR_SFD, SFD_LENGTH_SUB, 0x08);
					break;
				case DataRate::RATE_850KBPS:
					_writeByte(USR_SFD, SFD_LENGTH_SUB, 0x10);
					break;
				case DataRate::RATE_110KBPS:
					_writeByte(USR_SFD, SFD_LENGTH_SUB, 0x40);
					break;
				default:
					return; //TODO Proper error handling
			}
		}

		void _interruptOnSent(boolean val) {
			DW1000NgUtils::setBit(_sysmask, LEN_SYS_MASK, TXFRS_BIT, val);
		}

		void _interruptOnReceived(boolean val) {
			DW1000NgUtils::setBit(_sysmask, LEN_SYS_MASK, RXDFR_BIT, val);
			DW1000NgUtils::setBit(_sysmask, LEN_SYS_MASK, RXFCG_BIT, val);
		}

		void _interruptOnReceiveFailed(boolean val) {
			DW1000NgUtils::setBit(_sysmask, LEN_SYS_STATUS, LDEERR_BIT, val);
			DW1000NgUtils::setBit(_sysmask, LEN_SYS_STATUS, RXFCE_BIT, val);
			DW1000NgUtils::setBit(_sysmask, LEN_SYS_STATUS, RXPHE_BIT, val);
			DW1000NgUtils::setBit(_sysmask, LEN_SYS_STATUS, RXRFSL_BIT, val);
		}

		void _interruptOnReceiveTimeout(boolean val) {
			DW1000NgUtils::setBit(_sysmask, LEN_SYS_MASK, RXRFTO_BIT, val);
		}

		void _interruptOnReceiveTimestampAvailable(boolean val) {
			DW1000NgUtils::setBit(_sysmask, LEN_SYS_MASK, LDEDONE_BIT, val);
		}

		void _interruptOnAutomaticAcknowledgeTrigger(boolean val) {
			DW1000NgUtils::setBit(_sysmask, LEN_SYS_MASK, AAT_BIT, val);
		}

		void _manageLDE() {
			// transfer any ldo tune values
			byte ldoTune[LEN_OTP_RDAT];
			_readBytesOTP(0x04, ldoTune); // TODO #define
			if(ldoTune[0] != 0) {
				// TODO tuning available, copy over to RAM: use OTP_LDO bit
			}
			// tell the chip to load the LDE microcode
			// TODO remove clock-related code (PMSC_CTRL) as handled separately
			byte pmscctrl0[LEN_PMSC_CTRL0];
			byte otpctrl[LEN_OTP_CTRL];
			memset(pmscctrl0, 0, LEN_PMSC_CTRL0);
			memset(otpctrl, 0, LEN_OTP_CTRL);
			_readBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
			_readBytes(OTP_IF, OTP_CTRL_SUB, otpctrl, LEN_OTP_CTRL);
			pmscctrl0[0] = 0x01;
			pmscctrl0[1] = 0x03;
			otpctrl[0]   = 0x00;
			otpctrl[1]   = 0x80;
			_writeBytesToRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, 2);
			_writeBytesToRegister(OTP_IF, OTP_CTRL_SUB, otpctrl, 2);
			delay(5);
			pmscctrl0[0] = 0x00;
			pmscctrl0[1] &= 0x02;
			_writeBytesToRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, 2);
		}

		void _enableClock(byte clock) {
			byte pmscctrl0[LEN_PMSC_CTRL0];
			memset(pmscctrl0, 0, LEN_PMSC_CTRL0);
			_readBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
			/* SYSCLKS */
			if(clock == SYS_AUTO_CLOCK) {
				_currentSPI = &_fastSPI;
				pmscctrl0[0] = SYS_AUTO_CLOCK;
				pmscctrl0[1] &= 0xFE;
			} else if(clock == SYS_XTI_CLOCK) {
				_currentSPI = &_slowSPI;
				pmscctrl0[0] &= 0xFC;
				pmscctrl0[0] |= SYS_XTI_CLOCK;
			} else if(clock == SYS_PLL_CLOCK) {
				_currentSPI = &_fastSPI;
				pmscctrl0[0] &= 0xFC;
				pmscctrl0[0] |= SYS_PLL_CLOCK;
			} else if (clock == TX_PLL_CLOCK) { /* NOT SYSCLKS but TX */
				_currentSPI = &_fastSPI;
				pmscctrl0[0] &= 0xCF;
				pmscctrl0[0] |= TX_PLL_CLOCK;
			} else {
				// TODO deliver proper warning
			}
			_writeBytesToRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, 2);
		}

		void _clearReceiveStatus() {
			// clear latched RX bits (i.e. write 1 to clear)
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, RXDFR_BIT, true);
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, RXFCG_BIT, true);
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, RXPRD_BIT, true);
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, RXSFDD_BIT, true);
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, RXPHD_BIT, true);
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, LDEDONE_BIT, true);
			_writeBytesToRegister(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
		}

		void _clearReceiveTimestampAvailableStatus() {
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, LDEDONE_BIT, true);
			_writeBytesToRegister(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
		}

		void _clearReceiveTimeoutStatus() {
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, RXRFTO_BIT, true);
			_writeBytesToRegister(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
		}

		void _clearReceiveFailedStatus() {
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, RXPHE_BIT, true);
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, RXFCE_BIT, true);
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, RXRFSL_BIT, true);
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, RXSFDTO_BIT, true);
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, AFFREJ_BIT, true);
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, LDEERR_BIT, true);
			_writeBytesToRegister(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
		}

		void _clearTransmitStatus() {
			// clear latched TX bits
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, AAT_BIT, true);
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, TXFRB_BIT, true);
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, TXPRS_BIT, true);
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, TXPHS_BIT, true);
			DW1000NgUtils::setBit(_sysstatus, LEN_SYS_STATUS, TXFRS_BIT, true);
			_writeBytesToRegister(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
		}

		void _resetReceiver() {
			byte pmscctrl0[LEN_PMSC_CTRL0];
			_readBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
			pmscctrl0[3] = 0xE0;
			_writeBytesToRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
			pmscctrl0[3] = 0xF0;
			_writeBytesToRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
		}

		/* Internal helpers to read configuration */

		void _readSystemConfigurationRegister() {
			_readBytes(SYS_CFG, NO_SUB, _syscfg, LEN_SYS_CFG);
		}

		void _readSystemEventStatusRegister() {
			_readBytes(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
		}

		void _readNetworkIdAndDeviceAddress() {
			_readBytes(PANADR, NO_SUB, _networkAndAddress, LEN_PANADR);
		}

		void _readSystemEventMaskRegister() {
			_readBytes(SYS_MASK, NO_SUB, _sysmask, LEN_SYS_MASK);
		}

		void _readChannelControlRegister() {
			_readBytes(CHAN_CTRL, NO_SUB, _chanctrl, LEN_CHAN_CTRL);
		}

		void _readTransmitFrameControlRegister() {
			_readBytes(TX_FCTRL, NO_SUB, _txfctrl, LEN_TX_FCTRL);
		}

		boolean _isTransmitDone() {
			return DW1000NgUtils::getBit(_sysstatus, LEN_SYS_STATUS, TXFRS_BIT);
		}

		boolean _isReceiveTimestampAvailable() {
			return DW1000NgUtils::getBit(_sysstatus, LEN_SYS_STATUS, LDEDONE_BIT);
		}

		boolean _isReceiveDone() {
			if(_frameCheck) {
				return DW1000NgUtils::getBit(_sysstatus, LEN_SYS_STATUS, RXFCG_BIT);
			}
			return DW1000NgUtils::getBit(_sysstatus, LEN_SYS_STATUS, RXDFR_BIT);
		}

		boolean _isReceiveFailed() {
			boolean ldeErr, rxCRCErr, rxHeaderErr, rxDecodeErr;
			ldeErr      = DW1000NgUtils::getBit(_sysstatus, LEN_SYS_STATUS, LDEERR_BIT);
			rxCRCErr    = DW1000NgUtils::getBit(_sysstatus, LEN_SYS_STATUS, RXFCE_BIT);
			rxHeaderErr = DW1000NgUtils::getBit(_sysstatus, LEN_SYS_STATUS, RXPHE_BIT);
			rxDecodeErr = DW1000NgUtils::getBit(_sysstatus, LEN_SYS_STATUS, RXRFSL_BIT);
			if(ldeErr || rxCRCErr || rxHeaderErr || rxDecodeErr) {
				return true;
			}
			return false;
		}

		//Checks to see any of the three timeout bits in sysstatus are high (RXRFTO (Frame Wait timeout), RXPTO (Preamble timeout), RXSFDTO (Start frame delimiter(?) timeout).
		boolean _isReceiveTimeout() {
			return (DW1000NgUtils::getBit(_sysstatus, LEN_SYS_STATUS, RXRFTO_BIT) | DW1000NgUtils::getBit(_sysstatus, LEN_SYS_STATUS, RXPTO_BIT) | DW1000NgUtils::getBit(_sysstatus, LEN_SYS_STATUS, RXSFDTO_BIT));
		}

		boolean _isClockProblem() {
			boolean clkllErr, rfllErr;
			clkllErr = DW1000NgUtils::getBit(_sysstatus, LEN_SYS_STATUS, CLKPLL_LL_BIT);
			rfllErr  = DW1000NgUtils::getBit(_sysstatus, LEN_SYS_STATUS, RFPLL_LL_BIT);
			if(clkllErr || rfllErr) {
				return true;
			}
			return false;
		}

		void _disableSequencing() {
            _enableClock(SYS_XTI_CLOCK);
            byte zero[2];
            DW1000NgUtils::writeValueToBytes(zero, 0x0000, 2);
            _writeBytesToRegister(PMSC, PMSC_CTRL1_SUB, zero, 2); // To re-enable write 0xE7
        }

		// TODO check function, different type violations between byte and int
		void _correctTimestamp(DW1000NgTime& timestamp) {
			// base line dBm, which is -61, 2 dBm steps, total 18 data points (down to -95 dBm)
			float rxPowerBase     = -(getReceivePower()+61.0f)*0.5f;
			int16_t   rxPowerBaseLow  = (int16_t)rxPowerBase; // TODO check type
			int16_t   rxPowerBaseHigh = rxPowerBaseLow+1; // TODO check type
			if(rxPowerBaseLow <= 0) {
				rxPowerBaseLow  = 0;
				rxPowerBaseHigh = 0;
			} else if(rxPowerBaseHigh >= 17) {
				rxPowerBaseLow  = 17;
				rxPowerBaseHigh = 17;
			}
			// select range low/high values from corresponding table
			int16_t rangeBiasHigh;
			int16_t rangeBiasLow;
			if(_channel == Channel::CHANNEL_4 || _channel == Channel::CHANNEL_7) {
				// 900 MHz receiver bandwidth
				if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
					rangeBiasHigh = (rxPowerBaseHigh < BIAS_900_16_ZERO ? -BIAS_900_16[rxPowerBaseHigh] : BIAS_900_16[rxPowerBaseHigh]);
					rangeBiasHigh <<= 1;
					rangeBiasLow  = (rxPowerBaseLow < BIAS_900_16_ZERO ? -BIAS_900_16[rxPowerBaseLow] : BIAS_900_16[rxPowerBaseLow]);
					rangeBiasLow <<= 1;
				} else if(_pulseFrequency == PulseFrequency::FREQ_64MHZ) {
					rangeBiasHigh = (rxPowerBaseHigh < BIAS_900_64_ZERO ? -BIAS_900_64[rxPowerBaseHigh] : BIAS_900_64[rxPowerBaseHigh]);
					rangeBiasHigh <<= 1;
					rangeBiasLow  = (rxPowerBaseLow < BIAS_900_64_ZERO ? -BIAS_900_64[rxPowerBaseLow] : BIAS_900_64[rxPowerBaseLow]);
					rangeBiasLow <<= 1;
				} else {
					// TODO proper error handling
					return;
				}
			} else {
				// 500 MHz receiver bandwidth
				if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
					rangeBiasHigh = (rxPowerBaseHigh < BIAS_500_16_ZERO ? -BIAS_500_16[rxPowerBaseHigh] : BIAS_500_16[rxPowerBaseHigh]);
					rangeBiasLow  = (rxPowerBaseLow < BIAS_500_16_ZERO ? -BIAS_500_16[rxPowerBaseLow] : BIAS_500_16[rxPowerBaseLow]);
				} else if(_pulseFrequency == PulseFrequency::FREQ_64MHZ) {
					rangeBiasHigh = (rxPowerBaseHigh < BIAS_500_64_ZERO ? -BIAS_500_64[rxPowerBaseHigh] : BIAS_500_64[rxPowerBaseHigh]);
					rangeBiasLow  = (rxPowerBaseLow < BIAS_500_64_ZERO ? -BIAS_500_64[rxPowerBaseLow] : BIAS_500_64[rxPowerBaseLow]);
				} else {
					// TODO proper error handling
					return;
				}
			}
			// linear interpolation of bias values
			float      rangeBias = rangeBiasLow+(rxPowerBase-rxPowerBaseLow)*(rangeBiasHigh-rangeBiasLow);
			// range bias [mm] to timestamp modification value conversion
			DW1000NgTime adjustmentTime;
			adjustmentTime.setTimestamp((int16_t)(rangeBias*DW1000NgTime::DISTANCE_OF_RADIO_INV*0.001f));
			// apply correction
			timestamp -= adjustmentTime;
		}

        void _configureRFTransmitPowerSpectrumTestMode() {
			/* Enabled TXFEN, PLLFEN, LDOFEN and set TXRXSW to TX */
            byte enable_mask[4];
            DW1000NgUtils::writeValueToBytes(enable_mask, 0x005FFF00, LEN_RX_CONF_SUB);
            _writeBytesToRegister(RF_CONF, RF_CONF_SUB, enable_mask, LEN_RX_CONF_SUB);
        }
	}

	/* ####################### PUBLIC ###################### */

	void begin(uint8_t ss, uint8_t irq, uint8_t rst) {
		// generous initial init/wake-up-idle delay
		delay(5);
		_ss = ss;
		_irq = irq;
		// start SPI
		SPI.begin();
		// pin and basic member setup
		// attach interrupt
		// TODO throw error if pin is not a interrupt pin
		attachInterrupt(digitalPinToInterrupt(_irq), pollForEvents, RISING);
		select();
		// try locking clock at PLL speed (should be done already,
		// but just to be sure)
		_enableClock(SYS_AUTO_CLOCK);
		delay(5);
		// reset chip (either soft or hard)
		if(rst != 0xff) {
			_rst = rst;
			// DW1000Ng data sheet v2.08 §5.6.1 page 20, the RSTn pin should not be driven high but left floating.
			pinMode(_rst, INPUT);
		}
		reset();
		
		_enableClock(SYS_XTI_CLOCK);
		delay(5);
		_manageLDE();
		delay(5);
		_enableClock(SYS_AUTO_CLOCK);
		delay(5);

		_readNetworkIdAndDeviceAddress();
		_readSystemConfigurationRegister();
		_readChannelControlRegister();
		_readTransmitFrameControlRegister();
		_readSystemEventMaskRegister();
		
		/* Cleared AON:CFG1(0x2C:0x0A) for proper operation of deepSleep */
		_writeToRegister(AON, AON_CFG1_SUB, 0x00, LEN_AON_CFG1);

		// read the temp and vbat readings from OTP that were recorded during production test
		// see 6.3.1 OTP memory map
		byte buf_otp[4];
		_readBytesOTP(0x008, buf_otp); // the stored 3.3 V reading
		_vmeas3v3 = buf_otp[0];
		_readBytesOTP(0x009, buf_otp); // the stored 23C reading
		_tmeas23C = buf_otp[0];
	}

	void select() {
		SPI.usingInterrupt(digitalPinToInterrupt(_irq));
		pinMode(_ss, OUTPUT);
		digitalWrite(_ss, HIGH);
	}

	void end() {
		SPI.end();
	}

	/* callback handler management. */
	void attachErrorHandler(void (* handleError)(void)) {
		_handleError = handleError;
	}
	
	void attachSentHandler(void (* handleSent)(void)) {
		_handleSent = handleSent;
	}
	
	void attachReceivedHandler(void (* handleReceived)(void)) {
		_handleReceived = handleReceived;
	}
	
	void attachReceiveFailedHandler(void (* handleReceiveFailed)(void)) {
		_handleReceiveFailed = handleReceiveFailed;
	}
	
	void attachReceiveTimeoutHandler(void (* handleReceiveTimeout)(void)) {
		_handleReceiveTimeout = handleReceiveTimeout;
	}
	
	void attachReceiveTimestampAvailableHandler(void (* handleReceiveTimestampAvailable)(void)) {
		_handleReceiveTimestampAvailable = handleReceiveTimestampAvailable;
	}

	void pollForEvents() {
		// read current status and handle via callbacks
		_readSystemEventStatusRegister();
		if(_isClockProblem() /* TODO and others */ && _handleError != 0) {
			(*_handleError)();
		}
		if(_isTransmitDone()) {
			_clearTransmitStatus();
			if(_handleSent != nullptr)
				(*_handleSent)();
		}
		if(_isReceiveTimestampAvailable()) {
			_clearReceiveTimestampAvailableStatus();
			if(_handleReceiveTimestampAvailable != nullptr)
				(*_handleReceiveTimestampAvailable)();
		}
		if(_isReceiveFailed()) {
			_clearReceiveFailedStatus();
			forceTRxOff();
			_resetReceiver();
			if(_handleReceiveFailed != nullptr)
				(*_handleReceiveFailed)();
		} else if(_isReceiveTimeout()) {
			_clearReceiveTimeoutStatus();
			forceTRxOff();
			_resetReceiver();
			if(_handleReceiveTimeout != nullptr)
				(*_handleReceiveTimeout)();
		} else if(_isReceiveDone()) {
			_clearReceiveStatus();
			if(_handleReceived != nullptr)
				(*_handleReceived)();
		}
	}

	void enableDebounceClock() {
		byte pmscctrl0[LEN_PMSC_CTRL0];
		memset(pmscctrl0, 0, LEN_PMSC_CTRL0);
		_readBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
		DW1000NgUtils::setBit(pmscctrl0, LEN_PMSC_CTRL0, GPDCE_BIT, 1);
		DW1000NgUtils::setBit(pmscctrl0, LEN_PMSC_CTRL0, KHZCLKEN_BIT, 1);
		_writeBytesToRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
		_debounceClockEnabled = true;
	}

	void enableLedBlinking() {
		byte pmscledc[LEN_PMSC_LEDC];
		memset(pmscledc, 0, LEN_PMSC_LEDC);
		_readBytes(PMSC, PMSC_LEDC_SUB, pmscledc, LEN_PMSC_LEDC);
		DW1000NgUtils::setBit(pmscledc, LEN_PMSC_LEDC, BLNKEN, 1);
		_writeBytesToRegister(PMSC, PMSC_LEDC_SUB, pmscledc, LEN_PMSC_LEDC);
	}

	void setGPIOMode(uint8_t msgp, uint8_t mode) {
		byte gpiomode[LEN_GPIO_MODE];
		memset(gpiomode, 0, LEN_GPIO_MODE);
		_readBytes(GPIO_CTRL, GPIO_MODE_SUB, gpiomode, LEN_GPIO_MODE);
		for (char i = 0; i < 2; i++){
			DW1000NgUtils::setBit(gpiomode, LEN_GPIO_MODE, msgp + i, (mode >> i) & 1);
		}
		_writeBytesToRegister(GPIO_CTRL, GPIO_MODE_SUB, gpiomode, LEN_GPIO_MODE);
	}

	void deepSleep() {
		byte aon_wcfg[LEN_AON_WCFG];
		memset(aon_wcfg, 0, LEN_AON_WCFG);
		_readBytes(AON, AON_WCFG_SUB, aon_wcfg, LEN_AON_WCFG);
		DW1000NgUtils::setBit(aon_wcfg, LEN_AON_WCFG, ONW_LDC_BIT, true);
		DW1000NgUtils::setBit(aon_wcfg, LEN_AON_WCFG, ONW_PRES_SLEEP_BIT, false);
		DW1000NgUtils::setBit(aon_wcfg, LEN_AON_WCFG, ONW_LLDE_BIT, true);
		DW1000NgUtils::setBit(aon_wcfg, LEN_AON_WCFG, ONW_LDD0_BIT, true);
		_writeBytesToRegister(AON, AON_WCFG_SUB, aon_wcfg, LEN_AON_WCFG);

		byte aon_cfg0[LEN_AON_CFG0];
		memset(aon_cfg0, 0, LEN_AON_CFG0);
		_readBytes(AON, AON_CFG0_SUB, aon_cfg0, LEN_AON_CFG0);
		DW1000NgUtils::setBit(aon_cfg0, LEN_AON_CFG0, WAKE_PIN_BIT, true);
		DW1000NgUtils::setBit(aon_cfg0, LEN_AON_CFG0, WAKE_SPI_BIT, true);
		DW1000NgUtils::setBit(aon_cfg0, LEN_AON_CFG0, WAKE_CNT_BIT, false);
		DW1000NgUtils::setBit(aon_cfg0, LEN_AON_CFG0, SLEEP_EN_BIT, true);
		_writeBytesToRegister(AON, AON_CFG0_SUB, aon_cfg0, LEN_AON_CFG0);

		byte aon_ctrl[LEN_AON_CTRL];
		memset(aon_ctrl, 0, LEN_AON_CTRL);
		_readBytes(AON, AON_CTRL_SUB, aon_ctrl, LEN_AON_CTRL);
		//DW1000NgUtils::setBit(aon_ctrl, LEN_AON_CTRL, UPL_CFG_BIT, true);
		DW1000NgUtils::setBit(aon_ctrl, LEN_AON_CTRL, SAVE_BIT, true);
		_writeBytesToRegister(AON, AON_CTRL_SUB, aon_ctrl, LEN_AON_CTRL);
	}

	void spiWakeup(){
		byte deviceId[LEN_DEV_ID];
		byte expectedDeviceId[LEN_DEV_ID];
		DW1000NgUtils::writeValueToBytes(expectedDeviceId, 0xDECA0130, LEN_DEV_ID);
		_readBytes(DEV_ID, NO_SUB, deviceId, LEN_DEV_ID);
		if (memcmp(deviceId, expectedDeviceId, LEN_DEV_ID)) {
			digitalWrite(_ss, LOW);
			delay(1);
			digitalWrite(_ss, HIGH);
			delay(5);
			setTxAntennaDelay(_antennaTxDelay);
			if (_debounceClockEnabled){
					enableDebounceClock();
			}
		}
	}

	void reset() {
		if(_rst == 0xff) {
			softReset();
		} else {
			// DW1000Ng data sheet v2.08 §5.6.1 page 20, the RSTn pin should not be driven high but left floating.
			pinMode(_rst, OUTPUT);
			digitalWrite(_rst, LOW);
			delay(2);  // DW1000Ng data sheet v2.08 §5.6.1 page 20: nominal 50ns, to be safe take more time
			pinMode(_rst, INPUT);
			delay(10); // dw1000Ng data sheet v1.2 page 5: nominal 3 ms, to be safe take more time
			// force into idle mode (although it should be already after reset)
			forceTRxOff();
		}
	}

	void softReset() {
		byte pmscctrl0[LEN_PMSC_CTRL0];
		_readBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
		pmscctrl0[0] = 0x01;
		_writeBytesToRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
		pmscctrl0[3] = 0x00;
		_writeBytesToRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
		delay(10);
		pmscctrl0[0] = 0x00;
		pmscctrl0[3] = 0xF0;
		_writeBytesToRegister(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
		// force into idle mode
		forceTRxOff();
	}

	/* ###########################################################################
	* #### Pretty printed device information ####################################
	* ######################################################################### */


	void getPrintableDeviceIdentifier(char msgBuffer[]) {
		byte data[LEN_DEV_ID];
		_readBytes(DEV_ID, NO_SUB, data, LEN_DEV_ID);
		sprintf(msgBuffer, "%02X - model: %d, version: %d, revision: %d",
						(uint16_t)((data[3] << 8) | data[2]), data[1], (data[0] >> 4) & 0x0F, data[0] & 0x0F);
	}

	void getPrintableExtendedUniqueIdentifier(char msgBuffer[]) {
		byte data[LEN_EUI];
		_readBytes(EUI, NO_SUB, data, LEN_EUI);
		sprintf(msgBuffer, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
						data[7], data[6], data[5], data[4], data[3], data[2], data[1], data[0]);
	}

	void getPrintableNetworkIdAndShortAddress(char msgBuffer[]) {
		byte data[LEN_PANADR];
		_readBytes(PANADR, NO_SUB, data, LEN_PANADR);
		sprintf(msgBuffer, "PAN: %02X, Short Address: %02X",
						(uint16_t)((data[3] << 8) | data[2]), (uint16_t)((data[1] << 8) | data[0]));
	}

	void getPrintableDeviceMode(char msgBuffer[]) {
		uint16_t dr;
		uint8_t prf;
		uint16_t plen;
		uint8_t pcode;
		uint8_t ch;
		byte chan_ctrl[LEN_CHAN_CTRL];
		byte tx_fctrl[LEN_TX_FCTRL];
		_readBytes(CHAN_CTRL, NO_SUB, chan_ctrl, LEN_CHAN_CTRL);
		_readBytes(TX_FCTRL, NO_SUB, tx_fctrl, LEN_TX_FCTRL);
		/* Data Rate from 0x08 bits:13-14(tx_fctrl) */
		dr = (uint16_t)(tx_fctrl[1] >> 5 & 0x3);
		switch(dr) {
			case 0x00:
				dr = 110;
				break;
			case 0x01:
				dr = 850;
				break;
			case 0x02:
				dr = 6800;
				break;
			default:
				return; //TODO Error handling
		}
		/* PRF(16 or 64) from 0x1F bits:18-19(chan_ctrl) */
		prf = (uint8_t)(chan_ctrl[2] >> 2 & 0x03);
		if(prf == 0x01){
			prf = 16;
		} else if(prf == 0x02){
			prf = 64;
		} else{
			return; //TODO Error handling
		}
		/* PreambleLength from 0x08 bits:18-21(tx_fctrl) */
		plen = (uint16_t)(tx_fctrl[2] >> 2 & 0xF);
		switch(plen) {
			case 0x01:
				plen = 64;
				break;
			case 0x05:
				plen = 128;
				break;
			case 0x09:
				plen = 256;
				break;
			case 0x0D:
				plen = 512;
				break;
			case 0x02:
				plen = 1024;
				break;
			case 0x06:
				plen = 1536;
				break;
			case 0x0A:
				plen = 2048;
				break;
			case 0x03:
				plen = 4096;
				break;
			default:
				return; //TODO Error handling
		}
		/* Channel from 0x1F bits:0-4(tx_chan) */
		ch = (uint8_t)(chan_ctrl[0] & 0xF);
		/* Preamble Code from 0x1F bits:24-31(chan_ctrl) */
		pcode = (uint8_t)(chan_ctrl[3] >> 3 & 0x1F);
		sprintf(msgBuffer, "Data rate: %u kb/s, PRF: %u MHz, Preamble: %u symbols, Channel: #%u, Preamble code #%u" , dr, prf, plen, ch, pcode);
	}

	/* ###########################################################################
	* #### DW1000Ng operation functions ###########################################
	* ######################################################################### */

	void setNetworkId(uint16_t val) {
		_networkAndAddress[2] = (byte)(val & 0xFF);
		_networkAndAddress[3] = (byte)((val >> 8) & 0xFF);
		_writeNetworkIdAndDeviceAddress();
	}

	void setDeviceAddress(uint16_t val) {
		_networkAndAddress[0] = (byte)(val & 0xFF);
		_networkAndAddress[1] = (byte)((val >> 8) & 0xFF);
		_writeNetworkIdAndDeviceAddress();
	}

	void setEUI(char eui[]) {
		byte eui_byte[LEN_EUI];
		DW1000NgUtils::convertToByte(eui, eui_byte);
		setEUI(eui_byte);
	}

	void setEUI(byte eui[]) {
		//we reverse the address->
		byte    reverseEUI[8];
		uint8_t     size = 8;
		for(uint8_t i    = 0; i < size; i++) {
			*(reverseEUI+i) = *(eui+size-i-1);
		}
		_writeBytesToRegister(EUI, NO_SUB, reverseEUI, LEN_EUI);
	}

	void getTemperature(float& temp) {
		_vbatAndTempSteps();
		byte sar_ltemp = 0; _readBytes(TX_CAL, 0x04, &sar_ltemp, 1);
		temp = (sar_ltemp - _tmeas23C) * 1.14f + 23.0f;
	}

	void getBatteryVoltage(float& vbat) {
		_vbatAndTempSteps();
		byte sar_lvbat = 0; _readBytes(TX_CAL, 0x03, &sar_lvbat, 1);
		vbat = (sar_lvbat - _vmeas3v3) / 173.0f + 3.3f;
	}

	void getTemperatureAndBatteryVoltage(float& temp, float& vbat) {
		// follow the procedure from section 6.4 of the User Manual
		_vbatAndTempSteps();
		byte sar_lvbat = 0; _readBytes(TX_CAL, 0x03, &sar_lvbat, 1);
		byte sar_ltemp = 0; _readBytes(TX_CAL, 0x04, &sar_ltemp, 1);
		
		// calculate voltage and temperature
		vbat = (sar_lvbat - _vmeas3v3) / 173.0f + 3.3f;
		temp = (sar_ltemp - _tmeas23C) * 1.14f + 23.0f;
	}

	void setFrameFilterBehaveCoordinator(boolean val) {
		DW1000NgUtils::setBit(_syscfg, LEN_SYS_CFG, FFBC_BIT, val);
	}

	void setFrameFilterAllowBeacon(boolean val) {
		DW1000NgUtils::setBit(_syscfg, LEN_SYS_CFG, FFAB_BIT, val);
	}

	void setFrameFilterAllowData(boolean val) {
		DW1000NgUtils::setBit(_syscfg, LEN_SYS_CFG, FFAD_BIT, val);
	}

	void setFrameFilterAllowAcknowledgement(boolean val) {
		DW1000NgUtils::setBit(_syscfg, LEN_SYS_CFG, FFAA_BIT, val);
	}

	void setFrameFilterAllowMAC(boolean val) {
		DW1000NgUtils::setBit(_syscfg, LEN_SYS_CFG, FFAM_BIT, val);
	}

	void setFrameFilterAllowReserved(boolean val) {
		DW1000NgUtils::setBit(_syscfg, LEN_SYS_CFG, FFAR_BIT, val);
	}


	void setDoubleBuffering(boolean val) {
		DW1000NgUtils::setBit(_syscfg, LEN_SYS_CFG, DIS_DRXB_BIT, !val);
	}

	void setAntennaDelay(uint16_t value) {
		_antennaTxDelay = value;
		_antennaRxDelay = value;
		_writeAntennaDelayRegisters();
	}

	void setTxAntennaDelay(uint16_t value) {
		_antennaTxDelay = value;
		_writeAntennaDelayRegisters();	
	}
	void setRxAntennaDelay(uint16_t value) {
		_antennaRxDelay = value;
		_writeAntennaDelayRegisters();
	}

	uint16_t getTxAntennaDelay() {
		return _antennaTxDelay;
	}
	uint16_t getRxAntennaDelay() {
		return _antennaRxDelay;
	}

	void forceTRxOff() {
		memset(_sysctrl, 0, LEN_SYS_CTRL);
		DW1000NgUtils::setBit(_sysctrl, LEN_SYS_CTRL, TRXOFF_BIT, true);
		_writeBytesToRegister(SYS_CTRL, NO_SUB, _sysctrl, LEN_SYS_CTRL);
	}

	void startReceive(ReceiveMode mode) {
		memset(_sysctrl, 0, LEN_SYS_CTRL);
		DW1000NgUtils::setBit(_sysctrl, LEN_SYS_CTRL, SFCST_BIT, !_frameCheck);
		if(mode == ReceiveMode::DELAYED)
			DW1000NgUtils::setBit(_sysctrl, LEN_SYS_CTRL, RXDLYS_BIT, true);
		DW1000NgUtils::setBit(_sysctrl, LEN_SYS_CTRL, RXENAB_BIT, true);
		_writeBytesToRegister(SYS_CTRL, NO_SUB, _sysctrl, LEN_SYS_CTRL);
	}

	void startTransmit(TransmitMode mode) {
		memset(_sysctrl, 0, LEN_SYS_CTRL);
		DW1000NgUtils::setBit(_sysctrl, LEN_SYS_CTRL, SFCST_BIT, !_frameCheck);
		if(mode == TransmitMode::DELAYED)
			DW1000NgUtils::setBit(_sysctrl, LEN_SYS_CTRL, TXDLYS_BIT, true);
		DW1000NgUtils::setBit(_sysctrl, LEN_SYS_CTRL, TXSTRT_BIT, true);
		_writeBytesToRegister(SYS_CTRL, NO_SUB, _sysctrl, LEN_SYS_CTRL);
	}

	void setInterruptPolarity(boolean val) {
		DW1000NgUtils::setBit(_syscfg, LEN_SYS_CFG, HIRQ_POL_BIT, val);
		_writeSystemConfigurationRegister();
	}

	void applyConfiguration(device_configuration_t config) {
		forceTRxOff();

		_setFrameFilter(config.frameFiltering);
		_useExtendedFrameLength(config.extendedFrameLength);
		_setReceiverAutoReenable(config.receiverAutoReenable);
		_useSmartPower(config.smartPower);
		_useFrameCheck(config.frameCheck);
		_setNlosOptimization(config.nlos);
		_setSFDMode(config.sfd);
		_setChannel(config.channel);
		_setDataRate(config.dataRate);
		_setPulseFrequency(config.pulseFreq);
		_setPreambleLength(config.preambleLen);
		_setPreambleCode(config.preaCode);

		if(!_checkPreambleCodeValidity())
			_setValidPreambleCode();

		if(!_standardSFD)
			_setNonStandardSFDLength();

		// writes configuration to registers
		_writeConfiguration();
		// tune according to configuration
		_tune();
	}

	void applyInterruptConfiguration(interrupt_configuration_t interrupt_config) {
		forceTRxOff();

		_interruptOnSent(interrupt_config.interruptOnSent);
		_interruptOnReceived(interrupt_config.interruptOnReceived);
		_interruptOnReceiveFailed(interrupt_config.interruptOnReceiveFailed);
		_interruptOnReceiveTimeout(interrupt_config.interruptOnReceiveTimeout);
		_interruptOnReceiveTimestampAvailable(interrupt_config.interruptOnReceiveTimestampAvailable);
		_interruptOnAutomaticAcknowledgeTrigger(interrupt_config.interruptOnAutomaticAcknowledgeTrigger);

		_writeSystemEventMaskRegister();
	}

	void waitForResponse(boolean val) {
		DW1000NgUtils::setBit(_sysctrl, LEN_SYS_CTRL, WAIT4RESP_BIT, val);
	}

	void setTXPower(byte power[]) {
		//TODO Check byte length
		_writeBytesToRegister(TX_POWER, NO_SUB, power, LEN_TX_POWER);
		_autoTXPower = false;
	}

	void setTXPower(int32_t power) {
		byte txpower[LEN_TX_POWER];
		DW1000NgUtils::writeValueToBytes(txpower, power, LEN_TX_POWER);
		setTXPower(txpower);
	}

	void setTXPower(DriverAmplifierValue driver_amplifier, TransmitMixerValue mixer) {
		byte txpower[LEN_TX_POWER];
		byte pwr = 0x00;

		pwr |= ((byte) driver_amplifier << 5);
		pwr |= (byte) mixer;

		for(auto i = 0; i < LEN_TX_POWER; i++) {
			txpower[i] = pwr;
		}

		setTXPower(txpower);
	}

	void setTXPowerAuto() {
		_autoTXPower = true;
		_txpowertune();
	}

	void setTCPGDelay(byte tcpgdelay) {
		byte tcpgBytes[LEN_TC_PGDELAY];
		DW1000NgUtils::writeValueToBytes(tcpgBytes, tcpgdelay, LEN_TC_PGDELAY);
		_writeBytesToRegister(TX_CAL, TC_PGDELAY_SUB, tcpgBytes, LEN_TC_PGDELAY);
		_autoTCPGDelay = false;
	}

	void setTCPGDelayAuto() {
		_tcpgdelaytune();
		_autoTCPGDelay = true;
	}

	void enableTransmitPowerSpectrumTestMode(int32_t repeat_interval) {
        _disableSequencing();
        _configureRFTransmitPowerSpectrumTestMode();
        _enableClock(SYS_PLL_CLOCK);
        _enableClock(TX_PLL_CLOCK);

        if(repeat_interval < 4) 
            repeat_interval = 4;

		/* In diagnostic transmit power  mode (set next) the bytes 31:0 only are used for DX_TIME register */
        byte delayBytes[4];
        DW1000NgUtils::writeValueToBytes(delayBytes, repeat_interval, 4);
        _writeBytesToRegister(DX_TIME, NO_SUB, delayBytes, 4);

		/* Enable Transmit Power Spectrum Test Mode */
        byte diagnosticBytes[2];
        DW1000NgUtils::writeValueToBytes(diagnosticBytes, 0x0010, LEN_DIAG_TMC);
        _writeBytesToRegister(DIG_DIAG, DIAG_TMC_SUB, diagnosticBytes, LEN_DIAG_TMC);
    }

	void setDelayedTRX(byte futureTimeBytes[]) {
		/* the least significant 9-bits are ignored in DX_TIME in functional modes */
		_writeBytesToRegister(DX_TIME, NO_SUB, futureTimeBytes, LEN_DX_TIME);
	}

	void setTransmitData(byte data[], uint16_t n) {
		if(_frameCheck) {
			n += 2; // two bytes CRC-16
		}
		if(n > LEN_EXT_UWB_FRAMES) {
			return; // TODO proper error handling: frame/buffer size
		}
		if(n > LEN_UWB_FRAMES && !_extendedFrameLength) {
			return; // TODO proper error handling: frame/buffer size
		}
		// transmit data and length
		_writeBytesToRegister(TX_BUFFER, NO_SUB, data, n);
		
		/* Sets up transmit frame control length based on data length */
		_txfctrl[0] = (byte)(n & 0xFF); // 1 byte (regular length + 1 bit)
		_txfctrl[1] &= 0xE0;
		_txfctrl[1] |= (byte)((n >> 8) & 0x03);  // 2 added bits if extended length
		_writeTransmitFrameControlRegister();
	}

	void setTransmitData(const String& data) {
		uint16_t n = data.length()+1;
		byte* dataBytes = (byte*)malloc(n);
		data.getBytes(dataBytes, n);
		setTransmitData(dataBytes, n);
		free(dataBytes);
	}

	// TODO reorder
	uint16_t getReceivedDataLength() {
		uint16_t len = 0;

		// 10 bits of RX frame control register
		byte rxFrameInfo[LEN_RX_FINFO];
		_readBytes(RX_FINFO, NO_SUB, rxFrameInfo, LEN_RX_FINFO);
		len = ((((uint16_t)rxFrameInfo[1] << 8) | (uint16_t)rxFrameInfo[0]) & 0x03FF);
		
		if(_frameCheck && len > 2) {
			return len-2;
		}
		return len;
	}

	void getReceivedData(byte data[], uint16_t n) {
		if(n <= 0) {
			return;
		}
		_readBytes(RX_BUFFER, NO_SUB, data, n);
	}

	void getReceivedData(String& data) {
		uint16_t i;
		uint16_t n = getReceivedDataLength(); // number of bytes w/o the two FCS ones
		if(n <= 0) { // TODO
			return;
		}
		byte* dataBytes = (byte*)malloc(n);
		getReceivedData(dataBytes, n);
		// clear string
		data.remove(0);
		data  = "";
		// append to string
		for(i = 0; i < n; i++) {
			data += (char)dataBytes[i];
		}
		free(dataBytes);
	}

	void getTransmitTimestamp(DW1000NgTime& time) {
		byte txTimeBytes[LEN_TX_STAMP];
		_readBytes(TX_TIME, TX_STAMP_SUB, txTimeBytes, LEN_TX_STAMP);
		time.setTimestamp(txTimeBytes);
	}

	void getReceiveTimestamp(DW1000NgTime& time) {
		byte rxTimeBytes[LEN_RX_STAMP];
		_readBytes(RX_TIME, RX_STAMP_SUB, rxTimeBytes, LEN_RX_STAMP);
		time.setTimestamp(rxTimeBytes);
		// correct timestamp (i.e. consider range bias)
		_correctTimestamp(time);
	}

	void getSystemTimestamp(DW1000NgTime& time) {
		byte sysTimeBytes[LEN_SYS_TIME];
		_readBytes(SYS_TIME, NO_SUB, sysTimeBytes, LEN_SYS_TIME);
		time.setTimestamp(sysTimeBytes);
	}

	void getTransmitTimestamp(byte data[]) {
		_readBytes(TX_TIME, TX_STAMP_SUB, data, LEN_TX_STAMP);
	}

	void getReceiveTimestamp(byte data[]) {
		_readBytes(RX_TIME, RX_STAMP_SUB, data, LEN_RX_STAMP);
	}

	void getSystemTimestamp(byte data[]) {
		_readBytes(SYS_TIME, NO_SUB, data, LEN_SYS_TIME);
	}

	float getReceiveQuality() {
		byte         noiseBytes[LEN_STD_NOISE];
		byte         fpAmpl2Bytes[LEN_FP_AMPL2];
		uint16_t     noise, f2;
		_readBytes(RX_FQUAL, STD_NOISE_SUB, noiseBytes, LEN_STD_NOISE);
		_readBytes(RX_FQUAL, FP_AMPL2_SUB, fpAmpl2Bytes, LEN_FP_AMPL2);
		noise = (uint16_t)noiseBytes[0] | ((uint16_t)noiseBytes[1] << 8);
		f2    = (uint16_t)fpAmpl2Bytes[0] | ((uint16_t)fpAmpl2Bytes[1] << 8);
		return (float)f2/noise;
	}

	float getFirstPathPower() {
		byte         fpAmpl1Bytes[LEN_FP_AMPL1];
		byte         fpAmpl2Bytes[LEN_FP_AMPL2];
		byte         fpAmpl3Bytes[LEN_FP_AMPL3];
		byte         rxFrameInfo[LEN_RX_FINFO];
		uint16_t     f1, f2, f3, N;
		float        A, corrFac;
		_readBytes(RX_TIME, FP_AMPL1_SUB, fpAmpl1Bytes, LEN_FP_AMPL1);
		_readBytes(RX_FQUAL, FP_AMPL2_SUB, fpAmpl2Bytes, LEN_FP_AMPL2);
		_readBytes(RX_FQUAL, FP_AMPL3_SUB, fpAmpl3Bytes, LEN_FP_AMPL3);
		_readBytes(RX_FINFO, NO_SUB, rxFrameInfo, LEN_RX_FINFO);
		f1 = (uint16_t)fpAmpl1Bytes[0] | ((uint16_t)fpAmpl1Bytes[1] << 8);
		f2 = (uint16_t)fpAmpl2Bytes[0] | ((uint16_t)fpAmpl2Bytes[1] << 8);
		f3 = (uint16_t)fpAmpl3Bytes[0] | ((uint16_t)fpAmpl3Bytes[1] << 8);
		N  = (((uint16_t)rxFrameInfo[2] >> 4) & 0xFF) | ((uint16_t)rxFrameInfo[3] << 4);

		if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
			A       = 113.77;
			corrFac = 2.3334;
		} else {
			A       = 121.74;
			corrFac = 1.1667;
		}
		float estFpPwr = 10.0*log10(((float)f1*(float)f1+(float)f2*(float)f2+(float)f3*(float)f3)/((float)N*(float)N))-A;
		if(estFpPwr <= -88) {
			return estFpPwr;
		} else {
			// approximation of Fig. 22 in user manual for dbm correction
			estFpPwr += (estFpPwr+88)*corrFac;
		}
		return estFpPwr;
	}

	float getReceivePower() {
		byte     cirPwrBytes[LEN_CIR_PWR];
		byte     rxFrameInfo[LEN_RX_FINFO];
		uint32_t twoPower17 = 131072;
		uint16_t C, N;
		float    A, corrFac;
		_readBytes(RX_FQUAL, CIR_PWR_SUB, cirPwrBytes, LEN_CIR_PWR);
		_readBytes(RX_FINFO, NO_SUB, rxFrameInfo, LEN_RX_FINFO);
		C = (uint16_t)cirPwrBytes[0] | ((uint16_t)cirPwrBytes[1] << 8);
		N = (((uint16_t)rxFrameInfo[2] >> 4) & 0xFF) | ((uint16_t)rxFrameInfo[3] << 4);

		if(_pulseFrequency == PulseFrequency::FREQ_16MHZ) {
			A       = 113.77;
			corrFac = 2.3334;
		} else {
			A       = 121.74;
			corrFac = 1.1667;
		}
		float estRxPwr = 10.0*log10(((float)C*(float)twoPower17)/((float)N*(float)N))-A;
		if(estRxPwr <= -88) {
			return estRxPwr;
		} else {
			// approximation of Fig. 22 in user manual for dbm correction
			estRxPwr += (estRxPwr+88)*corrFac;
		}
		return estRxPwr;
	}

	#if DW1000NG_DEBUG
	void getPrettyBytes(byte data[], char msgBuffer[], uint16_t n) {
        uint16_t i, j, b;
        b = sprintf(msgBuffer, "Data, bytes: %d\nB: 7 6 5 4 3 2 1 0\n", n); // TODO - type
        for(i = 0; i < n; i++) {
            byte curByte = data[i];
            snprintf(&msgBuffer[b++], 2, "%d", (i+1));
            msgBuffer[b++] = (char)((i+1) & 0xFF);
            msgBuffer[b++] = ':';
            msgBuffer[b++] = ' ';
            for(j = 0; j < 8; j++) {
                msgBuffer[b++] = ((curByte >> (7-j)) & 0x01) ? '1' : '0';
                if(j < 7) {
                    msgBuffer[b++] = ' ';
                } else if(i < n-1) {
                    msgBuffer[b++] = '\n';
                } else {
                    msgBuffer[b++] = '\0';
                }
            }
        }
        msgBuffer[b++] = '\0';
    }

    void getPrettyBytes(byte cmd, uint16_t offset, char msgBuffer[], uint16_t n) {
        uint16_t i, j, b;
        byte* readBuf = (byte*)malloc(n);
        _readBytes(cmd, offset, readBuf, n);
        b     = sprintf(msgBuffer, "Reg: 0x%02x, bytes: %d\nB: 7 6 5 4 3 2 1 0\n", cmd, n);  // TODO - tpye
        for(i = 0; i < n; i++) {
            byte curByte = readBuf[i];
            snprintf(&msgBuffer[b++], 2, "%d", (i+1));
            msgBuffer[b++] = (char)((i+1) & 0xFF);
            msgBuffer[b++] = ':';
            msgBuffer[b++] = ' ';
            for(j = 0; j < 8; j++) {
                msgBuffer[b++] = ((curByte >> (7-j)) & 0x01) ? '1' : '0';
                if(j < 7) {
                    msgBuffer[b++] = ' ';
                } else if(i < n-1) {
                    msgBuffer[b++] = '\n';
                } else {
                    msgBuffer[b++] = '\0';
                }
            }
        }
        msgBuffer[b++] = '\0';
        free(readBuf);
    }
	#endif
}