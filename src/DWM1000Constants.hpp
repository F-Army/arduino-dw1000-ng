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
 * @file DWM1000Constants.h
 * Arduino driver library (header file) for the Decawave DWM1000 UWB transceiver module.
 */

#pragma once

#include <Arduino.h>

#define GPIO_MODE 0
#define LED_MODE 1

/* preamble codes (CHAN_CTRL - RX & TX _CODE) - reg:0x1F, bits:31-27,26-22 */
constexpr byte PREAMBLE_CODE_16MHZ_1  = 1;
constexpr byte PREAMBLE_CODE_16MHZ_2  = 2;
constexpr byte PREAMBLE_CODE_16MHZ_3  = 3;
constexpr byte PREAMBLE_CODE_16MHZ_4  = 4;
constexpr byte PREAMBLE_CODE_16MHZ_5  = 5;
constexpr byte PREAMBLE_CODE_16MHZ_6  = 6;
constexpr byte PREAMBLE_CODE_16MHZ_7  = 7;
constexpr byte PREAMBLE_CODE_16MHZ_8  = 8;
constexpr byte PREAMBLE_CODE_64MHZ_9  = 9;
constexpr byte PREAMBLE_CODE_64MHZ_10 = 10;
constexpr byte PREAMBLE_CODE_64MHZ_11 = 11;
constexpr byte PREAMBLE_CODE_64MHZ_12 = 12;
constexpr byte PREAMBLE_CODE_64MHZ_17 = 17;
constexpr byte PREAMBLE_CODE_64MHZ_18 = 18;
constexpr byte PREAMBLE_CODE_64MHZ_19 = 19;
constexpr byte PREAMBLE_CODE_64MHZ_20 = 20;

/* Validity matrix for 16 MHz PRF preamble codes */
constexpr byte preamble_validity_matrix_PRF16[8][2] = {
    {0,0}, /* Channel 0 doesn't exist */
    {PREAMBLE_CODE_16MHZ_1, PREAMBLE_CODE_16MHZ_2},
    {PREAMBLE_CODE_16MHZ_3, PREAMBLE_CODE_16MHZ_4},
    {PREAMBLE_CODE_16MHZ_5, PREAMBLE_CODE_16MHZ_6},
    {PREAMBLE_CODE_16MHZ_7, PREAMBLE_CODE_16MHZ_8},
    {PREAMBLE_CODE_16MHZ_3, PREAMBLE_CODE_16MHZ_4},
    {0,0}, /* Channel 5 doesn't exist */
    {PREAMBLE_CODE_16MHZ_7, PREAMBLE_CODE_16MHZ_8}
};

/* Validity matrix for 64 MHz PRF preamble codes */
constexpr byte preamble_validity_matrix_PRF64[8][4] = {
    {0,0,0,0}, /* Channel 0 doesn't exist */
    {PREAMBLE_CODE_64MHZ_9, PREAMBLE_CODE_64MHZ_10, PREAMBLE_CODE_64MHZ_11, PREAMBLE_CODE_64MHZ_12},
    {PREAMBLE_CODE_64MHZ_9, PREAMBLE_CODE_64MHZ_10, PREAMBLE_CODE_64MHZ_11, PREAMBLE_CODE_64MHZ_12},
    {PREAMBLE_CODE_64MHZ_9, PREAMBLE_CODE_64MHZ_10, PREAMBLE_CODE_64MHZ_11, PREAMBLE_CODE_64MHZ_12},
    {PREAMBLE_CODE_64MHZ_17, PREAMBLE_CODE_64MHZ_18, PREAMBLE_CODE_64MHZ_19, PREAMBLE_CODE_64MHZ_20},
    {PREAMBLE_CODE_64MHZ_9, PREAMBLE_CODE_64MHZ_10, PREAMBLE_CODE_64MHZ_11, PREAMBLE_CODE_64MHZ_12},
    {0,0,0,0}, /* Channel 5 doesn't exist */
    {PREAMBLE_CODE_64MHZ_17, PREAMBLE_CODE_64MHZ_18, PREAMBLE_CODE_64MHZ_19, PREAMBLE_CODE_64MHZ_20}
};

/* transmission/reception bit rate (TXBR) - reg:0x08, bits:14,13 */
constexpr byte TRX_RATE_110KBPS  = 0x00;
constexpr byte TRX_RATE_850KBPS  = 0x01;
constexpr byte TRX_RATE_6800KBPS = 0x02;

/* transmission pulse frequency (TXPRF) - reg:0x08, bits:17,16
* 0x00 is 4MHZ, but receiver in DWM1000 does not support it (!??) */
constexpr byte TX_PULSE_FREQ_16MHZ = 0x01;
constexpr byte TX_PULSE_FREQ_64MHZ = 0x02;

