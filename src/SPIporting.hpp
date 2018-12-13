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
 * @file SPIporting.hpp
 * Arduino porting for the SPI interface.
 */

#pragma once

#include <Arduino.h>
#include "DW1000NgConstants.hpp"

namespace SPIporting{

    /** 
	Initializes the SPI bus.
	*/
    void SPIinit();

    /** 
	Tells the driver library that no communication to a DW1000 will be required anymore.
	This basically just frees SPI and the previously used pins.
	*/
	void SPIend();

    /** 
	(Re-)selects a specific DW1000 chip for communication. Used in case you switched SPI to another device.
	*/
	void SPIselect(uint8_t chipSelectPin, uint8_t irq);

    /**
    Arduino function to write to the SPI.
    Takes two separate byte buffers for write header and write data

    @param [in] Header lenght
    @param [in] Header array built before 
    @param [in] Data lenght
    @param [in] Data array 
    */
    void writeToSPI(uint8_t chipSelectPin, uint8_t headerLen, byte header[], uint16_t dataLen, byte data[]);

    /**
    Arduino function to read from the SPI.
    Takes two separate byte buffers for write header and write data

    @param [in] Header lenght
    @param [in] Header array built before 
    @param [in] Data lenght
    @param [in] Data array 
    */
    void readFromSPI(uint8_t chipSelectPin, uint8_t headerLen, byte header[], uint16_t dataLen, byte data[]);

    /**
    Sets speed of SPI clock, fast or slow(20MHz or 2MHz)

    @param [in] SPIClock FAST or SLOW
    */
    void setSPIspeed(SPIClock speed);

    /**
    Sets speed of SPI clock, fast or slow(20MHz or 2MHz)

    @param [out] 1 SPIClock FAST, 0 SPIClock SLOW
    */
    int getSPIclock();
}