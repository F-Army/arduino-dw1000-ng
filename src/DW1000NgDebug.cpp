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
 * Debug utilities.
 */

#include <Arduino.h>
#include "DW1000Ng.hpp"

namespace DW1000NgDebug {
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
        DW1000Ng::readBytes(cmd, offset, readBuf, n);
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