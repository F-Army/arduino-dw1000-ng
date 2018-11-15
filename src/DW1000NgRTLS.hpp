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
constexpr uint16_t RTLS_APP_ID = RTLS_APP_ID_LOW | ((uint16_t) (RTLS_APP_ID_HIGH << 8));

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

/* BLINK Encoding Header */
constexpr byte BATTERY_GOOD = 0x00;
constexpr byte BATTERY_10_30_PERCENT = 0x02;
constexpr byte BATTERY_0_10_PERCENT = 0x01;
constexpr byte NO_BATTERY_STATUS = 0x03;

constexpr byte TEMPERATURE_DATA = 0x20;

constexpr byte EX_ID = 0x80;
constexpr byte NO_EX_ID = 0x40;

/* BLINK Ext Header */
constexpr byte BLINK_RATE_AND_LISTENING = 0x01;
constexpr byte TAG_LISTENING_NOW = 0x02;

enum class NextActivity {
    ACTIVITY_FINISHED,
    RANGING_CONFIRM
};

namespace DW1000NgRTLS {
    void transmitShortBlink();
    void transmitRangingInitiation(byte tag_eui[], byte tag_short_address[]);
    void transmitPoll(byte anchor_address[]);
    void transmitResponseToPoll(byte tag_short_address[]);
    void transmitFinalMessage(byte anchor_address[], uint16_t reply_delay, uint64_t timePollSent, uint64_t timeResponseToPollReceived);
    void transmitRangingConfirm(byte tag_short_address[], byte next_anchor[]);
    void transmitActivityFinished(byte tag_short_address[], byte blink_rate[]);

    uint32_t handleActivityFinished(byte frame[]);
    void handleRangingInitiation(byte frame[]);
    void handleRangingConfirm(byte frame[]);
    void handleRangingContinueEmbedded(byte frame[], uint16_t replyDelayUs);
    uint64_t handlePoll(byte frame[], byte tagShortAddress[]);
    double handleFinalMessageEmbedded(byte frame[], uint64_t timePollReceived, NextActivity next, byte param[]);
}