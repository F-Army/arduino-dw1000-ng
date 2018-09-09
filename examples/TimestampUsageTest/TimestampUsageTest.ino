/*  
 *	Arduino-DWM1000 - Arduino library to use Decawave's DWM1000 module.
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

/**
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>
 * Copyright (c) 2016 by Ludwig Grill (www.rotzbua.de); extended example
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
 * @file TimestampUsageTest.ino
 * This is a simple unit test for the DWM1000Time class. This test
 * has no actual use to the operation of the DW1000.
 */

#include <SPI.h>
#include <DWM1000Time.hpp>

void setup() {
  Serial.begin(9600);
  Serial.println(F("### DWM1000Time-arduino-test ###"));
}

void loop() {
  // variables for the test
  DWM1000Time time1, time2, time3, time4;
  byte stamp[DWM1000Time::LENGTH_TIMESTAMP];
  
  // unit test
  Serial.println(F("simple test for + - "));
  Serial.print(F("Time1 is   (0)[us] ... ")); Serial.println(time1.getAsMicroSeconds(), 4);
  time1 += DWM1000Time(10, DWM1000Time::MICROSECONDS);
  Serial.print(F("Time1 is  (10)[us] ... ")); Serial.println(time1.getAsMicroSeconds(), 4);
  time1 -= DWM1000Time(500, DWM1000Time::NANOSECONDS);
  Serial.print(F("Time1 is (9.5)[us] ... ")); Serial.println(time1.getAsMicroSeconds(), 4);
  Serial.println();
  
  Serial.println(F("test compare"));
  time2 = time1;
  time2 += DWM1000Time(10.0f);
  Serial.print(F("Time2 == Time1  (NO)... ")); Serial.println(time1 == time2 ? "YES" : "NO");
  time1 += DWM1000Time(10000, DWM1000Time::NANOSECONDS);
  Serial.print(F("Time2 == Time1 (YES)... ")); Serial.println(time1 == time2 ? "YES" : "NO");
  Serial.println();
  
  Serial.println(F("test different output"));
  memset(stamp, 0, DWM1000Time::LENGTH_TIMESTAMP);
  stamp[1] = 0x02; // = 512
  time2 = DWM1000Time(stamp);
  Serial.print(F("Time2 is             (512) ... ")); Serial.println(time2);
  Serial.print(F("Time2 is      (0.0080)[us] ... ")); Serial.println(time2.getAsMicroSeconds(), 4);
  Serial.print(F("Time2 range is (2.4022)[m] ... ")); Serial.println(time2.getAsMeters(), 4);
  time3 = DWM1000Time(10, DWM1000Time::SECONDS);
  time3.getTimestamp(stamp);
  time3.setTimestamp(stamp);
  Serial.print(F("Time3 is (10)[s]           ... ")); Serial.println(time3.getAsMicroSeconds() * 1.0e-6, 4);
  Serial.println();
  
  Serial.println(F("test negativ value"));
  memset(stamp, 0, DWM1000Time::LENGTH_TIMESTAMP);
  time2.setTimestamp(-512);
  Serial.print(F("Time2 is             (-512) ... ")); Serial.println(time2);
  Serial.print(F("Time2 is      (-0.0080)[us] ... ")); Serial.println(time2.getAsMicroSeconds(), 4);
  Serial.print(F("Time2 range is (-2.4022)[m] ... ")); Serial.println(time2.getAsMeters(), 4);
  Serial.println();
  
  Serial.println(F("test calculation"));
  time1.setTimestamp(1536);
  time2.setTimestamp(512);
  time3.setTimestamp(4);
  time4 = (time1 + time2) / time3;
  Serial.print(F("Time4 is (512) ... ")); Serial.println(time4);
  time4 = (time1 - time2) / time3;
  Serial.print(F("Time4 is (256) ... ")); Serial.println(time4);
  time4 = (time1 * time3 * time2 - time2 * time3 * time2) / (time2 * time2);
  Serial.print(F("Time4 is   (8) ... ")); Serial.println(time4);
  Serial.println();
  
  Serial.println(F("test valid/maxima"));
  time1.setTimestamp(DWM1000Time::TIME_MAX);
  time2.setTimestamp(DWM1000Time::TIME_MAX);
  time3.setTimestamp(2);
  time4 = (time1 + time2);
  Serial.print(F("Time4 is valid?    (NO) ... ")); Serial.println(time4.isValidTimestamp() ? "YES" : "NO");
  Serial.print(F("Time4 is TIME_MAX  (NO) ... ")); Serial.println(time4 == DWM1000Time::TIME_MAX ? "YES" : "NO");
  time4 /= time3;
  Serial.print(F("Time4 is valid?   (YES) ... ")); Serial.println(time4.isValidTimestamp() ? "YES" : "NO");
  Serial.print(F("Time4 is TIME_MAX (YES) ... ")); Serial.println(time4 == DWM1000Time::TIME_MAX ? "YES" : "NO");
  
  Serial.println();
  
  // keep calm
  delay(10000);
}
