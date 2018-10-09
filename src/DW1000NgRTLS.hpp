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

constexpr size_t FRAME_CONTROL_OFFSET = 0;
constexpr size_t FRAME_CONTROL_SIZE = 2;
constexpr size_t SEQUENCE_NUMBER_OFFSET = 2;
constexpr size_t SEQUENCE_NUMBER_SIZE = 1;
constexpr size_t APPLICATION_ID_OFFSET = 3;
constexpr size_t APPLICATION_ID_SIZE = 2;
constexpr size_t DESTINATION_OFFSET = 5;

enum class addressType : size_t {
    SHORT = 2,
    LONG = 8
};

enum class frameType {
    BLINK,
    DATA,
    OTHER
};

typedef struct message_data_settings_t {
    byte functionCode;
    byte *data;
    size_t dataLength;
} message_data_settings_t;

typedef struct data_offset_t {
    size_t source_offset;
    size_t application_offset;
} data_offset_t;

namespace DW1000Ng {
    void encodeMinimalBlink();
    //void encodeLongBlink(byte data[], size_t len);
    void encodeData(byte source[], addressType src_len, byte destination[], addressType dest_len, message_data_settings_t &data_settings);
    
    frameType getFrameType(byte frame[]);
    
    data_offset_t getDataOffsets(byte frame[]);
}