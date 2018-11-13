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

#include <Arduino.h>
#include "DW1000NgRTLS.hpp"
#include "DW1000Ng.hpp"

namespace DW1000NgRTLS {
    void transmitShortBlink() {
        byte Blink[] = {BLINK, DW1000Ng::getTxSequenceNumber(), 0,0,0,0,0,0,0,0, NO_BATTERY_STATUS | NO_EX_ID, TAG_LISTENING_NOW};
        DW1000Ng::getEUI(&Blink[2]);
        DW1000Ng::setTransmitData(Blink, sizeof(Blink));
        DW1000Ng::startTransmit();
    }

    void transmitRangingInitiation(uint64_t tag_eui, uint16_t tag_short_address);
    void transmitPoll(uint16_t anchor_address);
    void transmitResponseToPoll(uint16_t tag_short_address);
    void transmitFinalMessage(uint16_t anchor_address, uint16_t reply_delay, uint64_t timePollSent, uint64_t timeResponseToPollReceived);
    void transmitRangingConfirm(uint16_t tag_short_address, uint16_t next_anchor);
    void transmitActivityFinished(uint16_t tag_short_address, uint16_t blink_rate);
}