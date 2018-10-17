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

namespace DW1000NgRanging {

    /** 
    Asymmetric two-way ranging algorithm (more computation intense, less error prone) 
    
    @param [in] timePollSent timestamp of poll transmission
    @param [in] timePollReceived timestamp of poll receive
    @param [in] timePollAckSent timestamp of response to poll transmission
    @param [in] timePollAckReceived timestamp of response to poll receive
    @param [in] timeRangeSent timestamp of final message transmission
    @param [in] timeRangeReceived timestamp of final message receive

    returns the range in meters
    */
    double computeRangeAsymmetric(    
                                        uint64_t timePollSent, 
                                        uint64_t timePollReceived, 
                                        uint64_t timePollAckSent, 
                                        uint64_t timePollAckReceived,
                                        uint64_t timeRangeSent,
                                        uint64_t timeRangeReceived 
                                 );
    //TODO Symmetric

    /**
    Removes bias from the target range
    
    returns the unbiased range
    */
    double correctRange(double range);
}