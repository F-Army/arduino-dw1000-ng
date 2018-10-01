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

    /*
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
    */
}