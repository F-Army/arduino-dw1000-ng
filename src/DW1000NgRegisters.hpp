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

#pragma once

#include <Arduino.h>

// no sub-address for register write
constexpr uint16_t NO_SUB = 0xFF;

// device id register
constexpr uint16_t DEV_ID = 0x00;
constexpr uint16_t LEN_DEV_ID = 4;

// extended unique identifier register
constexpr uint16_t EUI = 0x01;
constexpr uint16_t LEN_EUI = 8;

// PAN identifier, short address register
constexpr uint16_t PANADR = 0x03;
constexpr uint16_t LEN_PANADR = 4;

// device configuration register
constexpr uint16_t SYS_CFG = 0x04;
constexpr uint16_t FFEN_BIT = 0;
constexpr uint16_t FFBC_BIT = 1;
constexpr uint16_t FFAB_BIT = 2;
constexpr uint16_t FFAD_BIT = 3;
constexpr uint16_t FFAA_BIT = 4;
constexpr uint16_t FFAM_BIT = 5;
constexpr uint16_t FFAR_BIT = 6;
constexpr uint16_t FFA4_BIT = 7;
constexpr uint16_t FFA5_BIT = 8;
constexpr uint16_t HIRQ_POL_BIT = 9;
constexpr uint16_t SPI_EDGE_BIT = 10;
constexpr uint16_t DIS_FCE_BIT = 11;
constexpr uint16_t DIS_DRXB_BIT = 12;
constexpr uint16_t DIS_PHE_BIT = 13;
constexpr uint16_t DIS_RSDE_BIT = 14;
constexpr uint16_t FCS_INIT2F_BIT = 15;
constexpr uint16_t PHR_MODE_0_BIT = 16;
constexpr uint16_t PHR_MODE_1_BIT = 17;
constexpr uint16_t DIS_STXP_BIT = 18;
constexpr uint16_t RXM110K_BIT = 22;
constexpr uint16_t RXWTOE_BIT = 28;
constexpr uint16_t RXAUTR_BIT = 29;
constexpr uint16_t AUTOACK_BIT = 30;
constexpr uint16_t AACKPEND_BIT = 31;
constexpr uint16_t LEN_SYS_CFG = 4;

// device control register
constexpr uint16_t SYS_CTRL = 0x0D;
constexpr uint16_t LEN_SYS_CTRL = 4;
constexpr uint16_t SFCST_BIT = 0;
constexpr uint16_t TXSTRT_BIT = 1;
constexpr uint16_t TXDLYS_BIT = 2;
constexpr uint16_t TRXOFF_BIT = 6;
constexpr uint16_t WAIT4RESP_BIT = 7;
constexpr uint16_t RXENAB_BIT = 8;
constexpr uint16_t RXDLYS_BIT = 9;

// system event status register
constexpr uint16_t SYS_STATUS = 0x0F;
constexpr uint16_t SYS_STATUS_SUB = 0x04;
constexpr uint16_t IRQS_BIT = 0;
constexpr uint16_t CPLOCK_BIT = 1;
constexpr uint16_t ESYNCR_BIT = 2;
constexpr uint16_t AAT_BIT = 3;
constexpr uint16_t TXFRB_BIT = 4;
constexpr uint16_t TXPRS_BIT = 5;
constexpr uint16_t TXPHS_BIT = 6;
constexpr uint16_t TXFRS_BIT = 7;
constexpr uint16_t RXPRD_BIT = 8;
constexpr uint16_t RXSFDD_BIT = 9;
constexpr uint16_t LDEDONE_BIT = 10;
constexpr uint16_t RXPHD_BIT = 11;
constexpr uint16_t RXPHE_BIT = 12;
constexpr uint16_t RXDFR_BIT = 13;
constexpr uint16_t RXFCG_BIT = 14;
constexpr uint16_t RXFCE_BIT = 15;
constexpr uint16_t RXRFSL_BIT = 16;
constexpr uint16_t RXRFTO_BIT = 17;
constexpr uint16_t LDEERR_BIT = 18;
constexpr uint16_t RXOVRR_BIT = 20;
constexpr uint16_t RXPTO_BIT = 21;
constexpr uint16_t GPIOIRQ_BIT = 22;
constexpr uint16_t SLP2INIT_BIT = 23;
constexpr uint16_t RFPLL_LL_BIT = 24;
constexpr uint16_t CLKPLL_LL_BIT = 25;
constexpr uint16_t RXSFDTO_BIT = 26;
constexpr uint16_t HPDWARN_BIT = 27;
constexpr uint16_t TXBERR_BIT = 28;
constexpr uint16_t AFFREJ_BIT = 29;
constexpr uint16_t HSRBP_BIT = 30;
constexpr uint16_t ICRBP_BIT = 31;
constexpr uint16_t RXRSCS_BIT = 0;
constexpr uint16_t RXPREJ_BIT = 1;
constexpr uint16_t TXPUTE_BIT = 2;
constexpr uint16_t LEN_SYS_STATUS = 4;
constexpr uint16_t LEN_SYS_STATUS_SUB = 1;

