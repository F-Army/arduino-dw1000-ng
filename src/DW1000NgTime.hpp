/**
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>
 * Copyright (c) 2016 by Ludwig Grill (www.rotzbua.de); refactored class
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
 * @file DW1000Time.h
 * Arduino driver library timestamp wrapper (header file) for the Decawave 
 * DW1000Ng UWB transceiver module.
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
#include "DW1000NgCompileOptions.hpp"
#include "deprecated.hpp"
#include "require_cpp11.hpp"

#if DW1000NGTIME_H_PRINTABLE
class DW1000NgTime : public Printable {
#else
class DW1000NgTime {
#endif // DW1000NgTime_H_PRINTABLE
public:
	
	// constructor
	DW1000NgTime();
	DW1000NgTime(int64_t time);
	DW1000NgTime(byte data[]);
	DW1000NgTime(const DW1000NgTime& copy);
	DW1000NgTime(float timeUs);
	DW1000NgTime(int32_t value, float factorUs);
	~DW1000NgTime();
	
	// setter
	// DW1000Ng timestamp, increase of +1 approx approx. 15.65ps real time
	void setTimestamp(int64_t value);
	void setTimestamp(byte data[]);
	void setTimestamp(const DW1000NgTime& copy);
	
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
	
	DW1000NgTime& wrap();
	
	// self test
	bool isValidTimestamp();
	
	// assign
	DW1000NgTime& operator=(const DW1000NgTime& assign);
	// add
	DW1000NgTime& operator+=(const DW1000NgTime& add);
	DW1000NgTime operator+(const DW1000NgTime& add) const;
	// subtract
	DW1000NgTime& operator-=(const DW1000NgTime& sub);
	DW1000NgTime operator-(const DW1000NgTime& sub) const;
	// multiply
	// multiply with float cause lost in accuracy, because float calculates only with 23bit matise
	DW1000NgTime& operator*=(float factor);
	DW1000NgTime operator*(float factor) const;
	// no accuracy lost
	DW1000NgTime& operator*=(const DW1000NgTime& factor);
	DW1000NgTime operator*(const DW1000NgTime& factor) const;
	// divide
	// divide with float cause lost in accuracy, because float calculates only with 23bit matise
	DW1000NgTime& operator/=(float factor);
	DW1000NgTime operator/(float factor) const;
	// no accuracy lost
	DW1000NgTime& operator/=(const DW1000NgTime& factor);
	DW1000NgTime operator/(const DW1000NgTime& factor) const;
	// compare
	boolean operator==(const DW1000NgTime& cmp) const;
	boolean operator!=(const DW1000NgTime& cmp) const;

#if DW1000NGTIME_H_PRINTABLE
	// print to serial for debug
	DEPRECATED_MSG("use Serial.print(object)")
	void print();
	// for usage with e.g. Serial.print()
	size_t printTo(Print& p) const;
#endif // DW1000NgTime_H_PRINTABLE
	
private:
	// timestamp size from DW1000Ng is 40bit, maximum number 1099511627775
	// signed because you can calculate with DW1000NgTime; negative values are possible errors
	int64_t _timestamp = 0;
};
