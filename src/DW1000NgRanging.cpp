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
#include "DW1000Ng.hpp"
#include "DW1000NgConstants.hpp"
#include "DW1000NgRanging.hpp"
#include "DW1000NgConstants.hpp"
#include "DW1000NgRTLS.hpp"

namespace DW1000NgRanging {

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
        uint64_t round1 = timePollAckReceived - timePollSent;
        uint64_t reply1 = timePollAckSent - timePollReceived;
        uint64_t round2 = timeRangeReceived - timePollAckSent;
        uint64_t reply2 = timeRangeSent - timePollAckReceived;

        int64_t dividend = round1 * round2 - reply1 * reply2;
        uint64_t divisor = round1 + round2 + reply1 + reply2;

        /* Work around a bug in the GCC shipped with the ESP32 IDF for Arduino
           that fails to preserve sign on int64_t division and would therefore
           hide measurement inconsistencies elsewhere in the design.  */
        bool isNegative = false;
        if (dividend < 0) {
            isNegative = true;
            dividend = -dividend;
        }

        int64_t tof_uwb = dividend / divisor;

        /* Reapply preserved sign */
        if (isNegative) tof_uwb = -tof_uwb;

        double distance = tof_uwb * DISTANCE_OF_RADIO;

        return distance;
    }

    double correctRange(double range) {
        double result = 0;

        Channel currentChannel = DW1000Ng::getChannel();
        double rxPower = -(static_cast<double>(DW1000Ng::getReceivePower()));
        
        size_t index = DW1000Ng::getPulseFrequency() == PulseFrequency::FREQ_16MHZ ? 1 : 2;
        if(currentChannel == Channel::CHANNEL_4 || currentChannel == Channel::CHANNEL_7)
            index+=2;
        
        if (rxPower < BIAS_TABLE[0][0]) {
            result = range += BIAS_TABLE[0][index]*0.001;
        } else if (rxPower >= BIAS_TABLE[17][0]) {
            result = range += BIAS_TABLE[17][index]*0.001;
        } else {
            for(auto i=0; i < 17; i++) {
                if (rxPower >= BIAS_TABLE[i][0] && rxPower < BIAS_TABLE[i+1][0]){
                    result = range += BIAS_TABLE[i][index]*0.001;
                    break;
                }
            }
        }

        return result;
    }

}