// system event mask register
// NOTE: uses the bit definitions of SYS_STATUS (below 32)
constexpr uint16_t SYS_MASK = 0x0E;
constexpr uint16_t LEN_SYS_MASK = 4;

// system time counter
constexpr uint16_t SYS_TIME = 0x06;
constexpr uint16_t LEN_SYS_TIME = 5;

// RX timestamp register
constexpr uint16_t RX_TIME = 0x15;
constexpr uint16_t LEN_RX_TIME = 14;
constexpr uint16_t RX_STAMP_SUB = 0x00;
constexpr uint16_t FP_AMPL1_SUB = 0x07;
constexpr uint16_t LEN_RX_STAMP = 5;
constexpr uint16_t LEN_FP_AMPL1 = 2;

// RX frame quality
constexpr uint16_t RX_FQUAL = 0x12;
constexpr uint16_t LEN_RX_FQUAL = 8;
constexpr uint16_t STD_NOISE_SUB = 0x00;
constexpr uint16_t FP_AMPL2_SUB = 0x02;
constexpr uint16_t FP_AMPL3_SUB = 0x04;
constexpr uint16_t CIR_PWR_SUB = 0x06;
constexpr uint16_t LEN_STD_NOISE = 2;
constexpr uint16_t LEN_FP_AMPL2 = 2;
constexpr uint16_t LEN_FP_AMPL3 = 2;
constexpr uint16_t LEN_CIR_PWR = 2;

// TX timestamp register
constexpr uint16_t TX_TIME = 0x17;
constexpr uint16_t LEN_TX_TIME = 10;
constexpr uint16_t TX_STAMP_SUB = 0;
constexpr uint16_t LEN_TX_STAMP = 5;

// timing register (for delayed RX/TX)
constexpr uint16_t DX_TIME = 0x0A;
constexpr uint16_t LEN_DX_TIME = 5;

// Receive Frame Wait Timeout Period
constexpr uint16_t RX_WFTO = 0x0C;
constexpr uint16_t LEN_RX_WFTO = 2;

// transmit data buffer
constexpr uint16_t TX_BUFFER = 0x09;
constexpr uint16_t LEN_TX_BUFFER = 1024;
constexpr uint16_t LEN_UWB_FRAMES = 127;
constexpr uint16_t LEN_EXT_UWB_FRAMES = 1023;

// RX frame info
constexpr uint16_t RX_FINFO = 0x10;
constexpr uint16_t LEN_RX_FINFO = 4;

// receive data buffer
constexpr uint16_t RX_BUFFER = 0x11;
constexpr uint16_t LEN_RX_BUFFER = 1024;

// transmit control
constexpr uint16_t TX_FCTRL = 0x08;
constexpr uint16_t LEN_TX_FCTRL = 5;

// channel control
constexpr uint16_t CHAN_CTRL = 0x1F;
constexpr uint16_t LEN_CHAN_CTRL = 4;
constexpr uint16_t DWSFD_BIT = 17;
constexpr uint16_t TNSSFD_BIT = 20;
constexpr uint16_t RNSSFD_BIT = 21;

// user-defined SFD
constexpr uint16_t USR_SFD = 0x21;
constexpr uint16_t LEN_USR_SFD = 41;
constexpr uint16_t SFD_LENGTH_SUB = 0x00;
constexpr uint16_t LEN_SFD_LENGTH = 1;

// OTP control (for LDE micro code loading only)
constexpr uint16_t OTP_IF = 0x2D;
constexpr uint16_t OTP_ADDR_SUB = 0x04;
constexpr uint16_t OTP_CTRL_SUB = 0x06;
constexpr uint16_t OTP_RDAT_SUB = 0x0A;
constexpr uint16_t LEN_OTP_ADDR = 2;
constexpr uint16_t LEN_OTP_CTRL = 2;
constexpr uint16_t LEN_OTP_RDAT = 4;

