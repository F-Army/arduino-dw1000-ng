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

#pragma once

#include "DW1000NgTime.hpp"

namespace DW1000NgRangingUtils {
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


    DW1000NgTime computeRangeAsymmetric(    
                                        DW1000NgTime &timePollSent, 
                                        DW1000NgTime &timePollReceived, 
                                        DW1000NgTime &timePollAckSent, 
                                        DW1000NgTime &timePollAckReceived,
                                        DW1000NgTime &timeRangeSent,
                                        DW1000NgTime &timeRangeReceived 
                                    );

    DW1000NgTime computeRangeSymmetric(    
                                        DW1000NgTime &timePollSent, 
                                        DW1000NgTime &timePollReceived, 
                                        DW1000NgTime &timePollAckSent, 
                                        DW1000NgTime &timePollAckReceived,
                                        DW1000NgTime &timeRangeSent,
                                        DW1000NgTime &timeRangeReceived 
                                    );
}