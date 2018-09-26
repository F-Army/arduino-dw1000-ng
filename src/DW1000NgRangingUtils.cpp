/*  
 *	Arduino-DW1000Ng - Arduino library to use Decawave's DW1000Ng module.
 *	Copyright (C) 2018  Michele Biondi <michelebiondi01@gmail.com>, Andrea Salvatori <andrea.salvatori92@gmail.com>
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DW1000NgRangingUtils.hpp"
#include "DW1000NgTime.hpp"

namespace DW1000NgRangingUtils {

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

}
