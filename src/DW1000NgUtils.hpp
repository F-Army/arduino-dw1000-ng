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
 * @file DW1000.h
 * Helper functions.
 */

#pragma once

#include <Arduino.h>
#include "DW1000NgConstants.hpp"

namespace DW1000NgUtils {

    /**
    Returns target bit value inside a byte array

    @param [in] data the byte array
    @param [in] n the length of the byte array
    @param [in] bit the bit position

    returns bit value (true = 1, false = 0)
    */
    boolean getBit(byte data[], uint16_t n, uint16_t bit);

    /**
    Sets the target bit value inside an array of bytes

    @param [in] data the byte array
    @param [in] n the length of the byte array
    @param [in] bit the bit position
    @param [in] val the bit value

    */
    void setBit(byte data[], uint16_t n, uint16_t bit, boolean val);

    /**
    Writes the target value inside a given byte array.

    @param [in] data the byte array
    @param [in] val the value to insert
    @param [in] n the length of the byte array
    */
    void writeValueToBytes(byte data[], uint64_t val, uint8_t n);
    
    /**
    Gets the target byte array value

    @param [in] data the byte array
    @param [in] n the length of the byte array

    returns the byte array value
    */
    uint64_t bytesAsValue(byte data[], uint8_t n);

    /**
    Converts from char to 4 bits (hexadecimal)

    @param [in] c the character

    returns target value
    */
    
	uint8_t nibbleFromChar(char c);

    /**
    Converts the target string to eui bytes

    @param [in] string The eui string (in format XX:XX:XX:XX:XX:XX:XX:XX)
    @param [out] eui_byte The eui bytes
    */
	void convertToByte(const char string[], byte* eui_byte);
}
