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
 * @file DWM1000Utils.cpp
 * Arduino driver library (header file) for the Decawave DWM1000 UWB transceiver Module.
 */

#include <Arduino.h>
#include "DWM1000Utils.h"

namespace DWM1000Utils {
	/*
	* Set the value of a bit in an array of bytes that are considered
	* consecutive and stored from MSB to LSB.
	* @param data
	* 		The number as byte array.
	* @param n
	* 		The number of bytes in the array.
	* @param bit
	* 		The position of the bit to be set.
	* @param val
	*		The boolean value to be set to the given bit position.
	*/
	void setBit(byte data[], uint16_t n, uint16_t bit, boolean val) {
		uint16_t idx;
		uint8_t shift;
		
		idx = bit/8;
		if(idx >= n) {
			return; // TODO proper error handling: out of bounds
		}
		byte* targetByte = &data[idx];
		shift = bit%8;
		if(val) {
			bitSet(*targetByte, shift);
		} else {
			bitClear(*targetByte, shift);
		}
	}

	/*
	* Check the value of a bit in an array of bytes that are considered
	* consecutive and stored from MSB to LSB.
	* @param data
	* 		The number as byte array.
	* @param n
	* 		The number of bytes in the array.
	* @param bit
	* 		The position of the bit to be checked.
	*/
	boolean getBit(byte data[], uint16_t n, uint16_t bit) {
		uint16_t idx;
		uint8_t  shift;
		
		idx = bit/8;
		if(idx >= n) {
			return false; // TODO proper error handling: out of bounds
		}
		byte targetByte = data[idx];
		shift = bit%8;
		
		return bitRead(targetByte, shift); // TODO wrong type returned byte instead of boolean
	}

	void writeValueToBytes(byte data[], int32_t val, uint16_t n) {
		uint16_t i;
		for(i = 0; i < n; i++) {
			data[i] = ((val >> (i*8)) & 0xFF); // TODO bad types - signed unsigned problem
		}
	}
}

