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
 * @file SPIporting.cpp
 * Helper functions.
 */

#include <Arduino.h>
#include <SPI.h>
#include "SPIporting.hpp"
#include "DW1000NgConstants.hpp"
#include "DW1000NgRegisters.hpp"

namespace SPIporting {
	
	namespace {
		/* SPI relative variables */
		#if defined(ESP32) || defined(ESP8266)
			const SPISettings  _fastSPI = SPISettings(20000000L, MSBFIRST, SPI_MODE0);
		#else
			const SPISettings  _fastSPI = SPISettings(16000000L, MSBFIRST, SPI_MODE0);
		#endif
		const SPISettings  _slowSPI = SPISettings(2000000L, MSBFIRST, SPI_MODE0);
		const SPISettings* _currentSPI = &_fastSPI;

		void _openSPI(uint8_t chipSelectPin) {
			SPI.beginTransaction(*_currentSPI);
			digitalWrite(chipSelectPin, LOW);
		}

    	void _closeSPI(uint8_t chipSelectPin) {
			digitalWrite(chipSelectPin, HIGH);
			SPI.endTransaction();
		}
	}

	void SPIinit() {
		SPI.begin();
	}

	void SPIend() {
		SPI.end();
	}

	void SPIselect(uint8_t chipSelectPin, uint8_t irq) {
		#if !defined(ESP32) && !defined(ESP8266)
			if(irq != 0xff)
				SPI.usingInterrupt(digitalPinToInterrupt(irq));
		#endif
		pinMode(chipSelectPin, OUTPUT);
		digitalWrite(chipSelectPin, HIGH);
	}

	void writeToSPI(uint8_t chipSelectPin, uint8_t headerLen, byte header[], uint16_t dataLen, byte data[]) {
		_openSPI(chipSelectPin);
		for(auto i = 0; i < headerLen; i++) {
			SPI.transfer(header[i]); // send header
		}
		for(auto i = 0; i < dataLen; i++) {
			SPI.transfer(data[i]); // write values
		}
		delayMicroseconds(5);
		_closeSPI(chipSelectPin);
	}

    void readFromSPI(uint8_t chipSelectPin, uint8_t headerLen, byte header[], uint16_t dataLen, byte data[]){
		_openSPI(chipSelectPin);
		for(auto i = 0; i < headerLen; i++) {
			SPI.transfer(header[i]); // send header
		}
		for(auto i = 0; i < dataLen; i++) {
			data[i] = SPI.transfer(0x00); // read values
		}
		delayMicroseconds(5);
		_closeSPI(chipSelectPin);
	}

	void setSPIspeed(SPIClock speed) {
		if(speed == SPIClock::FAST) {
			_currentSPI = &_fastSPI;
		 } else if(speed == SPIClock::SLOW) {
			_currentSPI = &_slowSPI;
		 } else {
			 // TODO error
		 }
	}

	int getSPIclock() {
		/*if(_currentSPI == SPIClock::FAST) {
			return 1;
		} else if(_currentSPI == SPIClock::SLOW) {
			return 0;
		} else {
			 // TODO error
		} */
	}
}