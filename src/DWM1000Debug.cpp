/*  
 *  Arduino-DWM1000 - Arduino library to use Decawave's DWM1000 module.
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

/*
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>
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
 * @file DWM1000Debug.cpp
 * Debug utilities.
 */

#include <Arduino.h>
#include "DWM1000.hpp"

namespace DWM1000Debug {
    void getPrettyBytes(byte data[], char msgBuffer[], uint16_t n) {
        uint16_t i, j, b;
        b = sprintf(msgBuffer, "Data, bytes: %d\nB: 7 6 5 4 3 2 1 0\n", n); // TODO - type
        for(i = 0; i < n; i++) {
            byte curByte = data[i];
            snprintf(&msgBuffer[b++], 2, "%d", (i+1));
            msgBuffer[b++] = (char)((i+1) & 0xFF);
            msgBuffer[b++] = ':';
            msgBuffer[b++] = ' ';
            for(j = 0; j < 8; j++) {
                msgBuffer[b++] = ((curByte >> (7-j)) & 0x01) ? '1' : '0';
                if(j < 7) {
                    msgBuffer[b++] = ' ';
                } else if(i < n-1) {
                    msgBuffer[b++] = '\n';
                } else {
                    msgBuffer[b++] = '\0';
                }
            }
        }
        msgBuffer[b++] = '\0';
    }

    void getPrettyBytes(byte cmd, uint16_t offset, char msgBuffer[], uint16_t n) {
        uint16_t i, j, b;
        byte* readBuf = (byte*)malloc(n);
        DWM1000::readBytes(cmd, offset, readBuf, n);
        b     = sprintf(msgBuffer, "Reg: 0x%02x, bytes: %d\nB: 7 6 5 4 3 2 1 0\n", cmd, n);  // TODO - tpye
        for(i = 0; i < n; i++) {
            byte curByte = readBuf[i];
            snprintf(&msgBuffer[b++], 2, "%d", (i+1));
            msgBuffer[b++] = (char)((i+1) & 0xFF);
            msgBuffer[b++] = ':';
            msgBuffer[b++] = ' ';
            for(j = 0; j < 8; j++) {
                msgBuffer[b++] = ((curByte >> (7-j)) & 0x01) ? '1' : '0';
                if(j < 7) {
                    msgBuffer[b++] = ' ';
                } else if(i < n-1) {
                    msgBuffer[b++] = '\n';
                } else {
                    msgBuffer[b++] = '\0';
                }
            }
        }
        msgBuffer[b++] = '\0';
        free(readBuf);
    }
}