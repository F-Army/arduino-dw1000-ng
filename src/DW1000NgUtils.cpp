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
 * Decawave DW1000 library for arduino.
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
 * @file DW1000.cpp
 * Helper functions.
 */

#include <Arduino.h>
#include "DW1000NgUtils.hpp"
#include "DW1000NgConstants.hpp"
#include "DW1000NgRegisters.hpp"

namespace DW1000NgUtils {
	
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

	void writeValueToBytes(byte data[], uint64_t val, uint8_t n) {
		for(auto i = 0; i < n; i++) {
			data[i] = ((val >> (i*8)) & 0xFF);
		}
	}

	uint64_t bytesAsValue(byte data[], uint8_t n) {
		uint64_t value = 0;
		for(auto i = 0; i < n; i++) {	
			value |= ((uint64_t)data[i] << (i*8));
		}
		return value;
	}

	uint8_t nibbleFromChar(char c) {
		if(c >= '0' && c <= '9') {
			return c-'0';
		}
		if(c >= 'a' && c <= 'f') {
			return c-'a'+10;
		}
		if(c >= 'A' && c <= 'F') {
			return c-'A'+10;
		}
		return 255;
	}

	void convertToByte(const char string[], byte* bytes) {
		byte eui_byte[LEN_EUI];
		// we fill it with the char array under the form of "AA:FF:1C:...."
		for(uint16_t i = 0; i < LEN_EUI; i++) {
			eui_byte[i] = (nibbleFromChar(string[i*3]) << 4)+nibbleFromChar(string[i*3+1]);
		}
		memcpy(bytes, eui_byte, LEN_EUI);
	}
	
}
