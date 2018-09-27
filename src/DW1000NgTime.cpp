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
 * @file DW1000NgTime.cpp
 * Arduino driver library timestamp wrapper (source file) for the Decawave 
 * DW1000Ng UWB transceiver module.
 */

#include <Arduino.h>
#include "DW1000NgTime.hpp"
#include "DW1000NgCompileOptions.hpp"

/**
 * Initiates DW1000NgTime with 0
 */
DW1000NgTime::DW1000NgTime() {
	_timestamp = 0;
}

/**
 * Initiates DW1000NgTime with timestamp
 * @param time timestamp with intervall 1 is approx. 15ps
 */
DW1000NgTime::DW1000NgTime(int64_t time) {
	setTimestamp(time);
}

/**
 * Initiates DW1000NgTime with timestamp
 * @param data timestamp as byte array
 */
DW1000NgTime::DW1000NgTime(byte data[]) {
	setTimestamp(data);
}

/**
 * Initiates DW100Time with another instance
 * @param copy other instance
 */
DW1000NgTime::DW1000NgTime(const DW1000NgTime& copy) {
	setTimestamp(copy);
}

/**
 * Initiates DW100Time with micro seconds
 * @param timeUs time in micro seconds
 * @todo maybe replace by better function without float
 */
DW1000NgTime::DW1000NgTime(float timeUs) {
	setTime(timeUs);
}

/**
 * Initiates DW100Time with time and factor
 * @param value time
 * @param factorUs multiply factor for time
 * @todo maybe replace by better function without float
 */
DW1000NgTime::DW1000NgTime(int32_t value, float factorUs) {
	setTime(value, factorUs);
}

/**
 * Empty
 */
DW1000NgTime::~DW1000NgTime() {}

/**
 * Set timestamp
 * @param value - timestamp with intervall 1 is approx. 15ps
 */
void DW1000NgTime::setTimestamp(int64_t value) {
	_timestamp = value;
}

/**
 * Set timestamp
 * @param data timestamp as byte array
 */
void DW1000NgTime::setTimestamp(byte data[]) {
	_timestamp = 0;
	for(uint8_t i = 0; i < LENGTH_TIMESTAMP; i++) {
		_timestamp |= ((int64_t)data[i] << (i*8));
	}
}

/**
 * Set timestamp from other instance
 * @param copy instance where the timestamp should be copied
 */
void DW1000NgTime::setTimestamp(const DW1000NgTime& copy) {
	_timestamp = copy.getTimestamp();
}

/**
 * Initiates DW100Time with micro seconds
 * @param timeUs time in micro seconds
 * @todo maybe replace by better function without float
 */
void DW1000NgTime::setTime(float timeUs) {
	_timestamp = (int64_t)(timeUs*TIME_RES_INV);
//	_timestamp %= TIME_OVERFLOW; // clean overflow
}

/**
 * Set DW100Time with time and factor
 * @param value time
 * @param factorUs multiply factor for time
 * @todo maybe replace by better function without float
 */
void DW1000NgTime::setTime(int32_t value, float factorUs) {
	//float tsValue = value*factorUs;
	//tsValue = fmod(tsValue, TIME_OVERFLOW);
	//setTime(tsValue);
	setTime(value*factorUs);
}

/**
 * Get timestamp as integer
 * @return timestamp as integer
 */
int64_t DW1000NgTime::getTimestamp() const {
	return _timestamp;
}

/**
 * Get timestamp as byte array
 * @param data var where data should be written
 */
void DW1000NgTime::getTimestamp(byte data[]) const {
	memset(data, 0, LENGTH_TIMESTAMP);
	for(uint8_t i = 0; i < LENGTH_TIMESTAMP; i++) {
		data[i] = (byte)((_timestamp >> (i*8)) & 0xFF);
	}
}

/**
 * Return real time in micro seconds
 * @return time in micro seconds
 * @deprecated use getAsMicroSeconds()
 */
float DW1000NgTime::getAsFloat() const {
	//return fmod((float)_timestamp, TIME_OVERFLOW)*TIME_RES;
	return getAsMicroSeconds();
}

/**
 * Return real time in micro seconds
 * @return time in micro seconds
 */
float DW1000NgTime::getAsMicroSeconds() const {
	return (_timestamp%TIME_OVERFLOW)*TIME_RES;
}

/**
 * Return time as distance in meter, d=c*t
 * this is useful for e.g. time of flight
 * @return distance in meters
 */
float DW1000NgTime::getAsMeters() const {
	//return fmod((float)_timestamp, TIME_OVERFLOW)*DISTANCE_OF_RADIO;
	return (_timestamp%TIME_OVERFLOW)*DISTANCE_OF_RADIO;
}