// AGC_TUNE1/2/3 (for re-tuning only)
constexpr uint16_t AGC_TUNE = 0x23;
constexpr uint16_t AGC_TUNE1_SUB = 0x04;
constexpr uint16_t AGC_TUNE2_SUB = 0x0C;
constexpr uint16_t AGC_TUNE3_SUB = 0x12;
constexpr uint16_t LEN_AGC_TUNE1 = 2;
constexpr uint16_t LEN_AGC_TUNE2 = 4;
constexpr uint16_t LEN_AGC_TUNE3 = 2;

// EXT_SYNC (External Synchronization Control)
constexpr uint16_t EXT_SYNC = 0x24;
constexpr uint16_t EC_CTRL_SUB = 0x00;
constexpr uint16_t PLLLDT_BIT = 2;
constexpr uint16_t EC_RXTC_SUB = 0x04;
constexpr uint16_t EC_GOLP_SUB = 0x08;
constexpr uint16_t LEN_EC_CTRL = 4;
constexpr uint16_t LEN_EC_RXTC = 4;
constexpr uint16_t LEN_EC_GOLP = 4;

// DRX_TUNE2 (for re-tuning only)
constexpr uint16_t DRX_TUNE = 0x27;
constexpr uint16_t DRX_TUNE0b_SUB = 0x02;
constexpr uint16_t DRX_TUNE1a_SUB = 0x04;
constexpr uint16_t DRX_TUNE1b_SUB = 0x06;
constexpr uint16_t DRX_TUNE2_SUB = 0x08;
constexpr uint16_t DRX_SFDTOC_SUB = 0x20;
constexpr uint16_t DRX_PRETOC_SUB = 0x24;
constexpr uint16_t DRX_TUNE4H_SUB = 0x26;
constexpr uint16_t DRX_CAR_INT_SUB = 0x28;
constexpr uint16_t RXPACC_NOSAT_SUB = 0x2C;
constexpr uint16_t LEN_DRX_TUNE0b = 2;
constexpr uint16_t LEN_DRX_TUNE1a = 2;
constexpr uint16_t LEN_DRX_TUNE1b = 2;
constexpr uint16_t LEN_DRX_TUNE2 = 4;
constexpr uint16_t LEN_DRX_SFDTOC = 2;
constexpr uint16_t LEN_DRX_PRETOC = 2;
constexpr uint16_t LEN_DRX_TUNE4H = 2;
constexpr uint16_t LEN_DRX_CAR_INT = 3;
constexpr uint16_t LEN_RXPACC_NOSAT = 2;

// LDE_CFG1 (for re-tuning only)
constexpr uint16_t LDE_IF = 0x2E;
constexpr uint16_t LDE_CFG1_SUB = 0x0806;
constexpr uint16_t LDE_RXANTD_SUB = 0x1804;
constexpr uint16_t LDE_CFG2_SUB = 0x1806;
constexpr uint16_t LDE_REPC_SUB = 0x2804;
constexpr uint16_t LEN_LDE_CFG1 = 1;
constexpr uint16_t LEN_LDE_CFG2 = 2;
constexpr uint16_t LEN_LDE_REPC = 2;
constexpr uint16_t LEN_LDE_RXANTD = 2;

// DIG_DIAG (Digital Diagnostics Interface)
constexpr uint16_t DIG_DIAG = 0x2F;
constexpr uint16_t EVC_CTRL_SUB = 0x00;
constexpr uint16_t EVC_STO_SUB = 0x10;
constexpr uint16_t EVC_PTO_SUB = 0x12;
constexpr uint16_t EVC_FWTO_SUB = 0x14;
constexpr uint16_t DIAG_TMC_SUB = 0x24;
constexpr uint16_t LEN_EVC_CTRL = 4;
constexpr uint16_t LEN_EVC_STO = 2;
constexpr uint16_t LEN_EVC_PTO = 2;
constexpr uint16_t LEN_EVC_FWTO = 2;
constexpr uint16_t LEN_DIAG_TMC = 2;

// TX_POWER (for re-tuning only)
constexpr uint16_t TX_POWER = 0x1E;
constexpr uint16_t LEN_TX_POWER = 4;

// RF_CONF (for re-tuning only)
constexpr uint16_t RF_CONF = 0x28;
constexpr uint16_t RF_CONF_SUB = 0x00;
constexpr uint16_t RF_RXCTRLH_SUB = 0x0B;
constexpr uint16_t RF_TXCTRL_SUB = 0x0C;
constexpr uint16_t LEN_RX_CONF_SUB = 4;
constexpr uint16_t LEN_RF_RXCTRLH = 1;
constexpr uint16_t LEN_RF_TXCTRL = 4;

