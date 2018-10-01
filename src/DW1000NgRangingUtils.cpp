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

#include "DW1000NgRangingUtils.hpp"
#include "DW1000NgTime.hpp"

namespace DW1000NgRangingUtils {
    /*

    DW1000NgTime computeRangeAsymmetric(    
                                        DW1000NgTime &timePollSent, 
                                        DW1000NgTime &timePollReceived, 
                                        DW1000NgTime &timePollAckSent, 
                                        DW1000NgTime &timePollAckReceived,
                                        DW1000NgTime &timeRangeSent,
                                        DW1000NgTime &timeRangeReceived 
                                    ) 
    {
        // asymmetric two-way ranging (more computation intense, less error prone)
        DW1000NgTime round1 = (timePollAckReceived - timePollSent).wrap();
        DW1000NgTime reply1 = (timePollAckSent - timePollReceived).wrap();
        DW1000NgTime round2 = (timeRangeReceived - timePollAckSent).wrap();
        DW1000NgTime reply2 = (timeRangeSent - timePollAckReceived).wrap();
        DW1000NgTime tof = (round1 * round2 - reply1 * reply2) / (round1 + round2 + reply1 + reply2);
        // set tof timestamp
        DW1000NgTime timeComputedRange;
        timeComputedRange.setTimestamp(tof);
        return timeComputedRange;
    }

    DW1000NgTime computeRangeSymmetric(    
                                        DW1000NgTime &timePollSent, 
                                        DW1000NgTime &timePollReceived, 
                                        DW1000NgTime &timePollAckSent, 
                                        DW1000NgTime &timePollAckReceived,
                                        DW1000NgTime &timeRangeSent,
                                        DW1000NgTime &timeRangeReceived 
                                    ) 
    {
        // symmetric two-way ranging (less computation intense, more error prone on clock drift)
        DW1000NgTime tof = ((timePollAckReceived - timePollSent) - (timePollAckSent - timePollReceived) +
                        (timeRangeReceived - timePollAckSent) - (timeRangeSent - timePollAckReceived)) * 0.25f;
        // set tof timestamp
        DW1000NgTime timeComputedRange;
        timeComputedRange.setTimestamp(tof);
        return timeComputedRange;
    }
    */

}