/**
 * Converts negative values due overflow of one node to correct value
 * @example:
 * Maximum timesamp is 1000.
 * Node N1 sends 999 as timesamp. N2 recieves and sends delayed and increased timestamp back.
 * Delay is 10, so timestamp would be 1009, but due overflow 009 is sent back.
 * Now calculate TOF: 009 - 999 = -990 -> not correct time, so wrap()
 * Wrap calculation: -990 + 1000 = 10 -> correct time 
 * @return 
 */
DW1000NgTime& DW1000NgTime::wrap() {
	if(_timestamp < 0) {
		_timestamp += TIME_OVERFLOW;
	}
	return *this;
}

/**
 * Check if timestamp is valid for usage with DW1000Ng device
 * @return true if valid, false if negative or overflow (maybe after calculation)
 */
bool DW1000NgTime::isValidTimestamp() {
	return (0 <= _timestamp && _timestamp <= TIME_MAX);
}

// assign
DW1000NgTime& DW1000NgTime::operator=(const DW1000NgTime& assign) {
	if(this == &assign) {
		return *this;
	}
	_timestamp = assign.getTimestamp();
	return *this;
}

// add
DW1000NgTime& DW1000NgTime::operator+=(const DW1000NgTime& add) {
	_timestamp += add.getTimestamp();
	return *this;
}

DW1000NgTime DW1000NgTime::operator+(const DW1000NgTime& add) const {
	return DW1000NgTime(*this) += add;
}

// subtract
DW1000NgTime& DW1000NgTime::operator-=(const DW1000NgTime& sub) {
	_timestamp -= sub.getTimestamp();
	return *this;
}

DW1000NgTime DW1000NgTime::operator-(const DW1000NgTime& sub) const {
	return DW1000NgTime(*this) -= sub;
}

// multiply
DW1000NgTime& DW1000NgTime::operator*=(float factor) {
	//float tsValue = (float)_timestamp*factor;
	//_timestamp = (int64_t)tsValue;
	_timestamp *= factor;
	return *this;
}

DW1000NgTime DW1000NgTime::operator*(float factor) const {
	return DW1000NgTime(*this) *= factor;
}

DW1000NgTime& DW1000NgTime::operator*=(const DW1000NgTime& factor) {
	_timestamp *= factor.getTimestamp();
	return *this;
}

DW1000NgTime DW1000NgTime::operator*(const DW1000NgTime& factor) const {
	return DW1000NgTime(*this) *= factor;
}

// divide
DW1000NgTime& DW1000NgTime::operator/=(float factor) {
	//_timestamp *= (1.0f/factor);
	_timestamp /= factor;
	return *this;
}

DW1000NgTime DW1000NgTime::operator/(float factor) const {
	return DW1000NgTime(*this) /= factor;
}

DW1000NgTime& DW1000NgTime::operator/=(const DW1000NgTime& factor) {
	_timestamp /= factor.getTimestamp();
	return *this;
}

DW1000NgTime DW1000NgTime::operator/(const DW1000NgTime& factor) const {
	return DW1000NgTime(*this) /= factor;
}

// compare
boolean DW1000NgTime::operator==(const DW1000NgTime& cmp) const {
	return _timestamp == cmp.getTimestamp();
}

boolean DW1000NgTime::operator!=(const DW1000NgTime& cmp) const {
	//return !(*this == cmp); // seems not as intended
	return _timestamp != cmp.getTimestamp();
}

#if DW1000NGTIME_H_PRINTABLE
/**
 * For debuging, print timestamp pretty as integer with arduinos serial
 * @deprecated use Serial.print(object)
 */
void DW1000NgTime::print() {
	Serial.print(*this);
	Serial.println();
}

/**
 * Print timestamp of instance as integer with e.g. Serial.print()
 * @param p printer instance
 * @return size of printed chars
 */
size_t DW1000NgTime::printTo(Print& p) const {
	int64_t       number  = _timestamp;
	unsigned char buf[21];
	uint8_t       i       = 0;
	uint8_t       printed = 0;
	// printf for arduino avr do not support int64, so we have to calculate
	if(number == 0) {
		p.print((char)'0');
		return 1;
	}
	if(number < 0) {
		p.print((char)'-');
		number = -number; // make positive
		printed++;
	}
	while(number > 0) {
		int64_t q = number/10;
		buf[i++] = number-q*10;
		number = q;
	}
	printed += i;
	for(; i > 0; i--)
		p.print((char)(buf[i-1] < 10 ? '0'+buf[i-1] : 'A'+buf[i-1]-10));
	
	return printed;
}
#endif // DW1000NGTime_H_PRINTABLE
