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

/*
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>
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
 * @file DWM1000.cpp
 * Arduino driver library (source file) for the Decawave DWM1000 UWB transceiver Module.
 */

#include <stdio.h>
#include <stdlib.h>
#include <SPI.h>
#include <string.h>
#include "DWM1000Utils.hpp"
#include "DWM1000Constants.hpp"
#include "DWM1000.hpp"
#include "DWM1000Time.hpp"

namespace DWM1000 {
	
	/* anonymous namespace to host private-like variables and methods */
	namespace {

		/* ########################### PRIVATE VARIABLES ################################# */

		/* pins */
		uint8_t _ss;
		uint8_t _rst;
		uint8_t _irq;

		/* IRQ callbacks */
		void (* _handleSent)(void)                      = nullptr;
		void (* _handleError)(void)                     = nullptr;
		void (* _handleReceived)(void)                  = nullptr;
		void (* _handleReceiveFailed)(void)             = nullptr;
		void (* _handleReceiveTimeout)(void)            = nullptr;
		void (* _handleReceiveTimestampAvailable)(void) = nullptr;

		/* SFD Mode */
		void _useDecawaveSFD();
		void _useStandardSFD();
		void _useRecommendedSFD();
		void (* _currentSFDMode)(void) = _useRecommendedSFD;

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
		byte        _extendedFrameLength;
		byte        _pacSize;
		byte        _pulseFrequency;
		byte        _dataRate;
		byte        _preambleLength;
		byte        _preambleCode;
		byte        _channel;
		boolean     _smartPower;
		boolean     _frameCheck;
		boolean     _debounceClockEnabled = false;
		boolean     _nlos = false;
		boolean     _autoTXPower = true;
		boolean     _autoTCPGDelay = true;
		DWM1000Time _antennaDelay;

		/* SPI relative variables */
		const SPISettings  _fastSPI = SPISettings(16000000L, MSBFIRST, SPI_MODE0);
		const SPISettings  _slowSPI = SPISettings(2000000L, MSBFIRST, SPI_MODE0);
		const SPISettings* _currentSPI = &_fastSPI;

		/* ############################# PRIVATE METHODS ################################### */

		/* Steps used to get Temp and Voltage */
		void _vbatAndTempSteps() {
			byte step1 = 0x80; writeBytes(RF_CONF, 0x11, &step1, 1);
			byte step2 = 0x0A; writeBytes(RF_CONF, 0x12, &step2, 1);
			byte step3 = 0x0F; writeBytes(RF_CONF, 0x12, &step3, 1);
			byte step4 = 0x01; writeBytes(TX_CAL, NO_SUB, &step4, 1);
			byte step5 = 0x00; writeBytes(TX_CAL, NO_SUB, &step5, 1);
		}

		/* AGC_TUNE1 - reg:0x23, sub-reg:0x04, table 24 */
		void _agctune1() {
			byte agctune1[LEN_AGC_TUNE1];
			if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
				DWM1000Utils::writeValueToBytes(agctune1, 0x8870, LEN_AGC_TUNE1);
			} else if(_pulseFrequency == TX_PULSE_FREQ_64MHZ) {
				DWM1000Utils::writeValueToBytes(agctune1, 0x889B, LEN_AGC_TUNE1);
			} else {
				// TODO proper error/warning handling
			}
			writeBytes(AGC_TUNE, AGC_TUNE1_SUB, agctune1, LEN_AGC_TUNE1);
		}

		/* AGC_TUNE2 - reg:0x23, sub-reg:0x0C, table 25 */
		void _agctune2() {
			byte agctune2[LEN_AGC_TUNE2];
			DWM1000Utils::writeValueToBytes(agctune2, 0x2502A907L, LEN_AGC_TUNE2);
			writeBytes(AGC_TUNE, AGC_TUNE2_SUB, agctune2, LEN_AGC_TUNE2);
		}

		/* AGC_TUNE3 - reg:0x23, sub-reg:0x12, table 26 */
		void _agctune3() {
			byte agctune3[LEN_AGC_TUNE3];
			DWM1000Utils::writeValueToBytes(agctune3, 0x0035, LEN_AGC_TUNE3);
			writeBytes(AGC_TUNE, AGC_TUNE3_SUB, agctune3, LEN_AGC_TUNE3);
		}

		/* DRX_TUNE0b - reg:0x27, sub-reg:0x02 (already optimized according to Table 30 of user manual) */
		void _drxtune0b() {
			byte drxtune0b[LEN_DRX_TUNE0b];	
			if(_dataRate == TRX_RATE_110KBPS) {
				DWM1000Utils::writeValueToBytes(drxtune0b, 0x0016, LEN_DRX_TUNE0b);
			} else if(_dataRate == TRX_RATE_850KBPS) {
				DWM1000Utils::writeValueToBytes(drxtune0b, 0x0006, LEN_DRX_TUNE0b);
			} else if(_dataRate == TRX_RATE_6800KBPS) {
				DWM1000Utils::writeValueToBytes(drxtune0b, 0x0001, LEN_DRX_TUNE0b);
			} else {
				// TODO proper error/warning handling
			}
			writeBytes(DRX_TUNE, DRX_TUNE0b_SUB, drxtune0b, LEN_DRX_TUNE0b);
		}

