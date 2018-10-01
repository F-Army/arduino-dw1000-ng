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
#include "DW1000NgRangingUtils.hpp"
#include "DW1000NgConstants.hpp"

static int64_t overflowCorrectedValue(int64_t value) {
    if(value < 0) {
        Serial.print("Minore di 0");
        return value+TIME_OVERFLOW;
    } else {
        Serial.print("Maggiore di 0");
        return value;
    }
}

namespace DW1000NgRangingUtils {

    

    /* asymmetric two-way ranging (more computation intense, less error prone) */
    float computeRangeAsymmetric(    
                                    uint64_t timePollSent, 
                                    uint64_t timePollReceived, 
                                    uint64_t timePollAckSent, 
                                    uint64_t timePollAckReceived,
                                    uint64_t timeRangeSent,
                                    uint64_t timeRangeReceived 
                                ) 
    {
        int64_t round1 = overflowCorrectedValue((int64_t)timePollAckReceived - (int64_t)timePollSent);
        int64_t reply1 = overflowCorrectedValue((int64_t)timePollAckSent - (int64_t)timePollReceived);
        int64_t round2 = overflowCorrectedValue((int64_t)timeRangeReceived - (int64_t)timePollAckSent);
        int64_t reply2 = overflowCorrectedValue((int64_t)timeRangeSent - (int64_t)timePollAckReceived);
        int64_t tof = (round1 * round2 - reply1 * reply2) / (round1 + round2 + reply1 + reply2);
        return ((float) ( (tof%TIME_OVERFLOW) * DISTANCE_OF_RADIO));
    }


    /* symmetric two-way ranging (less computation intense, more error prone on clock drift) */
    /*
    float computeRangeSymmetric(    
                                    uint64_t timePollSent, 
                                    uint64_t timePollReceived, 
                                    uint64_t timePollAckSent, 
                                    uint64_t timePollAckReceived,
                                    uint64_t timeRangeSent,
                                    uint64_t timeRangeReceived 
                                ) 
    {
        uint64_t tof = ((timePollAckReceived - timePollSent) - (timePollAckSent - timePollReceived) +
                        (timeRangeReceived - timePollAckSent) - (timeRangeSent - timePollAckReceived)) * 0.25f;
        return ((float) ((float)tof * DISTANCE_OF_RADIO));
    }
    */

}
