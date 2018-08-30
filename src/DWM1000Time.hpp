/**
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>
 * Copyright (c) 2016 by Ludwig Grill (www.rotzbua.de); refactored class
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
 * @file DWM1000Time.h
 * Arduino driver library timestamp wrapper (header file) for the Decawave 
 * DWM1000 UWB transceiver module.
 * 
 * @TODO
 * - avoid/remove floating operations, expensive on most microprocessors
 * 
 * @note
 * comments in cpp file, makes .h smaller and gives a better overview about
 * available methods and variables.
 */

#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <inttypes.h>
#include "DWM1000CompileOptions.hpp"
#include "deprecated.hpp"
#include "require_cpp11.hpp"

#if DWM1000TIME_H_PRINTABLE
class DWM1000Time : public Printable {
#else
class DWM1000Time {
#endif // DWM1000Time_H_PRINTABLE
public:
	// Time resolution in micro-seconds of time based registers/values.
	// Each bit in a timestamp counts for a period of approx. 15.65ps
	static constexpr float TIME_RES     = 0.000015650040064103f;
	static constexpr float TIME_RES_INV = 63897.6f;
	
	/* Speed of radio waves (light) [m/s] * timestamp resolution [~15.65ps] of DWM1000 */
	static constexpr float DISTANCE_OF_RADIO     = 0.0046917639786159f;
	static constexpr float DISTANCE_OF_RADIO_INV = 213.139451293f;
	
	// timestamp byte length - 40 bit -> 5 byte
	static constexpr uint8_t LENGTH_TIMESTAMP = 5;
	
	// timer/counter overflow (40 bits) -> 4overflow approx. every 17.2 seconds
	static constexpr int64_t TIME_OVERFLOW = 0x10000000000; //1099511627776LL
	static constexpr int64_t TIME_MAX      = 0xffffffffff;
	
	// time factors (relative to [us]) for setting delayed transceive
	// TODO use non float
	static constexpr float SECONDS      = 1e6;
	static constexpr float MILLISECONDS = 1e3;
	static constexpr float MICROSECONDS = 1;
	static constexpr float NANOSECONDS  = 1e-3;
	
	// constructor
	DWM1000Time();
	DWM1000Time(int64_t time);
	DWM1000Time(byte data[]);
	DWM1000Time(const DWM1000Time& copy);
	DWM1000Time(float timeUs);
	DWM1000Time(int32_t value, float factorUs);
	~DWM1000Time();
	
	// setter
	// DWM1000 timestamp, increase of +1 approx approx. 15.65ps real time
	void setTimestamp(int64_t value);
	void setTimestamp(byte data[]);
	void setTimestamp(const DWM1000Time& copy);
	
	// real time in us
	void setTime(float timeUs);
	void setTime(int32_t value, float factorUs);
	
	// getter
	int64_t getTimestamp() const;
	void    getTimestamp(byte data[]) const;
	
	DEPRECATED_MSG("use getAsMicroSeconds()")
	float getAsFloat() const;
	// getter, convert the timestamp to usual units
	float getAsMicroSeconds() const;
	//void getAsBytes(byte data[]) const; // TODO check why it is here, is it old version of getTimestamp(byte) ?
	float getAsMeters() const;
	
	DWM1000Time& wrap();
	
	// self test
	bool isValidTimestamp();
	
	// assign
	DWM1000Time& operator=(const DWM1000Time& assign);
	// add
	DWM1000Time& operator+=(const DWM1000Time& add);
	DWM1000Time operator+(const DWM1000Time& add) const;
	// subtract
	DWM1000Time& operator-=(const DWM1000Time& sub);
	DWM1000Time operator-(const DWM1000Time& sub) const;
	// multiply
	// multiply with float cause lost in accuracy, because float calculates only with 23bit matise
	DWM1000Time& operator*=(float factor);
	DWM1000Time operator*(float factor) const;
	// no accuracy lost
	DWM1000Time& operator*=(const DWM1000Time& factor);
	DWM1000Time operator*(const DWM1000Time& factor) const;
	// divide
	// divide with float cause lost in accuracy, because float calculates only with 23bit matise
	DWM1000Time& operator/=(float factor);
	DWM1000Time operator/(float factor) const;
	// no accuracy lost
	DWM1000Time& operator/=(const DWM1000Time& factor);
	DWM1000Time operator/(const DWM1000Time& factor) const;
	// compare
	boolean operator==(const DWM1000Time& cmp) const;
	boolean operator!=(const DWM1000Time& cmp) const;

#ifdef DWM1000TIME_H_PRINTABLE
	// print to serial for debug
	DEPRECATED_MSG("use Serial.print(object)")
	void print();
	// for usage with e.g. Serial.print()
	size_t printTo(Print& p) const;
#endif // DWM1000Time_H_PRINTABLE
	
private:
	// timestamp size from DWM1000 is 40bit, maximum number 1099511627775
	// signed because you can calculate with DWM1000Time; negative values are possible errors
	int64_t _timestamp = 0;
};