		/* DRX_TUNE1a - reg:0x27, sub-reg:0x04, table 31 */
		void _drxtune1a() {
			byte drxtune1a[LEN_DRX_TUNE1a];
			if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
				DWM1000Utils::writeValueToBytes(drxtune1a, 0x0087, LEN_DRX_TUNE1a);
			} else if(_pulseFrequency == TX_PULSE_FREQ_64MHZ) {
				DWM1000Utils::writeValueToBytes(drxtune1a, 0x008D, LEN_DRX_TUNE1a);
			} else {
				// TODO proper error/warning handling
			}
			writeBytes(DRX_TUNE, DRX_TUNE1a_SUB, drxtune1a, LEN_DRX_TUNE1a);
		}

		/* DRX_TUNE1b - reg:0x27, sub-reg:0x06, table 32 */
		void _drxtune1b() {
			byte drxtune1b[LEN_DRX_TUNE1b];
			if(_preambleLength == TX_PREAMBLE_LEN_1536 || _preambleLength == TX_PREAMBLE_LEN_2048 ||
				_preambleLength == TX_PREAMBLE_LEN_4096) {
				if(_dataRate == TRX_RATE_110KBPS) {
					DWM1000Utils::writeValueToBytes(drxtune1b, 0x0064, LEN_DRX_TUNE1b);
				} else {
					// TODO proper error/warning handling
				}
			} else if(_preambleLength != TX_PREAMBLE_LEN_64) {
				if(_dataRate == TRX_RATE_850KBPS || _dataRate == TRX_RATE_6800KBPS) {
					DWM1000Utils::writeValueToBytes(drxtune1b, 0x0020, LEN_DRX_TUNE1b);
				} else {
					// TODO proper error/warning handling
				}
			} else {
				if(_dataRate == TRX_RATE_6800KBPS) {
					DWM1000Utils::writeValueToBytes(drxtune1b, 0x0010, LEN_DRX_TUNE1b);
				} else {
					// TODO proper error/warning handling
				}
			}
			writeBytes(DRX_TUNE, DRX_TUNE1b_SUB, drxtune1b, LEN_DRX_TUNE1b);
		}

		/* DRX_TUNE2 - reg:0x27, sub-reg:0x08, table 33 */
		void _drxtune2() {
			byte drxtune2[LEN_DRX_TUNE2];	
			if(_pacSize == PAC_SIZE_8) {
				if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
					DWM1000Utils::writeValueToBytes(drxtune2, 0x311A002DL, LEN_DRX_TUNE2);
				} else if(_pulseFrequency == TX_PULSE_FREQ_64MHZ) {
					DWM1000Utils::writeValueToBytes(drxtune2, 0x313B006BL, LEN_DRX_TUNE2);
				} else {
					// TODO proper error/warning handling
				}
			} else if(_pacSize == PAC_SIZE_16) {
				if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
					DWM1000Utils::writeValueToBytes(drxtune2, 0x331A0052L, LEN_DRX_TUNE2);
				} else if(_pulseFrequency == TX_PULSE_FREQ_64MHZ) {
					DWM1000Utils::writeValueToBytes(drxtune2, 0x333B00BEL, LEN_DRX_TUNE2);
				} else {
					// TODO proper error/warning handling
				}
			} else if(_pacSize == PAC_SIZE_32) {
				if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
					DWM1000Utils::writeValueToBytes(drxtune2, 0x351A009AL, LEN_DRX_TUNE2);
				} else if(_pulseFrequency == TX_PULSE_FREQ_64MHZ) {
					DWM1000Utils::writeValueToBytes(drxtune2, 0x353B015EL, LEN_DRX_TUNE2);
				} else {
					// TODO proper error/warning handling
				}
			} else if(_pacSize == PAC_SIZE_64) {
				if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
					DWM1000Utils::writeValueToBytes(drxtune2, 0x371A011DL, LEN_DRX_TUNE2);
				} else if(_pulseFrequency == TX_PULSE_FREQ_64MHZ) {
					DWM1000Utils::writeValueToBytes(drxtune2, 0x373B0296L, LEN_DRX_TUNE2);
				} else {
					// TODO proper error/warning handling
				}
			} else {
				// TODO proper error/warning handling
			}
			writeBytes(DRX_TUNE, DRX_TUNE2_SUB, drxtune2, LEN_DRX_TUNE2);
		}

		/* DRX_TUNE4H - reg:0x27, sub-reg:0x26, table 34 */
		void _drxtune4H() {
			byte drxtune4H[LEN_DRX_TUNE4H];
			if(_preambleLength == TX_PREAMBLE_LEN_64) {
				DWM1000Utils::writeValueToBytes(drxtune4H, 0x0010, LEN_DRX_TUNE4H);
			} else {
				DWM1000Utils::writeValueToBytes(drxtune4H, 0x0028, LEN_DRX_TUNE4H);
			}
			writeBytes(DRX_TUNE, DRX_TUNE4H_SUB, drxtune4H, LEN_DRX_TUNE4H);
		}

		/* LDE_CFG1 - reg 0x2E, sub-reg:0x0806 */
		void _ldecfg1() {
			byte ldecfg1[LEN_LDE_CFG1];
			_nlos == true ? DWM1000Utils::writeValueToBytes(ldecfg1, 0x7, LEN_LDE_CFG1) : DWM1000Utils::writeValueToBytes(ldecfg1, 0xD, LEN_LDE_CFG1);
			writeBytes(LDE_IF, LDE_CFG1_SUB, ldecfg1, LEN_LDE_CFG1);
		}

		/* LDE_CFG2 - reg 0x2E, sub-reg:0x1806, table 50 */
		void _ldecfg2() {
			byte ldecfg2[LEN_LDE_CFG2];	
			if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
				_nlos == true ? DWM1000Utils::writeValueToBytes(ldecfg2, 0x0003, LEN_LDE_CFG2) : DWM1000Utils::writeValueToBytes(ldecfg2, 0x1607, LEN_LDE_CFG2);
			} else if(_pulseFrequency == TX_PULSE_FREQ_64MHZ) {
				DWM1000Utils::writeValueToBytes(ldecfg2, 0x0607, LEN_LDE_CFG2);
			} else {
				// TODO proper error/warning handling
			}
			writeBytes(LDE_IF, LDE_CFG2_SUB, ldecfg2, LEN_LDE_CFG2);
		}

		/* LDE_REPC - reg 0x2E, sub-reg:0x2804, table 51 */
		void _lderepc() {
			byte lderepc[LEN_LDE_REPC];
			if(_preambleCode == PREAMBLE_CODE_16MHZ_1 || _preambleCode == PREAMBLE_CODE_16MHZ_2) {
				if(_dataRate == TRX_RATE_110KBPS) {
					DWM1000Utils::writeValueToBytes(lderepc, ((0x5998 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DWM1000Utils::writeValueToBytes(lderepc, 0x5998, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PREAMBLE_CODE_16MHZ_3 || _preambleCode == PREAMBLE_CODE_16MHZ_8) {
				if(_dataRate == TRX_RATE_110KBPS) {
					DWM1000Utils::writeValueToBytes(lderepc, ((0x51EA >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DWM1000Utils::writeValueToBytes(lderepc, 0x51EA, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PREAMBLE_CODE_16MHZ_4) {
				if(_dataRate == TRX_RATE_110KBPS) {
					DWM1000Utils::writeValueToBytes(lderepc, ((0x428E >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DWM1000Utils::writeValueToBytes(lderepc, 0x428E, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PREAMBLE_CODE_16MHZ_5) {
				if(_dataRate == TRX_RATE_110KBPS) {
					DWM1000Utils::writeValueToBytes(lderepc, ((0x451E >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DWM1000Utils::writeValueToBytes(lderepc, 0x451E, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PREAMBLE_CODE_16MHZ_6) {
				if(_dataRate == TRX_RATE_110KBPS) {
					DWM1000Utils::writeValueToBytes(lderepc, ((0x2E14 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DWM1000Utils::writeValueToBytes(lderepc, 0x2E14, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PREAMBLE_CODE_16MHZ_7) {
				if(_dataRate == TRX_RATE_110KBPS) {
					DWM1000Utils::writeValueToBytes(lderepc, ((0x8000 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DWM1000Utils::writeValueToBytes(lderepc, 0x8000, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PREAMBLE_CODE_64MHZ_9) {
				if(_dataRate == TRX_RATE_110KBPS) {
					DWM1000Utils::writeValueToBytes(lderepc, ((0x28F4 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DWM1000Utils::writeValueToBytes(lderepc, 0x28F4, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PREAMBLE_CODE_64MHZ_10 || _preambleCode == PREAMBLE_CODE_64MHZ_17) {
				if(_dataRate == TRX_RATE_110KBPS) {
					DWM1000Utils::writeValueToBytes(lderepc, ((0x3332 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DWM1000Utils::writeValueToBytes(lderepc, 0x3332, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PREAMBLE_CODE_64MHZ_11) {
				if(_dataRate == TRX_RATE_110KBPS) {
					DWM1000Utils::writeValueToBytes(lderepc, ((0x3AE0 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DWM1000Utils::writeValueToBytes(lderepc, 0x3AE0, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PREAMBLE_CODE_64MHZ_12) {
				if(_dataRate == TRX_RATE_110KBPS) {
					DWM1000Utils::writeValueToBytes(lderepc, ((0x3D70 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DWM1000Utils::writeValueToBytes(lderepc, 0x3D70, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PREAMBLE_CODE_64MHZ_18 || _preambleCode == PREAMBLE_CODE_64MHZ_19) {
				if(_dataRate == TRX_RATE_110KBPS) {
					DWM1000Utils::writeValueToBytes(lderepc, ((0x35C2 >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DWM1000Utils::writeValueToBytes(lderepc, 0x35C2, LEN_LDE_REPC);
				}
			} else if(_preambleCode == PREAMBLE_CODE_64MHZ_20) {
				if(_dataRate == TRX_RATE_110KBPS) {
					DWM1000Utils::writeValueToBytes(lderepc, ((0x47AE >> 3) & 0xFFFF), LEN_LDE_REPC);
				} else {
					DWM1000Utils::writeValueToBytes(lderepc, 0x47AE, LEN_LDE_REPC);
				}
			} else {
				// TODO proper error/warning handling
			}
			
			writeBytes(LDE_IF, LDE_REPC_SUB, lderepc, LEN_LDE_REPC);
		}

		/* TX_POWER (enabled smart transmit power control) - reg:0x1E, tables 19-20
		* These values are based on a typical IC and an assumed IC to antenna loss of 1.5 dB with a 0 dBi antenna */
		void _txpowertune() {
			byte txpower[LEN_TX_POWER];
			if(_channel == CHANNEL_1 || _channel == CHANNEL_2) {
				if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
					if(_smartPower) {
						DWM1000Utils::writeValueToBytes(txpower, 0x1B153555L, LEN_TX_POWER);
					} else {
						DWM1000Utils::writeValueToBytes(txpower, 0x55555555L, LEN_TX_POWER);
					}
				} else if(_pulseFrequency == TX_PULSE_FREQ_64MHZ) {
					if(_smartPower) {
						DWM1000Utils::writeValueToBytes(txpower, 0x0D072747L, LEN_TX_POWER);
					} else {
						DWM1000Utils::writeValueToBytes(txpower, 0x47474747L, LEN_TX_POWER);
					}
				} else {
					// TODO proper error/warning handling
				}
			} else if(_channel == CHANNEL_3) {
				if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
					if(_smartPower) {
						DWM1000Utils::writeValueToBytes(txpower, 0x150F2F4FL, LEN_TX_POWER);
					} else {
						DWM1000Utils::writeValueToBytes(txpower, 0x4F4F4F4FL, LEN_TX_POWER);
					}
				} else if(_pulseFrequency == TX_PULSE_FREQ_64MHZ) {
					if(_smartPower) {
						DWM1000Utils::writeValueToBytes(txpower, 0x0B2B4B6BL, LEN_TX_POWER);
					} else {
						DWM1000Utils::writeValueToBytes(txpower, 0x6B6B6B6BL, LEN_TX_POWER);
					}
				} else {
					// TODO proper error/warning handling
				}
			} else if(_channel == CHANNEL_4) {
				if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
					if(_smartPower) {
						DWM1000Utils::writeValueToBytes(txpower, 0x1F1F1F3FL, LEN_TX_POWER);
					} else {
						DWM1000Utils::writeValueToBytes(txpower, 0x3F3F3F3FL, LEN_TX_POWER);
					}
				} else if(_pulseFrequency == TX_PULSE_FREQ_64MHZ) {
					if(_smartPower) {
						DWM1000Utils::writeValueToBytes(txpower, 0x1A3A5A7AL, LEN_TX_POWER);
					} else {
						DWM1000Utils::writeValueToBytes(txpower, 0x7A7A7A7AL, LEN_TX_POWER);
					}
				} else {
					// TODO proper error/warning handling
				}
			} else if(_channel == CHANNEL_5) {
				if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
					if(_smartPower) {
						DWM1000Utils::writeValueToBytes(txpower, 0x140E0828L, LEN_TX_POWER);
					} else {
						DWM1000Utils::writeValueToBytes(txpower, 0x28282828L, LEN_TX_POWER);
					}
				} else if(_pulseFrequency == TX_PULSE_FREQ_64MHZ) {
					if(_smartPower) {
						DWM1000Utils::writeValueToBytes(txpower, 0x05254565L, LEN_TX_POWER);
					} else {
						DWM1000Utils::writeValueToBytes(txpower, 0x65656565L, LEN_TX_POWER);
					}
				} else {
					// TODO proper error/warning handling
				}
			} else if(_channel == CHANNEL_7) {
				if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
					if(_smartPower) {
						DWM1000Utils::writeValueToBytes(txpower, 0x12325272L, LEN_TX_POWER);
					} else {
						DWM1000Utils::writeValueToBytes(txpower, 0x72727272L, LEN_TX_POWER);
					}
				} else if(_pulseFrequency == TX_PULSE_FREQ_64MHZ) {
					if(_smartPower) {
						DWM1000Utils::writeValueToBytes(txpower, 0x315191B1L, LEN_TX_POWER);
					} else {
						DWM1000Utils::writeValueToBytes(txpower, 0xB1B1B1B1L, LEN_TX_POWER);
					}
				} else {
					// TODO proper error/warning handling
				}
			} else {
				// TODO proper error/warning handling
			}
			writeBytes(TX_POWER, NO_SUB, txpower, LEN_TX_POWER);
		}

		/* RF_RXCTRLH - reg:0x28, sub-reg:0x0B, table 37 */
		void _rfrxctrlh() {
			byte rfrxctrlh[LEN_RF_RXCTRLH];
			if(_channel != CHANNEL_4 && _channel != CHANNEL_7) {
				DWM1000Utils::writeValueToBytes(rfrxctrlh, 0xD8, LEN_RF_RXCTRLH);
			} else {
				DWM1000Utils::writeValueToBytes(rfrxctrlh, 0xBC, LEN_RF_RXCTRLH);
			}
			writeBytes(RF_CONF, RF_RXCTRLH_SUB, rfrxctrlh, LEN_RF_RXCTRLH);
		}

		/* RX_TXCTRL - reg:0x28, sub-reg:0x0C */
		void _rftxctrl() {
			byte rftxctrl[LEN_RF_TXCTRL];
			if(_channel == CHANNEL_1) {
				DWM1000Utils::writeValueToBytes(rftxctrl, 0x00005C40L, LEN_RF_TXCTRL);
			} else if(_channel == CHANNEL_2) {
				DWM1000Utils::writeValueToBytes(rftxctrl, 0x00045CA0L, LEN_RF_TXCTRL);
			} else if(_channel == CHANNEL_3) {
				DWM1000Utils::writeValueToBytes(rftxctrl, 0x00086CC0L, LEN_RF_TXCTRL);
			} else if(_channel == CHANNEL_4) {
				DWM1000Utils::writeValueToBytes(rftxctrl, 0x00045C80L, LEN_RF_TXCTRL);
			} else if(_channel == CHANNEL_5) {
				DWM1000Utils::writeValueToBytes(rftxctrl, 0x001E3FE0L, LEN_RF_TXCTRL);
			} else if(_channel == CHANNEL_7) {
				DWM1000Utils::writeValueToBytes(rftxctrl, 0x001E7DE0L, LEN_RF_TXCTRL);
			} else {
				// TODO proper error/warning handling
			}
			writeBytes(RF_CONF, RF_TXCTRL_SUB, rftxctrl, LEN_RF_TXCTRL);
		}

		/* TC_PGDELAY - reg:0x2A, sub-reg:0x0B, table 40 */
		void _tcpgdelaytune() {
			byte tcpgdelay[LEN_TC_PGDELAY];	
			if(_channel == CHANNEL_1) {
				DWM1000Utils::writeValueToBytes(tcpgdelay, 0xC9, LEN_TC_PGDELAY);
			} else if(_channel == CHANNEL_2) {
				DWM1000Utils::writeValueToBytes(tcpgdelay, 0xC2, LEN_TC_PGDELAY);
			} else if(_channel == CHANNEL_3) {
				DWM1000Utils::writeValueToBytes(tcpgdelay, 0xC5, LEN_TC_PGDELAY);
			} else if(_channel == CHANNEL_4) {
				DWM1000Utils::writeValueToBytes(tcpgdelay, 0x95, LEN_TC_PGDELAY);
			} else if(_channel == CHANNEL_5) {
				DWM1000Utils::writeValueToBytes(tcpgdelay, 0xC0, LEN_TC_PGDELAY);
			} else if(_channel == CHANNEL_7) {
				DWM1000Utils::writeValueToBytes(tcpgdelay, 0x93, LEN_TC_PGDELAY);
			} else {
				// TODO proper error/warning handling
			}
			writeBytes(TX_CAL, TC_PGDELAY_SUB, tcpgdelay, LEN_TC_PGDELAY);
		}

		// FS_PLLCFG and FS_PLLTUNE - reg:0x2B, sub-reg:0x07-0x0B, tables 43-44
		void _fspll() {
			byte fspllcfg[LEN_FS_PLLCFG];
			byte fsplltune[LEN_FS_PLLTUNE];
			if(_channel == CHANNEL_1) {
				DWM1000Utils::writeValueToBytes(fspllcfg, 0x09000407L, LEN_FS_PLLCFG);
				DWM1000Utils::writeValueToBytes(fsplltune, 0x1E, LEN_FS_PLLTUNE);
			} else if(_channel == CHANNEL_2 || _channel == CHANNEL_4) {
				DWM1000Utils::writeValueToBytes(fspllcfg, 0x08400508L, LEN_FS_PLLCFG);
				DWM1000Utils::writeValueToBytes(fsplltune, 0x26, LEN_FS_PLLTUNE);
			} else if(_channel == CHANNEL_3) {
				DWM1000Utils::writeValueToBytes(fspllcfg, 0x08401009L, LEN_FS_PLLCFG);
				DWM1000Utils::writeValueToBytes(fsplltune, 0x56, LEN_FS_PLLTUNE);
			} else if(_channel == CHANNEL_5 || _channel == CHANNEL_7) {
				DWM1000Utils::writeValueToBytes(fspllcfg, 0x0800041DL, LEN_FS_PLLCFG);
				DWM1000Utils::writeValueToBytes(fsplltune, 0xBE, LEN_FS_PLLTUNE);
			} else {
				// TODO proper error/warning handling
			}
			writeBytes(FS_CTRL, FS_PLLTUNE_SUB, fsplltune, LEN_FS_PLLTUNE);
			writeBytes(FS_CTRL, FS_PLLCFG_SUB, fspllcfg, LEN_FS_PLLCFG);
		}

		/* Crystal calibration from OTP (if available)
		* FS_XTALT - reg:0x2B, sub-reg:0x0E
		* OTP(one-time-programmable) memory map - table 10 */
		void _fsxtalt() {
			byte fsxtalt[LEN_FS_XTALT];
			byte buf_otp[4];
			readBytesOTP(0x01E, buf_otp); //0x01E -> byte[0]=XTAL_Trim
			if (buf_otp[0] == 0) {
				// No trim value available from OTP, use midrange value of 0x10
				DWM1000Utils::writeValueToBytes(fsxtalt, ((0x10 & 0x1F) | 0x60), LEN_FS_XTALT);
			} else {
				DWM1000Utils::writeValueToBytes(fsxtalt, ((buf_otp[0] & 0x1F) | 0x60), LEN_FS_XTALT);
			}
			// write configuration back to chip
			writeBytes(FS_CTRL, FS_XTALT_SUB, fsxtalt, LEN_FS_XTALT);
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

		void _writeSystemConfigurationRegister() {
			writeBytes(SYS_CFG, NO_SUB, _syscfg, LEN_SYS_CFG);
		}

		void _writeChannelControlRegister() {
			writeBytes(CHAN_CTRL, NO_SUB, _chanctrl, LEN_CHAN_CTRL);
		}

		void _writeTransmitFrameControlRegister() {
			writeBytes(TX_FCTRL, NO_SUB, _txfctrl, LEN_TX_FCTRL);
		}

		void _writeSystemEventMaskRegister() {
			writeBytes(SYS_MASK, NO_SUB, _sysmask, LEN_SYS_MASK);
		}

		void _writeAntennaDelayRegisters() {
			byte antennaDelayBytes[DWM1000Time::LENGTH_TIMESTAMP];
			_antennaDelay.getTimestamp(antennaDelayBytes);
			writeBytes(TX_ANTD, NO_SUB, antennaDelayBytes, LEN_TX_ANTD);
			writeBytes(LDE_IF, LDE_RXANTD_SUB, antennaDelayBytes, LEN_LDE_RXANTD);
		}

		void _writeConfiguration() {
			// write all configurations back to device
			writeNetworkIdAndDeviceAddress();
			_writeSystemConfigurationRegister();
			_writeChannelControlRegister();
			_writeTransmitFrameControlRegister();
			_writeSystemEventMaskRegister();
			_writeAntennaDelayRegisters();
		}

		void _manageLDE() {
			// transfer any ldo tune values
			byte ldoTune[LEN_OTP_RDAT];
			readBytesOTP(0x04, ldoTune); // TODO #define
			if(ldoTune[0] != 0) {
				// TODO tuning available, copy over to RAM: use OTP_LDO bit
			}
			// tell the chip to load the LDE microcode
			// TODO remove clock-related code (PMSC_CTRL) as handled separately
			byte pmscctrl0[LEN_PMSC_CTRL0];
			byte otpctrl[LEN_OTP_CTRL];
			memset(pmscctrl0, 0, LEN_PMSC_CTRL0);
			memset(otpctrl, 0, LEN_OTP_CTRL);
			readBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
			readBytes(OTP_IF, OTP_CTRL_SUB, otpctrl, LEN_OTP_CTRL);
			pmscctrl0[0] = 0x01;
			pmscctrl0[1] = 0x03;
			otpctrl[0]   = 0x00;
			otpctrl[1]   = 0x80;
			writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, 2);
			writeBytes(OTP_IF, OTP_CTRL_SUB, otpctrl, 2);
			delay(5);
			pmscctrl0[0] = 0x00;
			pmscctrl0[1] &= 0x02;
			writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, 2);
		}

		void _useDecawaveSFD() {
			DWM1000Utils::setBit(_chanctrl, LEN_CHAN_CTRL, DWSFD_BIT, true);
			DWM1000Utils::setBit(_chanctrl, LEN_CHAN_CTRL, TNSSFD_BIT, true);
			DWM1000Utils::setBit(_chanctrl, LEN_CHAN_CTRL, RNSSFD_BIT, true);
			switch(_dataRate) {
				case TRX_RATE_6800KBPS:
					writeByte(USR_SFD, SFD_LENGTH_SUB, 0x08);
					break;
				case TRX_RATE_850KBPS:
					writeByte(USR_SFD, SFD_LENGTH_SUB, 0x10);
					break;
				case TRX_RATE_110KBPS:
					writeByte(USR_SFD, SFD_LENGTH_SUB, 0x40);
					break;
				default:
					return; //TODO Proper error handling
			}
		}

		void _useStandardSFD() {
			DWM1000Utils::setBit(_chanctrl, LEN_CHAN_CTRL, DWSFD_BIT, false);
			DWM1000Utils::setBit(_chanctrl, LEN_CHAN_CTRL, TNSSFD_BIT, false);
			DWM1000Utils::setBit(_chanctrl, LEN_CHAN_CTRL, RNSSFD_BIT, false);
			switch(_dataRate) {
				case TRX_RATE_6800KBPS:
					writeByte(USR_SFD, SFD_LENGTH_SUB, 0x08);
					break;
				case TRX_RATE_850KBPS:
					writeByte(USR_SFD, SFD_LENGTH_SUB, 0x08);
					break;
				case TRX_RATE_110KBPS:
					writeByte(USR_SFD, SFD_LENGTH_SUB, 0x40);
					break;
				default:
					return; //TODO Proper error handling
			}
		}

		void _useRecommendedSFD() {
			/* SFD mode and types recommended by DW1000 User manual for optimal performance */
			switch(_dataRate) {
				case TRX_RATE_6800KBPS:
					DWM1000Utils::setBit(_chanctrl, LEN_CHAN_CTRL, DWSFD_BIT, false);
					DWM1000Utils::setBit(_chanctrl, LEN_CHAN_CTRL, TNSSFD_BIT, false);
					DWM1000Utils::setBit(_chanctrl, LEN_CHAN_CTRL, RNSSFD_BIT, false);
					writeByte(USR_SFD, SFD_LENGTH_SUB, 0x08);
					break;
				case TRX_RATE_850KBPS:
					DWM1000Utils::setBit(_chanctrl, LEN_CHAN_CTRL, DWSFD_BIT, true);
					DWM1000Utils::setBit(_chanctrl, LEN_CHAN_CTRL, TNSSFD_BIT, true);
					DWM1000Utils::setBit(_chanctrl, LEN_CHAN_CTRL, RNSSFD_BIT, true);
					writeByte(USR_SFD, SFD_LENGTH_SUB, 0x10);
					break;
				case TRX_RATE_110KBPS:
					DWM1000Utils::setBit(_chanctrl, LEN_CHAN_CTRL, DWSFD_BIT, true);
					DWM1000Utils::setBit(_chanctrl, LEN_CHAN_CTRL, TNSSFD_BIT, false);
					DWM1000Utils::setBit(_chanctrl, LEN_CHAN_CTRL, RNSSFD_BIT, false);
					writeByte(USR_SFD, SFD_LENGTH_SUB, 0x40);
					break;
				default:
					return; //TODO Error handling
			}
		}

		void _enableClock(byte clock) {
			byte pmscctrl0[LEN_PMSC_CTRL0];
			memset(pmscctrl0, 0, LEN_PMSC_CTRL0);
			readBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
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
			writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, 2);
		}
		
		/* interrupt state handling */
		
		void _clearInterrupts() {
			memset(_sysmask, 0, LEN_SYS_MASK);
		}

		void _clearAllStatus() {
			//Latched bits in status register are reset by writing 1 to them
			memset(_sysstatus, 0xff, LEN_SYS_STATUS);
			writeBytes(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
		}

		void _clearReceiveStatus() {
			// clear latched RX bits (i.e. write 1 to clear)
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, RXDFR_BIT, true);
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, RXFCG_BIT, true);
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, RXPRD_BIT, true);
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, RXSFDD_BIT, true);
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, RXPHD_BIT, true);
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, LDEDONE_BIT, true);
			writeBytes(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
		}

		void _clearReceiveTimestampAvailableStatus() {
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, LDEDONE_BIT, true);
			writeBytes(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
		}

		void _clearReceiveTimeoutStatus() {
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, RXRFTO_BIT, true);
			writeBytes(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
		}

		void _clearReceiveFailedStatus() {
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, RXPHE_BIT, true);
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, RXFCE_BIT, true);
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, RXRFSL_BIT, true);
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, RXSFDTO_BIT, true);
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, AFFREJ_BIT, true);
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, LDEERR_BIT, true);
			writeBytes(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
		}

		void _clearTransmitStatus() {
			// clear latched TX bits
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, AAT_BIT, true);
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, TXFRB_BIT, true);
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, TXPRS_BIT, true);
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, TXPHS_BIT, true);
			DWM1000Utils::setBit(_sysstatus, LEN_SYS_STATUS, TXFRS_BIT, true);
			writeBytes(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
		}

		void _resetReceiver() {
			byte pmscctrl0[LEN_PMSC_CTRL0];
			readBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
			pmscctrl0[3] = 0xE0;
			writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
			pmscctrl0[3] = 0xF0;
			writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
		}

		/* Internal helpers to read configuration */

		void _readSystemConfigurationRegister() {
			readBytes(SYS_CFG, NO_SUB, _syscfg, LEN_SYS_CFG);
		}

		void _readSystemEventStatusRegister() {
			readBytes(SYS_STATUS, NO_SUB, _sysstatus, LEN_SYS_STATUS);
		}

		void _readNetworkIdAndDeviceAddress() {
			readBytes(PANADR, NO_SUB, _networkAndAddress, LEN_PANADR);
		}

		void _readSystemEventMaskRegister() {
			readBytes(SYS_MASK, NO_SUB, _sysmask, LEN_SYS_MASK);
		}

		void _readChannelControlRegister() {
			readBytes(CHAN_CTRL, NO_SUB, _chanctrl, LEN_CHAN_CTRL);
		}

		void _readTransmitFrameControlRegister() {
			readBytes(TX_FCTRL, NO_SUB, _txfctrl, LEN_TX_FCTRL);
		}

		void _handleInterrupt() {
			// read current status and handle via callbacks
			_readSystemEventStatusRegister();
			if(isClockProblem() /* TODO and others */ && _handleError != 0) {
				(*_handleError)();
			}
			if(isTransmitDone()) {
				_clearTransmitStatus();
				if(_handleSent != nullptr)
					(*_handleSent)();
			}
			if(isReceiveTimestampAvailable()) {
				_clearReceiveTimestampAvailableStatus();
				if(_handleReceiveTimestampAvailable != nullptr)
					(*_handleReceiveTimestampAvailable)();
			}
			if(isReceiveFailed()) {
				_clearReceiveFailedStatus();
				forceTRxOff();
				_resetReceiver();
				if(_handleReceiveFailed != nullptr)
					(*_handleReceiveFailed)();
			} else if(isReceiveTimeout()) {
				_clearReceiveTimeoutStatus();
				forceTRxOff();
				_resetReceiver();
				if(_handleReceiveTimeout != nullptr)
					(*_handleReceiveTimeout)();
			} else if(isReceiveDone()) {
				_clearReceiveStatus();
				if(_handleReceived != nullptr)
					(*_handleReceived)();
			}
		}

		void _setInterruptPolarity(boolean val) {
			DWM1000Utils::setBit(_syscfg, LEN_SYS_CFG, HIRQ_POL_BIT, val);
		}

		void _disableSequencing() {
            _enableClock(SYS_XTI_CLOCK);
            byte zero[2];
            DWM1000Utils::writeValueToBytes(zero, 0x0000, 2);
            writeBytes(PMSC, PMSC_CTRL1_SUB, zero, 2); // To re-enable write 0xE7
        }

        void _configureRFTransmitPowerSpectrumTestMode() {
			/* Enabled TXFEN, PLLFEN, LDOFEN and set TXRXSW to TX */
            byte enable_mask[4];
            DWM1000Utils::writeValueToBytes(enable_mask, 0x005FFF00, LEN_RX_CONF_SUB);
            writeBytes(RF_CONF, RF_CONF_SUB, enable_mask, LEN_RX_CONF_SUB);
        }
	}

	/* ####################### PUBLIC ###################### */

	void begin(uint8_t irq, uint8_t rst) {
		// generous initial init/wake-up-idle delay
		delay(5);
		// start SPI
		SPI.begin();
		SPI.usingInterrupt(digitalPinToInterrupt(irq));
		// pin and basic member setup
		_rst        = rst;
		_irq        = irq;
		// attach interrupt
		//attachInterrupt(_irq, _handleInterrupt, CHANGE);
		// TODO throw error if pin is not a interrupt pin
		attachInterrupt(digitalPinToInterrupt(_irq), _handleInterrupt, RISING);
	}

	void select(uint8_t ss) {
		reselect(ss);
		// try locking clock at PLL speed (should be done already,
		// but just to be sure)
		_enableClock(SYS_AUTO_CLOCK);
		delay(5);
		// reset chip (either soft or hard)
		if(_rst != 0xff) {
			// DWM1000 data sheet v2.08 §5.6.1 page 20, the RSTn pin should not be driven high but left floating.
			pinMode(_rst, INPUT);
		}
		reset();
		// default network and node id
		DWM1000Utils::writeValueToBytes(_networkAndAddress, 0xFF, LEN_PANADR);
		writeNetworkIdAndDeviceAddress();
		// default system configuration
		memset(_syscfg, 0, LEN_SYS_CFG);
		setDoubleBuffering(false);
		_setInterruptPolarity(true);
		_writeSystemConfigurationRegister();
		// default interrupt mask, i.e. no interrupts
		_clearInterrupts();
		_writeSystemEventMaskRegister();
		// load LDE micro-code
		_enableClock(SYS_XTI_CLOCK);
		delay(5);
		_manageLDE();
		delay(5);
		_enableClock(SYS_AUTO_CLOCK);
		delay(5);
		
		// read the temp and vbat readings from OTP that were recorded during production test
		// see 6.3.1 OTP memory map
		byte buf_otp[4];
		readBytesOTP(0x008, buf_otp); // the stored 3.3 V reading
		_vmeas3v3 = buf_otp[0];
		readBytesOTP(0x009, buf_otp); // the stored 23C reading
		_tmeas23C = buf_otp[0];
	}

	void reselect(uint8_t ss) {
		_ss = ss;
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

	void enableDebounceClock() {
		byte pmscctrl0[LEN_PMSC_CTRL0];
		memset(pmscctrl0, 0, LEN_PMSC_CTRL0);
		readBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
		DWM1000Utils::setBit(pmscctrl0, LEN_PMSC_CTRL0, GPDCE_BIT, 1);
		DWM1000Utils::setBit(pmscctrl0, LEN_PMSC_CTRL0, KHZCLKEN_BIT, 1);
		writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
			_debounceClockEnabled = true;
	}

	void enableLedBlinking() {
		byte pmscledc[LEN_PMSC_LEDC];
		memset(pmscledc, 0, LEN_PMSC_LEDC);
		readBytes(PMSC, PMSC_LEDC_SUB, pmscledc, LEN_PMSC_LEDC);
		DWM1000Utils::setBit(pmscledc, LEN_PMSC_LEDC, BLNKEN, 1);
		writeBytes(PMSC, PMSC_LEDC_SUB, pmscledc, LEN_PMSC_LEDC);
	}

	void setGPIOMode(uint8_t msgp, uint8_t mode) {
		byte gpiomode[LEN_GPIO_MODE];
		memset(gpiomode, 0, LEN_GPIO_MODE);
		readBytes(GPIO_CTRL, GPIO_MODE_SUB, gpiomode, LEN_GPIO_MODE);
		for (char i = 0; i < 2; i++){
			DWM1000Utils::setBit(gpiomode, LEN_GPIO_MODE, msgp + i, (mode >> i) & 1);
		}
		writeBytes(GPIO_CTRL, GPIO_MODE_SUB, gpiomode, LEN_GPIO_MODE);
	}

	void deepSleep() {
		byte aon_wcfg[LEN_AON_WCFG];
		memset(aon_wcfg, 0, LEN_AON_WCFG);
		readBytes(AON, AON_WCFG_SUB, aon_wcfg, LEN_AON_WCFG);
		DWM1000Utils::setBit(aon_wcfg, LEN_AON_WCFG, ONW_LDC_BIT, true);
		DWM1000Utils::setBit(aon_wcfg, LEN_AON_WCFG, ONW_LDD0_BIT, true);
		writeBytes(AON, AON_WCFG_SUB, aon_wcfg, LEN_AON_WCFG);

		byte pmsc_ctrl1[LEN_PMSC_CTRL1];
		memset(pmsc_ctrl1, 0, LEN_PMSC_CTRL1);
		readBytes(PMSC, PMSC_CTRL1_SUB, pmsc_ctrl1, LEN_PMSC_CTRL1);
		DWM1000Utils::setBit(pmsc_ctrl1, LEN_PMSC_CTRL1, ATXSLP_BIT, false);
		DWM1000Utils::setBit(pmsc_ctrl1, LEN_PMSC_CTRL1, ARXSLP_BIT, false);
		writeBytes(PMSC, PMSC_CTRL1_SUB, pmsc_ctrl1, LEN_PMSC_CTRL1);

		byte aon_cfg0[LEN_AON_CFG0];
		memset(aon_cfg0, 0, LEN_AON_CFG0);
		readBytes(AON, AON_CFG0_SUB, aon_cfg0, LEN_AON_CFG0);
		DWM1000Utils::setBit(aon_cfg0, LEN_AON_CFG0, WAKE_SPI_BIT, true);
		DWM1000Utils::setBit(aon_cfg0, LEN_AON_CFG0, WAKE_PIN_BIT, true);
		DWM1000Utils::setBit(aon_cfg0, LEN_AON_CFG0, WAKE_CNT_BIT, false);
		DWM1000Utils::setBit(aon_cfg0, LEN_AON_CFG0, SLEEP_EN_BIT, true);
		writeBytes(AON, AON_CFG0_SUB, aon_cfg0, LEN_AON_CFG0);

		byte aon_ctrl[LEN_AON_CTRL];
		memset(aon_ctrl, 0, LEN_AON_CTRL);
		readBytes(AON, AON_CTRL_SUB, aon_ctrl, LEN_AON_CTRL);
		DWM1000Utils::setBit(aon_ctrl, LEN_AON_CTRL, UPL_CFG_BIT, true);
		DWM1000Utils::setBit(aon_ctrl, LEN_AON_CTRL, SAVE_BIT, true);
		writeBytes(AON, AON_CTRL_SUB, aon_ctrl, LEN_AON_CTRL);
	}

	void spiWakeup(){
			digitalWrite(_ss, LOW);
			delay(2);
			digitalWrite(_ss, HIGH);
			if (_debounceClockEnabled){
					enableDebounceClock();
			}
	}


	void reset() {
		if(_rst == 0xff) {
			softReset();
		} else {
			// DWM1000 data sheet v2.08 §5.6.1 page 20, the RSTn pin should not be driven high but left floating.
			pinMode(_rst, OUTPUT);
			digitalWrite(_rst, LOW);
			delay(2);  // DWM1000 data sheet v2.08 §5.6.1 page 20: nominal 50ns, to be safe take more time
			pinMode(_rst, INPUT);
			delay(10); // dwm1000 data sheet v1.2 page 5: nominal 3 ms, to be safe take more time
			// force into idle mode (although it should be already after reset)
			forceTRxOff();
		}
	}

	void softReset() {
		byte pmscctrl0[LEN_PMSC_CTRL0];
		readBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
		pmscctrl0[0] = 0x01;
		writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
		pmscctrl0[3] = 0x00;
		writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
		delay(10);
		pmscctrl0[0] = 0x00;
		pmscctrl0[3] = 0xF0;
		writeBytes(PMSC, PMSC_CTRL0_SUB, pmscctrl0, LEN_PMSC_CTRL0);
		// force into idle mode
		forceTRxOff();
	}

	void enableMode(const byte mode[]) {
		setDataRate(mode[0]);
		setPulseFrequency(mode[1]);
		setPreambleLength(mode[2]);
	}

	/* ###########################################################################
	* #### Pretty printed device information ####################################
	* ######################################################################### */


	void getPrintableDeviceIdentifier(char msgBuffer[]) {
		byte data[LEN_DEV_ID];
		readBytes(DEV_ID, NO_SUB, data, LEN_DEV_ID);
		sprintf(msgBuffer, "%02X - model: %d, version: %d, revision: %d",
						(uint16_t)((data[3] << 8) | data[2]), data[1], (data[0] >> 4) & 0x0F, data[0] & 0x0F);
	}

	void getPrintableExtendedUniqueIdentifier(char msgBuffer[]) {
		byte data[LEN_EUI];
		readBytes(EUI, NO_SUB, data, LEN_EUI);
		sprintf(msgBuffer, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
						data[7], data[6], data[5], data[4], data[3], data[2], data[1], data[0]);
	}

	void getPrintableNetworkIdAndShortAddress(char msgBuffer[]) {
		byte data[LEN_PANADR];
		readBytes(PANADR, NO_SUB, data, LEN_PANADR);
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
		readBytes(CHAN_CTRL, NO_SUB, chan_ctrl, LEN_CHAN_CTRL);
		readBytes(TX_FCTRL, NO_SUB, tx_fctrl, LEN_TX_FCTRL);
		/* Data Rate from 0x08 bits:13-14(tx_fctrl) */
		dr = (uint16_t)(tx_fctrl[1] >> 5 & 0x3);
		switch(dr) {
			case TRX_RATE_110KBPS:
				dr = 110;
				break;
			case TRX_RATE_850KBPS:
				dr = 850;
				break;
			case TRX_RATE_6800KBPS:
				dr = 6800;
				break;
			default:
				return; //TODO Error handling
		}
		/* PRF(16 or 64) from 0x1F bits:18-19(chan_ctrl) */
		prf = (uint8_t)(chan_ctrl[2] >> 2 & 0x03);
		if(prf == TX_PULSE_FREQ_16MHZ){
			prf = 16;
		} else if(prf == TX_PULSE_FREQ_64MHZ){
			prf = 64;
		} else{
			return; //TODO Error handling
		}
		/* PreambleLength from 0x08 bits:18-21(tx_fctrl) */
		plen = (uint16_t)(tx_fctrl[2] >> 2 & 0xF);
		switch(plen) {
			case TX_PREAMBLE_LEN_64:
				plen = 64;
				break;
			case TX_PREAMBLE_LEN_128:
				plen = 128;
				break;
			case TX_PREAMBLE_LEN_256:
				plen = 256;
				break;
			case TX_PREAMBLE_LEN_512:
				plen = 512;
				break;
			case TX_PREAMBLE_LEN_1024:
				plen = 1024;
				break;
			case TX_PREAMBLE_LEN_1536:
				plen = 1536;
				break;
			case TX_PREAMBLE_LEN_2048:
				plen = 2048;
				break;
			case TX_PREAMBLE_LEN_4096:
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
	* #### DWM1000 operation functions ###########################################
	* ######################################################################### */

	void setNetworkId(uint16_t val) {
		_networkAndAddress[2] = (byte)(val & 0xFF);
		_networkAndAddress[3] = (byte)((val >> 8) & 0xFF);
	}

	void setDeviceAddress(uint16_t val) {
		_networkAndAddress[0] = (byte)(val & 0xFF);
		_networkAndAddress[1] = (byte)((val >> 8) & 0xFF);
	}

	void writeNetworkIdAndDeviceAddress() {
		writeBytes(PANADR, NO_SUB, _networkAndAddress, LEN_PANADR);
	}

	void getTemp(float& temp) {
		_vbatAndTempSteps();
		byte sar_ltemp = 0; readBytes(TX_CAL, 0x04, &sar_ltemp, 1);
		temp = (sar_ltemp - _tmeas23C) * 1.14f + 23.0f;
	}


	void getVbat(float& vbat) {
		_vbatAndTempSteps();
		byte sar_lvbat = 0; readBytes(TX_CAL, 0x03, &sar_lvbat, 1);
		vbat = (sar_lvbat - _vmeas3v3) / 173.0f + 3.3f;
	}

	void getTempAndVbat(float& temp, float& vbat) {
		// follow the procedure from section 6.4 of the User Manual
		_vbatAndTempSteps();
		byte sar_lvbat = 0; readBytes(TX_CAL, 0x03, &sar_lvbat, 1);
		byte sar_ltemp = 0; readBytes(TX_CAL, 0x04, &sar_ltemp, 1);
		
		// calculate voltage and temperature
		vbat = (sar_lvbat - _vmeas3v3) / 173.0f + 3.3f;
		temp = (sar_ltemp - _tmeas23C) * 1.14f + 23.0f;
	}

	void setEUI(char eui[]) {
		byte eui_byte[LEN_EUI];
		DWM1000Utils::convertToByte(eui, eui_byte);
		setEUI(eui_byte);
	}

	void setEUI(byte eui[]) {
		//we reverse the address->
		byte    reverseEUI[8];
		uint8_t     size = 8;
		for(uint8_t i    = 0; i < size; i++) {
			*(reverseEUI+i) = *(eui+size-i-1);
		}
		writeBytes(EUI, NO_SUB, reverseEUI, LEN_EUI);
	}


	//Frame Filtering BIT in the SYS_CFG register
	void setFrameFilter(boolean val) {
		DWM1000Utils::setBit(_syscfg, LEN_SYS_CFG, FFEN_BIT, val);
	}

	void setFrameFilterBehaveCoordinator(boolean val) {
		DWM1000Utils::setBit(_syscfg, LEN_SYS_CFG, FFBC_BIT, val);
	}

	void setFrameFilterAllowBeacon(boolean val) {
		DWM1000Utils::setBit(_syscfg, LEN_SYS_CFG, FFAB_BIT, val);
	}

	void setFrameFilterAllowData(boolean val) {
		DWM1000Utils::setBit(_syscfg, LEN_SYS_CFG, FFAD_BIT, val);
	}

	void setFrameFilterAllowAcknowledgement(boolean val) {
		DWM1000Utils::setBit(_syscfg, LEN_SYS_CFG, FFAA_BIT, val);
	}

	void setFrameFilterAllowMAC(boolean val) {
		DWM1000Utils::setBit(_syscfg, LEN_SYS_CFG, FFAM_BIT, val);
	}

	void setFrameFilterAllowReserved(boolean val) {
		DWM1000Utils::setBit(_syscfg, LEN_SYS_CFG, FFAR_BIT, val);
	}


	void setDoubleBuffering(boolean val) {
		DWM1000Utils::setBit(_syscfg, LEN_SYS_CFG, DIS_DRXB_BIT, !val);
	}

	void setReceiverAutoReenable(boolean val) {
		DWM1000Utils::setBit(_syscfg, LEN_SYS_CFG, RXAUTR_BIT, val);
	}

	void interruptOnSent(boolean val) {
		DWM1000Utils::setBit(_sysmask, LEN_SYS_MASK, TXFRS_BIT, val);
	}

	void interruptOnReceived(boolean val) {
		DWM1000Utils::setBit(_sysmask, LEN_SYS_MASK, RXDFR_BIT, val);
		DWM1000Utils::setBit(_sysmask, LEN_SYS_MASK, RXFCG_BIT, val);
	}

	void interruptOnReceiveFailed(boolean val) {
		DWM1000Utils::setBit(_sysmask, LEN_SYS_STATUS, LDEERR_BIT, val);
		DWM1000Utils::setBit(_sysmask, LEN_SYS_STATUS, RXFCE_BIT, val);
		DWM1000Utils::setBit(_sysmask, LEN_SYS_STATUS, RXPHE_BIT, val);
		DWM1000Utils::setBit(_sysmask, LEN_SYS_STATUS, RXRFSL_BIT, val);
	}

	void interruptOnReceiveTimeout(boolean val) {
		DWM1000Utils::setBit(_sysmask, LEN_SYS_MASK, RXRFTO_BIT, val);
	}

	void interruptOnReceiveTimestampAvailable(boolean val) {
		DWM1000Utils::setBit(_sysmask, LEN_SYS_MASK, LDEDONE_BIT, val);
	}

	void interruptOnAutomaticAcknowledgeTrigger(boolean val) {
		DWM1000Utils::setBit(_sysmask, LEN_SYS_MASK, AAT_BIT, val);
	}

	void setAntennaDelay(const uint16_t value) {
		_antennaDelay.setTimestamp(value);
	}

	uint16_t getAntennaDelay() {
		return static_cast<uint16_t>(_antennaDelay.getTimestamp());
	}

	void forceTRxOff() {
		memset(_sysctrl, 0, LEN_SYS_CTRL);
		DWM1000Utils::setBit(_sysctrl, LEN_SYS_CTRL, TRXOFF_BIT, true);
		writeBytes(SYS_CTRL, NO_SUB, _sysctrl, LEN_SYS_CTRL);
	}

	void startReceive(ReceiveMode mode) {
		memset(_sysctrl, 0, LEN_SYS_CTRL);
		DWM1000Utils::setBit(_sysctrl, LEN_SYS_CTRL, SFCST_BIT, !_frameCheck);
		if(mode == ReceiveMode::DELAYED)
			DWM1000Utils::setBit(_sysctrl, LEN_SYS_CTRL, RXDLYS_BIT, true);
		DWM1000Utils::setBit(_sysctrl, LEN_SYS_CTRL, RXENAB_BIT, true);
		writeBytes(SYS_CTRL, NO_SUB, _sysctrl, LEN_SYS_CTRL);
	}

	void startTransmit(TransmitMode mode) {
		memset(_sysctrl, 0, LEN_SYS_CTRL);
		_clearTransmitStatus();
		_writeTransmitFrameControlRegister();
		DWM1000Utils::setBit(_sysctrl, LEN_SYS_CTRL, SFCST_BIT, !_frameCheck);
		if(mode == TransmitMode::DELAYED)
			DWM1000Utils::setBit(_sysctrl, LEN_SYS_CTRL, TXDLYS_BIT, true);
		DWM1000Utils::setBit(_sysctrl, LEN_SYS_CTRL, TXSTRT_BIT, true);
		writeBytes(SYS_CTRL, NO_SUB, _sysctrl, LEN_SYS_CTRL);
	}

	void newConfiguration() {
		forceTRxOff();
		_readNetworkIdAndDeviceAddress();
		_readSystemConfigurationRegister();
		_readChannelControlRegister();
		_readTransmitFrameControlRegister();
		_readSystemEventMaskRegister();
	}

	void commitConfiguration() {
		// writes configuration to registers
		_writeConfiguration();
		// tune according to configuration
		_tune();
	}

	void waitForResponse(boolean val) {
		DWM1000Utils::setBit(_sysctrl, LEN_SYS_CTRL, WAIT4RESP_BIT, val);
	}

	void suppressFrameCheck(boolean val) {
		_frameCheck = !val;
	}

	void setNLOS(boolean val) {
		_nlos = val;
	}

	void useSmartPower(boolean smartPower) {
		_smartPower = smartPower;
		DWM1000Utils::setBit(_syscfg, LEN_SYS_CFG, DIS_STXP_BIT, !smartPower);
		if(_smartPower) 
			_autoTXPower = true;
	}

	void setTXPower(int32_t power) {
		byte txpower[LEN_TX_POWER];
		DWM1000Utils::writeValueToBytes(txpower, power, LEN_TX_POWER);
		writeBytes(TX_POWER, NO_SUB, txpower, LEN_TX_POWER);
		useSmartPower(false);
		_autoTXPower = false;
	}

	void setTXPower(DriverAmplifierValue driver_amplifier, TransmitMixerValue mixer) {
		byte txpower[LEN_TX_POWER];
		byte pwr = 0x00;

		pwr |= ((byte) driver_amplifier << 5);
		pwr |= (byte) mixer;

		for(auto i = 0; i < LEN_TX_POWER; i++) {
			txpower[i] = pwr;
		}

		writeBytes(TX_POWER, NO_SUB, txpower, LEN_TX_POWER);
		useSmartPower(false);
		_autoTXPower = false;
	}

	void setTXPowerAuto() {
		_autoTXPower = true;
	}

	void setTCPGDelay(uint8_t tcpgdelay) {
		byte tcpgBytes[LEN_TC_PGDELAY];
		DWM1000Utils::writeValueToBytes(tcpgBytes, tcpgdelay, LEN_TC_PGDELAY);
		writeBytes(TX_CAL, TC_PGDELAY_SUB, tcpgBytes, LEN_TC_PGDELAY);
		_autoTCPGDelay = false;
	}

	void setTCPGDelayAuto() {
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
        DWM1000Utils::writeValueToBytes(delayBytes, repeat_interval, 4);
        writeBytes(DX_TIME, NO_SUB, delayBytes, 4);

		/* Enable Transmit Power Spectrum Test Mode */
        byte diagnosticBytes[2];
        DWM1000Utils::writeValueToBytes(diagnosticBytes, 0x0010, LEN_DIAG_TMC);
        writeBytes(DIG_DIAG, DIAG_TMC_SUB, diagnosticBytes, LEN_DIAG_TMC);
    }

	DWM1000Time setDelay(uint16_t delayUS) {
		byte delayBytes[LEN_DX_TIME];
		DWM1000Time futureTime;
		DWM1000Time delayTime = DWM1000Time(delayUS, DWM1000Time::MICROSECONDS);
		getSystemTimestamp(futureTime);
		futureTime += delayTime;
		futureTime.getTimestamp(delayBytes);
		/* the least significant 9-bits are ignored in DX_TIME in functional modes */
		delayBytes[0] = 0;
		delayBytes[1] &= 0xFE;
		writeBytes(DX_TIME, NO_SUB, delayBytes, LEN_DX_TIME);
		// adjust expected time with configured antenna delay
		futureTime.setTimestamp(delayBytes);
		futureTime += _antennaDelay;
		return futureTime;
	}


	void setDataRate(byte rate) {
		rate &= 0x03;
		_txfctrl[1] &= 0x83;
		_txfctrl[1] |= (byte)((rate << 5) & 0xFF);
		// special 110kbps flag
		if(rate == TRX_RATE_110KBPS) {
			DWM1000Utils::setBit(_syscfg, LEN_SYS_CFG, RXM110K_BIT, true);
		} else {
			DWM1000Utils::setBit(_syscfg, LEN_SYS_CFG, RXM110K_BIT, false);
		}
		_dataRate = rate;
		(*_currentSFDMode)();
	}

	void setPulseFrequency(byte freq) {
		freq &= 0x03;
		_txfctrl[2] &= 0xFC;
		_txfctrl[2] |= (byte)(freq & 0xFF);
		_chanctrl[2] &= 0xF3;
		_chanctrl[2] |= (byte)((freq << 2) & 0xFF);
		_pulseFrequency = freq;
		setPreambleCode();
	}

	byte getPulseFrequency() {
		return _pulseFrequency;
	}

	void setPreambleLength(byte prealen) {
		prealen &= 0x0F;
		_txfctrl[2] &= 0xC3;
		_txfctrl[2] |= (byte)((prealen << 2) & 0xFF);
		
		switch(prealen) {
			case TX_PREAMBLE_LEN_64:
				_pacSize = PAC_SIZE_8;
				break;
			case TX_PREAMBLE_LEN_128:
				_pacSize = PAC_SIZE_8;
				break;
			case TX_PREAMBLE_LEN_256:
				_pacSize = PAC_SIZE_16;
				break;
			case TX_PREAMBLE_LEN_512:
				_pacSize = PAC_SIZE_16;
				break;
			case TX_PREAMBLE_LEN_1024:
				_pacSize = PAC_SIZE_32;
				break;
			default:
				_pacSize = PAC_SIZE_64; // In case of 1536, 2048 or 4096 preamble length.
		}
		
		_preambleLength = prealen;
	}

	void setSFDMode(SFDMode mode) {
		switch(mode) {
			case SFDMode::STANDARD_SFD:
				_currentSFDMode = _useStandardSFD;
				break;
			case SFDMode::DECAWAVE_SFD:
				_currentSFDMode = _useDecawaveSFD;
				break;
			case SFDMode::RECOMMENDED_SFD:
				_currentSFDMode = _useRecommendedSFD;
				break;
			default:
				return; //TODO Proper error handling
		}

		/* Sets new SFD parameters by calling the relative function */
		(*_currentSFDMode)();
	}

	void useExtendedFrameLength(boolean val) {
		_extendedFrameLength = (val ? FRAME_LENGTH_EXTENDED : FRAME_LENGTH_NORMAL);
		_syscfg[2] &= 0xFC;
		_syscfg[2] |= _extendedFrameLength;
	}

	void setChannel(byte channel) {
		channel &= 0xF;
		_chanctrl[0] = ((channel | (channel << 4)) & 0xFF);
		_channel = channel;
		setPreambleCode();
	}

	static boolean checkPreambleCodeValidity(byte preamble_code) {
		if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
			for (auto i = 0; i < 2; i++) {
				if(preamble_code == preamble_validity_matrix_PRF16[(int) _channel][i])
					return true;
			}
			return false;
		} else if (_pulseFrequency == TX_PULSE_FREQ_64MHZ) {
			for(auto i = 0; i < 4; i++) {
				if(preamble_code == preamble_validity_matrix_PRF64[(int) _channel][i])
					return true;
			}
			return false;
		} else {
			return false; //TODO Proper error handling
		}
	}

	void setPreambleCode() {
		if(checkPreambleCodeValidity(_preambleCode)) 
			return;
		
		byte preacode;

		switch(_channel) {
			case CHANNEL_1:
				preacode = _pulseFrequency == TX_PULSE_FREQ_16MHZ ? PREAMBLE_CODE_16MHZ_2 : PREAMBLE_CODE_64MHZ_10;
				break;
			case CHANNEL_3:
				preacode = _pulseFrequency == TX_PULSE_FREQ_16MHZ ? PREAMBLE_CODE_16MHZ_6 : PREAMBLE_CODE_64MHZ_10;
				break;
			case CHANNEL_4:
			case CHANNEL_7:
				preacode = _pulseFrequency == TX_PULSE_FREQ_16MHZ ? PREAMBLE_CODE_16MHZ_8 : PREAMBLE_CODE_64MHZ_18;
				break;
			case CHANNEL_2:
			case CHANNEL_5:
				preacode = _pulseFrequency == TX_PULSE_FREQ_16MHZ ? PREAMBLE_CODE_16MHZ_4 : PREAMBLE_CODE_64MHZ_10;
				break;
			default:
				return; //TODO Proper Error Handling
		}

		preacode &= 0x1F;
		_chanctrl[2] &= 0x3F;
		_chanctrl[2] |= ((preacode << 6) & 0xFF);
		_chanctrl[3] = 0x00;
		_chanctrl[3] = ((((preacode >> 2) & 0x07) | (preacode << 3)) & 0xFF);
		_preambleCode = preacode;
	}

	void setPreambleCode(byte preacode) {
		if(checkPreambleCodeValidity(preacode)) {
			preacode &= 0x1F;
			_chanctrl[2] &= 0x3F;
			_chanctrl[2] |= ((preacode << 6) & 0xFF);
			_chanctrl[3] = 0x00;
			_chanctrl[3] = ((((preacode >> 2) & 0x07) | (preacode << 3)) & 0xFF);
			_preambleCode = preacode;
		} else {
			return; //TODO Proper error handling
		}
	}

	void setDefaults() {
			useExtendedFrameLength(false);
			useSmartPower(true);
			suppressFrameCheck(false);
			//for global frame filtering
			setFrameFilter(false);
			/* old defaults with active frame filter - better set filter in every script where you really need it
			setFrameFilter(true);
			//for data frame (poll, poll_ack, range, range report, range failed) filtering
			setFrameFilterAllowData(true);
			//for reserved (blink) frame filtering
			setFrameFilterAllowReserved(true);
			//setFrameFilterAllowMAC(true);
			//setFrameFilterAllowBeacon(true);
			//setFrameFilterAllowAcknowledgement(true);
			*/
			interruptOnSent(true);
			interruptOnReceived(true);
			interruptOnReceiveFailed(true);
			interruptOnReceiveTimestampAvailable(false);
			interruptOnAutomaticAcknowledgeTrigger(true);
			// TODO add channel and code to mode tuples
			// TODO add channel and code settings with checks (see DWM1000 user manual 10.5 table 61)/
			setChannel(CHANNEL_5);
			setAntennaDelay(16384);
			// default mode when powering up the chip
			// still explicitly selected for later tuning
			enableMode(MODE_SHORTRANGE_LOWPRF_MEDIUMPREAMBLE);
	}

	void setData(byte data[], uint16_t n) {
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
		writeBytes(TX_BUFFER, NO_SUB, data, n);
		_txfctrl[0] = (byte)(n & 0xFF); // 1 byte (regular length + 1 bit)
		_txfctrl[1] &= 0xE0;
		_txfctrl[1] |= (byte)((n >> 8) & 0x03);  // 2 added bits if extended length
	}

	void setData(const String& data) {
		uint16_t n = data.length()+1;
		byte* dataBytes = (byte*)malloc(n);
		data.getBytes(dataBytes, n);
		setData(dataBytes, n);
		free(dataBytes);
	}

	// TODO reorder
	uint16_t getDataLength() {
		uint16_t len = 0;

		// 10 bits of RX frame control register
		byte rxFrameInfo[LEN_RX_FINFO];
		readBytes(RX_FINFO, NO_SUB, rxFrameInfo, LEN_RX_FINFO);
		len = ((((uint16_t)rxFrameInfo[1] << 8) | (uint16_t)rxFrameInfo[0]) & 0x03FF);
		
		if(_frameCheck && len > 2) {
			return len-2;
		}
		return len;
	}

	void getData(byte data[], uint16_t n) {
		if(n <= 0) {
			return;
		}
		readBytes(RX_BUFFER, NO_SUB, data, n);
	}

	void getData(String& data) {
		uint16_t i;
		uint16_t n = getDataLength(); // number of bytes w/o the two FCS ones
		if(n <= 0) { // TODO
			return;
		}
		byte* dataBytes = (byte*)malloc(n);
		getData(dataBytes, n);
		// clear string
		data.remove(0);
		data  = "";
		// append to string
		for(i = 0; i < n; i++) {
			data += (char)dataBytes[i];
		}
		free(dataBytes);
	}

	void getTransmitTimestamp(DWM1000Time& time) {
		byte txTimeBytes[LEN_TX_STAMP];
		readBytes(TX_TIME, TX_STAMP_SUB, txTimeBytes, LEN_TX_STAMP);
		time.setTimestamp(txTimeBytes);
	}

	void getReceiveTimestamp(DWM1000Time& time) {
		byte rxTimeBytes[LEN_RX_STAMP];
		readBytes(RX_TIME, RX_STAMP_SUB, rxTimeBytes, LEN_RX_STAMP);
		time.setTimestamp(rxTimeBytes);
		// correct timestamp (i.e. consider range bias)
		correctTimestamp(time);
	}

	// TODO check function, different type violations between byte and int
	void correctTimestamp(DWM1000Time& timestamp) {
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
		if(_channel == CHANNEL_4 || _channel == CHANNEL_7) {
			// 900 MHz receiver bandwidth
			if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
				rangeBiasHigh = (rxPowerBaseHigh < BIAS_900_16_ZERO ? -BIAS_900_16[rxPowerBaseHigh] : BIAS_900_16[rxPowerBaseHigh]);
				rangeBiasHigh <<= 1;
				rangeBiasLow  = (rxPowerBaseLow < BIAS_900_16_ZERO ? -BIAS_900_16[rxPowerBaseLow] : BIAS_900_16[rxPowerBaseLow]);
				rangeBiasLow <<= 1;
			} else if(_pulseFrequency == TX_PULSE_FREQ_64MHZ) {
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
			if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
				rangeBiasHigh = (rxPowerBaseHigh < BIAS_500_16_ZERO ? -BIAS_500_16[rxPowerBaseHigh] : BIAS_500_16[rxPowerBaseHigh]);
				rangeBiasLow  = (rxPowerBaseLow < BIAS_500_16_ZERO ? -BIAS_500_16[rxPowerBaseLow] : BIAS_500_16[rxPowerBaseLow]);
			} else if(_pulseFrequency == TX_PULSE_FREQ_64MHZ) {
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
		DWM1000Time adjustmentTime;
		adjustmentTime.setTimestamp((int16_t)(rangeBias*DWM1000Time::DISTANCE_OF_RADIO_INV*0.001f));
		// apply correction
		timestamp -= adjustmentTime;
	}

	void getSystemTimestamp(DWM1000Time& time) {
		byte sysTimeBytes[LEN_SYS_TIME];
		readBytes(SYS_TIME, NO_SUB, sysTimeBytes, LEN_SYS_TIME);
		time.setTimestamp(sysTimeBytes);
	}

	void getTransmitTimestamp(byte data[]) {
		readBytes(TX_TIME, TX_STAMP_SUB, data, LEN_TX_STAMP);
	}

	void getReceiveTimestamp(byte data[]) {
		readBytes(RX_TIME, RX_STAMP_SUB, data, LEN_RX_STAMP);
	}

	void getSystemTimestamp(byte data[]) {
		readBytes(SYS_TIME, NO_SUB, data, LEN_SYS_TIME);
	}

	boolean isTransmitDone() {
		return DWM1000Utils::getBit(_sysstatus, LEN_SYS_STATUS, TXFRS_BIT);
	}

	boolean isReceiveTimestampAvailable() {
		return DWM1000Utils::getBit(_sysstatus, LEN_SYS_STATUS, LDEDONE_BIT);
	}

	boolean isReceiveDone() {
		if(_frameCheck) {
			return DWM1000Utils::getBit(_sysstatus, LEN_SYS_STATUS, RXFCG_BIT);
		}
		return DWM1000Utils::getBit(_sysstatus, LEN_SYS_STATUS, RXDFR_BIT);
	}

	boolean isReceiveFailed() {
		boolean ldeErr, rxCRCErr, rxHeaderErr, rxDecodeErr;
		ldeErr      = DWM1000Utils::getBit(_sysstatus, LEN_SYS_STATUS, LDEERR_BIT);
		rxCRCErr    = DWM1000Utils::getBit(_sysstatus, LEN_SYS_STATUS, RXFCE_BIT);
		rxHeaderErr = DWM1000Utils::getBit(_sysstatus, LEN_SYS_STATUS, RXPHE_BIT);
		rxDecodeErr = DWM1000Utils::getBit(_sysstatus, LEN_SYS_STATUS, RXRFSL_BIT);
		if(ldeErr || rxCRCErr || rxHeaderErr || rxDecodeErr) {
			return true;
		}
		return false;
	}

	//Checks to see any of the three timeout bits in sysstatus are high (RXRFTO (Frame Wait timeout), RXPTO (Preamble timeout), RXSFDTO (Start frame delimiter(?) timeout).
	boolean isReceiveTimeout() {
		return (DWM1000Utils::getBit(_sysstatus, LEN_SYS_STATUS, RXRFTO_BIT) | DWM1000Utils::getBit(_sysstatus, LEN_SYS_STATUS, RXPTO_BIT) | DWM1000Utils::getBit(_sysstatus, LEN_SYS_STATUS, RXSFDTO_BIT));
	}

	boolean isClockProblem() {
		boolean clkllErr, rfllErr;
		clkllErr = DWM1000Utils::getBit(_sysstatus, LEN_SYS_STATUS, CLKPLL_LL_BIT);
		rfllErr  = DWM1000Utils::getBit(_sysstatus, LEN_SYS_STATUS, RFPLL_LL_BIT);
		if(clkllErr || rfllErr) {
			return true;
		}
		return false;
	}

	float getReceiveQuality() {
		byte         noiseBytes[LEN_STD_NOISE];
		byte         fpAmpl2Bytes[LEN_FP_AMPL2];
		uint16_t     noise, f2;
		readBytes(RX_FQUAL, STD_NOISE_SUB, noiseBytes, LEN_STD_NOISE);
		readBytes(RX_FQUAL, FP_AMPL2_SUB, fpAmpl2Bytes, LEN_FP_AMPL2);
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
		readBytes(RX_TIME, FP_AMPL1_SUB, fpAmpl1Bytes, LEN_FP_AMPL1);
		readBytes(RX_FQUAL, FP_AMPL2_SUB, fpAmpl2Bytes, LEN_FP_AMPL2);
		readBytes(RX_FQUAL, FP_AMPL3_SUB, fpAmpl3Bytes, LEN_FP_AMPL3);
		readBytes(RX_FINFO, NO_SUB, rxFrameInfo, LEN_RX_FINFO);
		f1 = (uint16_t)fpAmpl1Bytes[0] | ((uint16_t)fpAmpl1Bytes[1] << 8);
		f2 = (uint16_t)fpAmpl2Bytes[0] | ((uint16_t)fpAmpl2Bytes[1] << 8);
		f3 = (uint16_t)fpAmpl3Bytes[0] | ((uint16_t)fpAmpl3Bytes[1] << 8);
		N  = (((uint16_t)rxFrameInfo[2] >> 4) & 0xFF) | ((uint16_t)rxFrameInfo[3] << 4);

		if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
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
		readBytes(RX_FQUAL, CIR_PWR_SUB, cirPwrBytes, LEN_CIR_PWR);
		readBytes(RX_FINFO, NO_SUB, rxFrameInfo, LEN_RX_FINFO);
		C = (uint16_t)cirPwrBytes[0] | ((uint16_t)cirPwrBytes[1] << 8);
		N = (((uint16_t)rxFrameInfo[2] >> 4) & 0xFF) | ((uint16_t)rxFrameInfo[3] << 4);

		if(_pulseFrequency == TX_PULSE_FREQ_16MHZ) {
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

	/* ###########################################################################
	* #### Helper functions #####################################################
	* ######################################################################### */

	/*
	* Read bytes from the DWM1000. Number of bytes depend on register length.
	* @param cmd
	* 		The register address (see Chapter 7 in the DWM1000 user manual).
	* @param data
	*		The data array to be read into.
	* @param n
	*		The number of bytes expected to be received.
	*/
	// TODO incomplete doc
	void readBytes(byte cmd, uint16_t offset, byte data[], uint16_t n) {
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
			data[i] = SPI.transfer(JUNK); // read values
		}
		delayMicroseconds(5);
		digitalWrite(_ss, HIGH);
		SPI.endTransaction();
	}

	// always 4 bytes
	// TODO why always 4 bytes? can be different, see p. 58 table 10 otp memory map
	void readBytesOTP(uint16_t address, byte data[]) {
		byte addressBytes[LEN_OTP_ADDR];
		
		// p60 - 6.3.3 Reading a value from OTP memory
		// bytes of address
		addressBytes[0] = (address & 0xFF);
		addressBytes[1] = ((address >> 8) & 0xFF);
		// set address
		writeBytes(OTP_IF, OTP_ADDR_SUB, addressBytes, LEN_OTP_ADDR);
		// switch into read mode
		writeByte(OTP_IF, OTP_CTRL_SUB, 0x03); // OTPRDEN | OTPREAD
		writeByte(OTP_IF, OTP_CTRL_SUB, 0x01); // OTPRDEN
		// read value/block - 4 bytes
		readBytes(OTP_IF, OTP_RDAT_SUB, data, LEN_OTP_RDAT);
		// end read mode
		writeByte(OTP_IF, OTP_CTRL_SUB, 0x00);
	}

	// Helper to set a single register
	void writeByte(byte cmd, uint16_t offset, byte data) {
		writeBytes(cmd, offset, &data, 1);
	}

	/*
	* Write bytes to the DWM1000. Single bytes can be written to registers via sub-addressing.
	* @param cmd
	* 		The register address (see Chapter 7 in the DWM1000 user manual).
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
	void writeBytes(byte cmd, uint16_t offset, byte data[], uint16_t data_size) {
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
}