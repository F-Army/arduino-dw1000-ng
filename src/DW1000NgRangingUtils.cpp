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

namespace DW1000NgRangingUtils {

    /* asymmetric two-way ranging (more computation intense, less error prone) */
    double computeRangeAsymmetric(    
                                    uint64_t timePollSent, 
                                    uint64_t timePollReceived, 
                                    uint64_t timePollAckSent, 
                                    uint64_t timePollAckReceived,
                                    uint64_t timeRangeSent,
                                    uint64_t timeRangeReceived
                                )
    {
        
        uint32_t timePollSent_32 = static_cast<uint32_t>(timePollSent);
        uint32_t timePollReceived_32 = static_cast<uint32_t>(timePollReceived);
        uint32_t timePollAckSent_32 = static_cast<uint32_t>(timePollAckSent);
        uint32_t timePollAckReceived_32 = static_cast<uint32_t>(timePollAckReceived);
        uint32_t timeRangeSent_32 = static_cast<uint32_t>(timeRangeSent);
        uint32_t timeRangeReceived_32 = static_cast<uint32_t>(timeRangeReceived);
        
        double round1 = static_cast<double>(timePollAckReceived_32 - timePollSent_32);
        double reply1 = static_cast<double>(timePollAckSent_32 - timePollReceived_32);
        double round2 = static_cast<double>(timeRangeReceived_32 - timePollAckSent_32);
        double reply2 = static_cast<double>(timeRangeSent_32 - timePollAckReceived_32);

        int64_t tof_uwb = static_cast<int64_t>((round1 * round2 - reply1 * reply2) / (round1 + round2 + reply1 + reply2));
        double distance = tof_uwb * DISTANCE_OF_RADIO;
    
        return distance;
    }
}
