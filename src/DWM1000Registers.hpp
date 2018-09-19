/*  
 *  Arduino-DWM1000 - Arduino library to use Decawave's DWM1000 module.
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

// no sub-address for register write
constexpr byte NO_SUB = 0xFF;

// device id register
constexpr byte DEV_ID = 0x00;
constexpr byte LEN_DEV_ID = 4;

// extended unique identifier register
constexpr byte EUI = 0x01;
constexpr byte LEN_EUI = 8;

// PAN identifier, short address register
constexpr byte PANADR = 0x03;
constexpr byte LEN_PANADR = 4;

// device configuration register
constexpr byte SYS_CFG = 0x04;
constexpr byte LEN_SYS_CFG = 4;
constexpr byte FFEN_BIT = 0;
constexpr byte FFBC_BIT = 1;
constexpr byte FFAB_BIT = 2;
constexpr byte FFAD_BIT = 3;
constexpr byte FFAA_BIT = 4;
constexpr byte FFAM_BIT = 5;
constexpr byte FFAR_BIT = 6;
constexpr byte DIS_DRXB_BIT = 12;
constexpr byte DIS_STXP_BIT = 18;
constexpr byte HIRQ_POL_BIT = 9;
constexpr byte RXAUTR_BIT = 29;
constexpr byte PHR_MODE_SUB = 16;
constexpr byte LEN_PHR_MODE_SUB = 2;
constexpr byte RXM110K_BIT = 22;

// device control register
constexpr byte SYS_CTRL = 0x0D;
constexpr byte LEN_SYS_CTRL = 4;
constexpr byte SFCST_BIT = 0;
constexpr byte TXSTRT_BIT = 1;
constexpr byte TXDLYS_BIT = 2;
constexpr byte TRXOFF_BIT = 6;
constexpr byte WAIT4RESP_BIT = 7;
constexpr byte RXENAB_BIT = 8;
constexpr byte RXDLYS_BIT = 9;

// system event status register
constexpr byte SYS_STATUS = 0x0F;
constexpr byte LEN_SYS_STATUS = 5;
constexpr byte CPLOCK_BIT = 1;
constexpr byte AAT_BIT = 3;
constexpr byte TXFRB_BIT = 4;
constexpr byte TXPRS_BIT = 5;
constexpr byte TXPHS_BIT = 6;
constexpr byte TXFRS_BIT = 7;
constexpr byte LDEDONE_BIT = 10;
constexpr byte RXPRD_BIT = 8;
constexpr byte RXSFDD_BIT = 9;
constexpr byte RXPHD_BIT = 11;
constexpr byte RXPHE_BIT = 12;
constexpr byte RXDFR_BIT = 13;
constexpr byte RXFCG_BIT = 14;
constexpr byte RXFCE_BIT = 15;
constexpr byte RXRFSL_BIT = 16;
constexpr byte RXRFTO_BIT = 17;
constexpr byte RXPTO_BIT = 21;
constexpr byte RXSFDTO_BIT = 26;
constexpr byte LDEERR_BIT = 18;
constexpr byte RFPLL_LL_BIT = 24;
constexpr byte CLKPLL_LL_BIT = 25;
constexpr byte AFFREJ_BIT = 29;

// system event mask register
// NOTE: uses the bit definitions of SYS_STATUS (below 32)
constexpr byte SYS_MASK = 0x0E;
constexpr byte LEN_SYS_MASK = 4;

// system time counter
constexpr byte SYS_TIME = 0x06;
constexpr byte LEN_SYS_TIME = 5;

// RX timestamp register
constexpr byte RX_TIME = 0x15;
constexpr byte LEN_RX_TIME = 14;
constexpr byte RX_STAMP_SUB = 0x00;
constexpr byte FP_AMPL1_SUB = 0x07;
constexpr byte LEN_RX_STAMP = 5;
constexpr byte LEN_FP_AMPL1 = 2;

// RX frame quality
constexpr byte RX_FQUAL = 0x12;
constexpr byte LEN_RX_FQUAL = 8;
constexpr byte STD_NOISE_SUB = 0x00;
constexpr byte FP_AMPL2_SUB = 0x02;
constexpr byte FP_AMPL3_SUB = 0x04;
constexpr byte CIR_PWR_SUB = 0x06;
constexpr byte LEN_STD_NOISE = 2;
constexpr byte LEN_FP_AMPL2 = 2;
constexpr byte LEN_FP_AMPL3 = 2;
constexpr byte LEN_CIR_PWR = 2;

// TX timestamp register
constexpr byte TX_TIME = 0x17;
constexpr byte LEN_TX_TIME = 10;
constexpr byte TX_STAMP_SUB = 0;
constexpr byte LEN_TX_STAMP = 5;

// timing register (for delayed RX/TX)
constexpr byte DX_TIME = 0x0A;
constexpr byte LEN_DX_TIME = 5;

// transmit data buffer
constexpr byte TX_BUFFER = 0x09;
constexpr byte LEN_TX_BUFFER = 1024;
constexpr byte LEN_UWB_FRAMES = 127;
constexpr byte LEN_EXT_UWB_FRAMES = 1023;

// RX frame info
constexpr byte RX_FINFO = 0x10;
constexpr byte LEN_RX_FINFO = 4;

// receive data buffer
constexpr byte RX_BUFFER = 0x11;
constexpr byte LEN_RX_BUFFER = 1024;

// transmit control
constexpr byte TX_FCTRL = 0x08;
constexpr byte LEN_TX_FCTRL = 5;

// channel control
constexpr byte CHAN_CTRL = 0x1F;
constexpr byte LEN_CHAN_CTRL = 4;
constexpr byte DWSFD_BIT = 17;
constexpr byte TNSSFD_BIT = 20;
constexpr byte RNSSFD_BIT = 21;

// user-defined SFD
constexpr byte USR_SFD = 0x21;
constexpr byte LEN_USR_SFD = 41;
constexpr byte SFD_LENGTH_SUB = 0x00;
constexpr byte LEN_SFD_LENGTH = 1;

// OTP control (for LDE micro code loading only)
constexpr byte OTP_IF = 0x2D;
constexpr byte OTP_ADDR_SUB = 0x04;
constexpr byte OTP_CTRL_SUB = 0x06;
constexpr byte OTP_RDAT_SUB = 0x0A;
constexpr byte LEN_OTP_ADDR = 2;
constexpr byte LEN_OTP_CTRL = 2;
constexpr byte LEN_OTP_RDAT = 4;

// AGC_TUNE1/2 (for re-tuning only)
constexpr byte AGC_TUNE = 0x23;
constexpr byte AGC_TUNE1_SUB = 0x04;
constexpr byte AGC_TUNE2_SUB = 0x0C;
constexpr byte AGC_TUNE3_SUB = 0x12;
constexpr byte LEN_AGC_TUNE1 = 2;
constexpr byte LEN_AGC_TUNE2 = 4;
constexpr byte LEN_AGC_TUNE3 = 2;

// DRX_TUNE2 (for re-tuning only)
constexpr byte DRX_TUNE = 0x27;
constexpr byte DRX_TUNE0b_SUB = 0x02;
constexpr byte DRX_TUNE1a_SUB = 0x04;
constexpr byte DRX_TUNE1b_SUB = 0x06;
constexpr byte DRX_TUNE2_SUB = 0x08;
constexpr byte DRX_TUNE4H_SUB = 0x26;
constexpr byte RXPACC_NOSAT_SUB = 0x2C;
constexpr byte LEN_DRX_TUNE0b = 2;
constexpr byte LEN_DRX_TUNE1a = 2;
constexpr byte LEN_DRX_TUNE1b = 2;
constexpr byte LEN_DRX_TUNE2 = 4;
constexpr byte LEN_DRX_TUNE4H = 2;
constexpr byte LEN_RXPACC_NOSAT = 2;

// LDE_CFG1 (for re-tuning only)
constexpr byte LDE_IF = 0x2E;
constexpr byte LDE_CFG1_SUB = 0x0806;
constexpr byte LDE_RXANTD_SUB = 0x1804;
constexpr byte LDE_CFG2_SUB = 0x1806;
constexpr byte LDE_REPC_SUB = 0x2804;
constexpr byte LEN_LDE_CFG1 = 1;
constexpr byte LEN_LDE_CFG2 = 2;
constexpr byte LEN_LDE_REPC = 2;
constexpr byte LEN_LDE_RXANTD = 2;

// DIG_DIAG (Digital Diagnostics Interface)
constexpr byte DIG_DIAG = 0x2F;
constexpr byte DIAG_TMC_SUB = 0x24;
constexpr byte LEN_DIAG_TMC = 2;

// TX_POWER (for re-tuning only)
constexpr byte TX_POWER = 0x1E;
constexpr byte LEN_TX_POWER = 4;

// RF_CONF (for re-tuning only)
constexpr byte RF_CONF = 0x28;
constexpr byte RF_CONF_SUB = 0x00;
constexpr byte RF_RXCTRLH_SUB = 0x0B;
constexpr byte RF_TXCTRL_SUB = 0x0C;
constexpr byte LEN_RX_CONF_SUB = 4;
constexpr byte LEN_RF_RXCTRLH = 1;
constexpr byte LEN_RF_TXCTRL = 4;

// TX_CAL (for re-tuning only)
constexpr byte TX_CAL = 0x2A;
constexpr byte TC_PGDELAY_SUB = 0x0B;
constexpr byte LEN_TC_PGDELAY = 1;
constexpr byte TC_SARC = 0x00;
constexpr byte TC_SARL = 0x03;

// FS_CTRL (for re-tuning only)
constexpr byte FS_CTRL = 0x2B;
constexpr byte FS_PLLCFG_SUB = 0x07;
constexpr byte FS_PLLTUNE_SUB = 0x0B;
constexpr byte FS_XTALT_SUB = 0x0E;
constexpr byte LEN_FS_PLLCFG = 4;
constexpr byte LEN_FS_PLLTUNE = 1;
constexpr byte LEN_FS_XTALT = 1;

// AON
constexpr byte AON = 0x2C;
constexpr byte AON_WCFG_SUB = 0x00;
constexpr byte LEN_AON_WCFG = 2;
constexpr byte ONW_LDC_BIT = 6;
constexpr byte ONW_LDD0_BIT = 12;
constexpr byte AON_CTRL_SUB = 0x02;
constexpr byte LEN_AON_CTRL = 1;
constexpr byte RESTORE_BIT = 0;
constexpr byte SAVE_BIT = 1;
constexpr byte UPL_CFG_BIT = 2;

constexpr byte AON_CFG0_SUB = 0x06;
constexpr byte LEN_AON_CFG0 = 4;
constexpr byte SLEEP_EN_BIT = 0;
constexpr byte WAKE_PIN_BIT = 1;
constexpr byte WAKE_SPI_BIT = 2;
constexpr byte WAKE_CNT_BIT = 3;

// PMSC
constexpr byte PMSC = 0x36;
constexpr byte PMSC_CTRL0_SUB = 0x00;
constexpr byte PMSC_CTRL1_SUB = 0x04;
constexpr byte PMSC_LEDC_SUB = 0x28;
constexpr byte LEN_PMSC_CTRL0 = 4;
constexpr byte LEN_PMSC_CTRL1 = 4;
constexpr byte LEN_PMSC_LEDC = 4;
constexpr byte GPDCE_BIT = 18;
constexpr byte KHZCLKEN_BIT = 23;
constexpr byte BLNKEN = 8;

constexpr byte ATXSLP_BIT = 11;
constexpr byte ARXSLP_BIT = 12;

// TX_ANTD Antenna delays
constexpr byte TX_ANTD = 0x18;
constexpr byte LEN_TX_ANTD = 2;

// GPIO
constexpr byte GPIO_CTRL = 0x26;
constexpr byte GPIO_MODE_SUB = 0x00;
constexpr byte LEN_GPIO_MODE = 4;