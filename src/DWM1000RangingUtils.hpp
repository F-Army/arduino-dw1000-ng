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

#pragma once

#include "DWM1000Time.hpp"

namespace DWM1000RangingUtils {
    /*
 * RANGING ALGORITHMS
 * ------------------
 * Either of the below functions can be used for range computation (see line "CHOSEN
 * RANGING ALGORITHM" in the code).
 * - Asymmetric is more computation intense but least error prone
 * - Symmetric is less computation intense but more error prone to clock drifts
 *
 * The anchors and tags of this reference example use the same reply delay times, hence
 * are capable of symmetric ranging (and of asymmetric ranging anyway).
 */


    DWM1000Time computeRangeAsymmetric(    
                                        DWM1000Time &timePollSent, 
                                        DWM1000Time &timePollReceived, 
                                        DWM1000Time &timePollAckSent, 
                                        DWM1000Time &timePollAckReceived,
                                        DWM1000Time &timeRangeSent,
                                        DWM1000Time &timeRangeReceived 
                                    );

    DWM1000Time computeRangeSymmetric(    
                                        DWM1000Time &timePollSent, 
                                        DWM1000Time &timePollReceived, 
                                        DWM1000Time &timePollAckSent, 
                                        DWM1000Time &timePollAckReceived,
                                        DWM1000Time &timeRangeSent,
                                        DWM1000Time &timeRangeReceived 
                                    );
}