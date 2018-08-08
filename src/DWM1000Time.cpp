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
 * @file DWM1000Time.cpp
 * Arduino driver library timestamp wrapper (source file) for the Decawave 
 * DWM1000 UWB transceiver module.
 */

#include "DWM1000Time.h"
#include "Arduino.h"

/**
 * Initiates DWM1000Time with 0
 */
DWM1000Time::DWM1000Time() {
	_timestamp = 0;
}

/**
 * Initiates DWM1000Time with timestamp
 * @param time timestamp with intervall 1 is approx. 15ps
 */
DWM1000Time::DWM1000Time(int64_t time) {
	setTimestamp(time);
}

/**
 * Initiates DWM1000Time with timestamp
 * @param data timestamp as byte array
 */
DWM1000Time::DWM1000Time(byte data[]) {
	setTimestamp(data);
}

/**
 * Initiates DW100Time with another instance
 * @param copy other instance
 */
DWM1000Time::DWM1000Time(const DWM1000Time& copy) {
	setTimestamp(copy);
}

/**
 * Initiates DW100Time with micro seconds
 * @param timeUs time in micro seconds
 * @todo maybe replace by better function without float
 */
DWM1000Time::DWM1000Time(float timeUs) {
	setTime(timeUs);
}

/**
 * Initiates DW100Time with time and factor
 * @param value time
 * @param factorUs multiply factor for time
 * @todo maybe replace by better function without float
 */
DWM1000Time::DWM1000Time(int32_t value, float factorUs) {
	setTime(value, factorUs);
}

/**
 * Empty
 */
DWM1000Time::~DWM1000Time() {}

/**
 * Set timestamp
 * @param value - timestamp with intervall 1 is approx. 15ps
 */
void DWM1000Time::setTimestamp(int64_t value) {
	_timestamp = value;
}

/**
 * Set timestamp
 * @param data timestamp as byte array
 */
void DWM1000Time::setTimestamp(byte data[]) {
	_timestamp = 0;
	for(uint8_t i = 0; i < LENGTH_TIMESTAMP; i++) {
		_timestamp |= ((int64_t)data[i] << (i*8));
	}
}

/**
 * Set timestamp from other instance
 * @param copy instance where the timestamp should be copied
 */
void DWM1000Time::setTimestamp(const DWM1000Time& copy) {
	_timestamp = copy.getTimestamp();
}

/**
 * Initiates DW100Time with micro seconds
 * @param timeUs time in micro seconds
 * @todo maybe replace by better function without float
 */
void DWM1000Time::setTime(float timeUs) {
	_timestamp = (int64_t)(timeUs*TIME_RES_INV);
//	_timestamp %= TIME_OVERFLOW; // clean overflow
}

/**
 * Set DW100Time with time and factor
 * @param value time
 * @param factorUs multiply factor for time
 * @todo maybe replace by better function without float
 */
void DWM1000Time::setTime(int32_t value, float factorUs) {
	//float tsValue = value*factorUs;
	//tsValue = fmod(tsValue, TIME_OVERFLOW);
	//setTime(tsValue);
	setTime(value*factorUs);
}

/**
 * Get timestamp as integer
 * @return timestamp as integer
 */
int64_t DWM1000Time::getTimestamp() const {
	return _timestamp;
}

/**
 * Get timestamp as byte array
 * @param data var where data should be written
 */
void DWM1000Time::getTimestamp(byte data[]) const {
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
float DWM1000Time::getAsFloat() const {
	//return fmod((float)_timestamp, TIME_OVERFLOW)*TIME_RES;
	return getAsMicroSeconds();
}

/**
 * Return real time in micro seconds
 * @return time in micro seconds
 */
float DWM1000Time::getAsMicroSeconds() const {
	return (_timestamp%TIME_OVERFLOW)*TIME_RES;
}

/**
 * Return time as distance in meter, d=c*t
 * this is useful for e.g. time of flight
 * @return distance in meters
 */
float DWM1000Time::getAsMeters() const {
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
DWM1000Time& DWM1000Time::wrap() {
	if(_timestamp < 0) {
		_timestamp += TIME_OVERFLOW;
	}
	return *this;
}

/**
 * Check if timestamp is valid for usage with DWM1000 device
 * @return true if valid, false if negative or overflow (maybe after calculation)
 */
bool DWM1000Time::isValidTimestamp() {
	return (0 <= _timestamp && _timestamp <= TIME_MAX);
}

// assign
DWM1000Time& DWM1000Time::operator=(const DWM1000Time& assign) {
	if(this == &assign) {
		return *this;
	}
	_timestamp = assign.getTimestamp();
	return *this;
}

// add
DWM1000Time& DWM1000Time::operator+=(const DWM1000Time& add) {
	_timestamp += add.getTimestamp();
	return *this;
}

DWM1000Time DWM1000Time::operator+(const DWM1000Time& add) const {
	return DWM1000Time(*this) += add;
}

// subtract
DWM1000Time& DWM1000Time::operator-=(const DWM1000Time& sub) {
	_timestamp -= sub.getTimestamp();
	return *this;
}

DWM1000Time DWM1000Time::operator-(const DWM1000Time& sub) const {
	return DWM1000Time(*this) -= sub;
}

// multiply
DWM1000Time& DWM1000Time::operator*=(float factor) {
	//float tsValue = (float)_timestamp*factor;
	//_timestamp = (int64_t)tsValue;
	_timestamp *= factor;
	return *this;
}

DWM1000Time DWM1000Time::operator*(float factor) const {
	return DWM1000Time(*this) *= factor;
}

DWM1000Time& DWM1000Time::operator*=(const DWM1000Time& factor) {
	_timestamp *= factor.getTimestamp();
	return *this;
}

DWM1000Time DWM1000Time::operator*(const DWM1000Time& factor) const {
	return DWM1000Time(*this) *= factor;
}

// divide
DWM1000Time& DWM1000Time::operator/=(float factor) {
	//_timestamp *= (1.0f/factor);
	_timestamp /= factor;
	return *this;
}

DWM1000Time DWM1000Time::operator/(float factor) const {
	return DWM1000Time(*this) /= factor;
}

DWM1000Time& DWM1000Time::operator/=(const DWM1000Time& factor) {
	_timestamp /= factor.getTimestamp();
	return *this;
}

DWM1000Time DWM1000Time::operator/(const DWM1000Time& factor) const {
	return DWM1000Time(*this) /= factor;
}

// compare
boolean DWM1000Time::operator==(const DWM1000Time& cmp) const {
	return _timestamp == cmp.getTimestamp();
}

boolean DWM1000Time::operator!=(const DWM1000Time& cmp) const {
	//return !(*this == cmp); // seems not as intended
	return _timestamp != cmp.getTimestamp();
}

#ifdef DWM1000TIME_H_PRINTABLE
/**
 * For debuging, print timestamp pretty as integer with arduinos serial
 * @deprecated use Serial.print(object)
 */
void DWM1000Time::print() {
	Serial.print(*this);
	Serial.println();
}

/**
 * Print timestamp of instance as integer with e.g. Serial.print()
 * @param p printer instance
 * @return size of printed chars
 */
size_t DWM1000Time::printTo(Print& p) const {
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
#endif // DWM1000Time_H_PRINTABLE
