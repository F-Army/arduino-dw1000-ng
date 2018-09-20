/*  
 *	Arduino-DWM1000 - Arduino library to use Decawave's DWM1000 module.
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

#include "DWM1000RangingUtils.hpp"
#include "DWM1000Time.hpp"

namespace DWM1000RangingUtils {

    DWM1000Time computeRangeAsymmetric(    
                                        DWM1000Time &timePollSent, 
                                        DWM1000Time &timePollReceived, 
                                        DWM1000Time &timePollAckSent, 
                                        DWM1000Time &timePollAckReceived,
                                        DWM1000Time &timeRangeSent,
                                        DWM1000Time &timeRangeReceived 
                                    ) 
    {
        // asymmetric two-way ranging (more computation intense, less error prone)
        DWM1000Time round1 = (timePollAckReceived - timePollSent).wrap();
        DWM1000Time reply1 = (timePollAckSent - timePollReceived).wrap();
        DWM1000Time round2 = (timeRangeReceived - timePollAckSent).wrap();
        DWM1000Time reply2 = (timeRangeSent - timePollAckReceived).wrap();
        DWM1000Time tof = (round1 * round2 - reply1 * reply2) / (round1 + round2 + reply1 + reply2);
        // set tof timestamp
        DWM1000Time timeComputedRange;
        timeComputedRange.setTimestamp(tof);
        return timeComputedRange;
    }

    DWM1000Time computeRangeSymmetric(    
                                        DWM1000Time &timePollSent, 
                                        DWM1000Time &timePollReceived, 
                                        DWM1000Time &timePollAckSent, 
                                        DWM1000Time &timePollAckReceived,
                                        DWM1000Time &timeRangeSent,
                                        DWM1000Time &timeRangeReceived 
                                    ) 
    {
        // symmetric two-way ranging (less computation intense, more error prone on clock drift)
        DWM1000Time tof = ((timePollAckReceived - timePollSent) - (timePollAckSent - timePollReceived) +
                        (timeRangeReceived - timePollAckSent) - (timeRangeSent - timePollAckReceived)) * 0.25f;
        // set tof timestamp
        DWM1000Time timeComputedRange;
        timeComputedRange.setTimestamp(tof);
        return timeComputedRange;
    }

}