// TX_CAL (for re-tuning only)
constexpr uint16_t TX_CAL = 0x2A;
constexpr uint16_t TC_PGDELAY_SUB = 0x0B;
constexpr uint16_t LEN_TC_PGDELAY = 1;
constexpr uint16_t TC_SARC = 0x00;
constexpr uint16_t TC_SARL = 0x03;

// FS_CTRL (for re-tuning only)
constexpr uint16_t FS_CTRL = 0x2B;
constexpr uint16_t FS_PLLCFG_SUB = 0x07;
constexpr uint16_t FS_PLLTUNE_SUB = 0x0B;
constexpr uint16_t FS_XTALT_SUB = 0x0E;
constexpr uint16_t LEN_FS_PLLCFG = 4;
constexpr uint16_t LEN_FS_PLLTUNE = 1;
constexpr uint16_t LEN_FS_XTALT = 1;

// AON
constexpr uint16_t AON = 0x2C;
constexpr uint16_t AON_WCFG_SUB = 0x00;
constexpr uint16_t ONW_RADC_BIT = 0;
constexpr uint16_t ONW_RX_BIT = 1;
constexpr uint16_t ONW_LEUI_BIT = 3;
constexpr uint16_t ONW_LDC_BIT = 6;
constexpr uint16_t ONW_L64P_BIT = 7;
constexpr uint16_t ONW_PRES_SLEEP_BIT = 8;
constexpr uint16_t ONW_LLDE_BIT = 11;
constexpr uint16_t ONW_LLDO_BIT = 12;
constexpr uint16_t LEN_AON_WCFG = 2;

constexpr uint16_t AON_CTRL_SUB = 0x02;
constexpr uint16_t RESTORE_BIT = 0;
constexpr uint16_t SAVE_BIT = 1;
constexpr uint16_t UPL_CFG_BIT = 2;
constexpr uint16_t LEN_AON_CTRL = 1;

constexpr uint16_t AON_CFG0_SUB = 0x06;
constexpr uint16_t SLEEP_EN_BIT = 0;
constexpr uint16_t WAKE_PIN_BIT = 1;
constexpr uint16_t WAKE_SPI_BIT = 2;
constexpr uint16_t WAKE_CNT_BIT = 3;
constexpr uint16_t LPDIV_EN_BIT = 4;
constexpr uint16_t LEN_AON_CFG0 = 4;

constexpr uint16_t AON_CFG1_SUB = 0x0A;
constexpr uint16_t SLEEP_CEN_BIT = 0;
constexpr uint16_t SMXX_BIT = 1;
constexpr uint16_t LPOSC_CAL_BIT = 2;
constexpr uint16_t LEN_AON_CFG1 = 2;

// PMSC
constexpr uint16_t PMSC = 0x36;
constexpr uint16_t PMSC_CTRL0_SUB = 0x00;
constexpr uint16_t GPDCE_BIT = 18;
constexpr uint16_t KHZCLKEN_BIT = 23;
constexpr uint16_t PMSC_SOFTRESET_SUB = 0x03;
constexpr uint16_t PMSC_CTRL1_SUB = 0x04;
constexpr uint16_t ATXSLP_BIT = 11;
constexpr uint16_t ARXSLP_BIT = 12;
constexpr uint16_t PMSC_LEDC_SUB = 0x28;
constexpr uint16_t BLNKEN = 8;
constexpr uint16_t LEN_PMSC_CTRL0 = 4;
constexpr uint16_t LEN_PMSC_SOFTRESET = 1;
constexpr uint16_t LEN_PMSC_CTRL1 = 4;
constexpr uint16_t LEN_PMSC_LEDC = 4;

// TX_ANTD Antenna delays
constexpr uint16_t TX_ANTD = 0x18;
constexpr uint16_t LEN_TX_ANTD = 2;

// Acknowledgement time and response time
constexpr uint16_t ACK_RESP_T = 0x1A;
constexpr uint16_t ACK_RESP_T_W4R_TIME_SUB = 0x00;
constexpr uint16_t LEN_ACK_RESP_T_W4R_TIME_SUB = 3;
constexpr uint16_t LEN_ACK_RESP_T = 4;

// GPIO
constexpr uint16_t GPIO_CTRL = 0x26;
constexpr uint16_t GPIO_MODE_SUB = 0x00;
constexpr uint16_t LEN_GPIO_MODE = 4;