/* preamble length (PE + TXPSR) - reg:0x08, bits:21,20,19,18 - table 16 */
constexpr byte TX_PREAMBLE_LEN_64   = 0x01;
constexpr byte TX_PREAMBLE_LEN_128  = 0x05;
constexpr byte TX_PREAMBLE_LEN_256  = 0x09;
constexpr byte TX_PREAMBLE_LEN_512  = 0x0D;
constexpr byte TX_PREAMBLE_LEN_1024 = 0x02;
constexpr byte TX_PREAMBLE_LEN_1536 = 0x06;
constexpr byte TX_PREAMBLE_LEN_2048 = 0x0A;
constexpr byte TX_PREAMBLE_LEN_4096 = 0x03;

/* PAC size (DRX_TUNE2) - reg:0x08, sub-reg:0x27, bits:26,25 - table 33
* The value to program the sub-register changes in based of RXPRF */
constexpr byte PAC_SIZE_8  = 8;
constexpr byte PAC_SIZE_16 = 16;
constexpr byte PAC_SIZE_32 = 32;
constexpr byte PAC_SIZE_64 = 64;

/* channel of operation (CHAN_CTRL - TX & RX _CHAN) - reg:0x1F, bits:3-0,7-4 */
constexpr byte CHANNEL_1 = 1;
constexpr byte CHANNEL_2 = 2;
constexpr byte CHANNEL_3 = 3;
constexpr byte CHANNEL_4 = 4;
constexpr byte CHANNEL_5 = 5;
constexpr byte CHANNEL_7 = 7;

/* frame length settings. */
constexpr byte FRAME_LENGTH_NORMAL   = 0x00;
constexpr byte FRAME_LENGTH_EXTENDED = 0x03;

/* Register is 6 bit, 7 = write, 6 = sub-adressing, 5-0 = register value
 * Total header with sub-adressing can be 15 bit. */
const byte WRITE      = 0x80; // regular write
const byte WRITE_SUB  = 0xC0; // write with sub address
const byte READ       = 0x00; // regular read
const byte READ_SUB   = 0x40; // read with sub address
const byte RW_SUB_EXT = 0x80; // R/W with sub address extension

/* clocks available. */
const byte SYS_AUTO_CLOCK = 0x00;
const byte SYS_XTI_CLOCK  = 0x01;
const byte SYS_PLL_CLOCK  = 0x02;
const byte TX_PLL_CLOCK = 0x20;

/* range bias tables (500/900 MHz band, 16/64 MHz PRF), -61 to -95 dBm. */
const byte BIAS_500_16_ZERO = 10;
const byte BIAS_500_64_ZERO = 8;
const byte BIAS_900_16_ZERO = 7;
const byte BIAS_900_64_ZERO = 7;

/* range bias tables (500 MHz in [mm] and 900 MHz in [2mm] - to fit into bytes) */
constexpr byte BIAS_500_16[] = {198, 187, 179, 163, 143, 127, 109, 84, 59, 31, 0, 36, 65, 84, 97, 106, 110, 112};
constexpr byte BIAS_500_64[] = {110, 105, 100, 93, 82, 69, 51, 27, 0, 21, 35, 42, 49, 62, 71, 76, 81, 86};
constexpr byte BIAS_900_16[] = {137, 122, 105, 88, 69, 47, 25, 0, 21, 48, 79, 105, 127, 147, 160, 169, 178, 197};
constexpr byte BIAS_900_64[] = {147, 133, 117, 99, 75, 50, 29, 0, 24, 45, 63, 76, 87, 98, 116, 122, 132, 142};

enum class DriverAmplifierValue : byte {
    dB_18,
    dB_15,
    dB_12,
    dB_9,
    dB_6,
    dB_3,
    dB_0,
    OFF
};

enum class TransmitMixerValue : byte {
    dB_0, 
    dB_0_5, 
    dB_1, 
    dB_1_5, 
    dB_2, 
    dB_2_5,
    dB_3,
    dB_3_5,
    dB_4,
    dB_4_5,
    dB_5,
    dB_5_5,
    dB_6,
    dB_6_5,
    dB_7,
    dB_7_5,
    dB_8,
    dB_8_5,
    dB_9,
    dB_9_5,
    dB_10,
    dB_10_5,
    dB_11,
    dB_11_5,
    dB_12,
    dB_12_5,
    dB_13,
    dB_13_5,
    dB_14,
    dB_14_5,
    dB_15,
    dB_15_5
};

enum class SFDMode {STANDARD_SFD, DECAWAVE_SFD, RECOMMENDED_SFD};

enum class TransmitMode {IMMEDIATE, DELAYED};

enum class ReceiveMode {IMMEDIATE, DELAYED};