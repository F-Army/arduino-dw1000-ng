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

#pragma once

#include <Arduino.h>

/* Frame control */
constexpr byte BLINK = 0xC5;
constexpr byte DATA = 0x41;
constexpr byte SHORT_SRC_AND_DEST = 0x88;
constexpr byte LONG_SRC_AND_DEST = 0xCC;
constexpr byte SHORT_SRC_LONG_DEST = 0x8C;
constexpr byte LONG_SRC_SHORT_DEST = 0xC8;

/* Application ID */
constexpr byte RTLS_APP_ID_LOW = 0x9A;
constexpr byte RTLS_APP_ID_HIGH = 0x60;

/* Function code */
constexpr byte ACTIVITY_CONTROL = 0x10;
constexpr byte RANGING_INITIATION = 0x20;
constexpr byte RANGING_TAG_POLL = 0x21;
constexpr byte RANGING_TAG_FINAL_RESPONSE_EMBEDDED = 0x23;
constexpr byte RANGING_TAG_FINAL_RESPONSE_NO_EMBEDDED = 0x25;
constexpr byte RANGING_TAG_FINAL_SEND_TIME = 0x27;

/* Activity code */
constexpr byte ACTIVITY_FINISHED = 0x00;
constexpr byte RANGING_CONFIRM = 0x01;
constexpr byte RANGING_CONTINUE = 0x02;