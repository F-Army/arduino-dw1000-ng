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

    double correctRange(double range) {
        double rxPower = -(static_cast<double>(DW1000Ng::getReceivePower()));
        double mmToCorrectRange = 0;
        int currentMatrixPosition = 0;
        Channel currentChannel = DW1000Ng::getChannel();
        PulseFrequency currentPRF = DW1000Ng::getPulseFrequency();
        if(currentChannel == Channel::CHANNEL_4 || currentChannel == Channel::CHANNEL_7) {
            if(currentPRF == PulseFrequency::FREQ_16MHZ) {

            } else if(currentPRF == PulseFrequency::FREQ_64MHZ) {

            }
        } else {
            // 500 MHz receiver bandwidth
            if(currentPRF == PulseFrequency::FREQ_16MHZ) {
                if (rxPower < BIAS_500_16_1[0][0]) {
                    mmToCorrectRange += BIAS_500_16_1[0][1];
                } else if (rxPower > BIAS_500_16_1[17][0]) {
                    mmToCorrectRange += BIAS_500_16_1[17][1];
                    currentMatrixPosition = 17;
                } else {
                    for(auto i=0; i < 18; i++){
                        if (rxPower == BIAS_500_16_1[i][0]){
                            mmToCorrectRange += BIAS_500_16_1[i][0];
                            currentMatrixPosition = i;
                        } else if (rxPower > BIAS_500_16_1[i][0] && rxPower < BIAS_500_16_1[i+1][0] ){
                            mmToCorrectRange += BIAS_500_16_1[i][1];
                            currentMatrixPosition = i;
                        }
                    }
                }
            } else if(currentPRF == PulseFrequency::FREQ_64MHZ) {

            }
        }

        if (currentMatrixPosition < 10){
            range -= (mmToCorrectRange*0.001);
            //Serial.println("CORREGGO il RANGE con -");
            //Serial.println(mmToCorrectRange);
        } else if (currentMatrixPosition < 10) {
            range += (mmToCorrectRange*0.001);
            //Serial.println("CORREGGO il RANGE con +");
            //Serial.println(mmToCorrectRange);
        }

        return range;
    }
/*
    double correctRange(double range) {
        // base line dBm, which is -61, 2 dBm steps, total 18 data points (down to -95 dBm)
        double rxPowerBase     = -(DW1000Ng::getReceivePower()+61.0f)*0.5f;
        int16_t   rxPowerBaseLow  = (int16_t)rxPowerBase; // TODO check type
        int16_t   rxPowerBaseHigh = rxPowerBaseLow+1; // TODO check type
        if(rxPowerBaseLow <= 0) {
            rxPowerBaseLow  = 0;
            rxPowerBaseHigh = 0;
        } else if(rxPowerBaseHigh >= 17) {
            rxPowerBaseLow  = 17;
            rxPowerBaseHigh = 17;
        }
        // select range low/high values from corresponding table
        int16_t rangeBiasHigh;
        int16_t rangeBiasLow;
        Channel currentChannel = DW1000Ng::getChannel();
        PulseFrequency currentPRF = DW1000Ng::getPulseFrequency();
        if(currentChannel == Channel::CHANNEL_4 || currentChannel == Channel::CHANNEL_7) {
            // 900 MHz receiver bandwidth
            if(currentPRF == PulseFrequency::FREQ_16MHZ) {
                rangeBiasHigh = (rxPowerBaseHigh < BIAS_900_16_ZERO ? -BIAS_900_16[rxPowerBaseHigh] : BIAS_900_16[rxPowerBaseHigh]);
                rangeBiasHigh <<= 1;
                rangeBiasLow  = (rxPowerBaseLow < BIAS_900_16_ZERO ? -BIAS_900_16[rxPowerBaseLow] : BIAS_900_16[rxPowerBaseLow]);
                rangeBiasLow <<= 1;
            } else if(currentPRF == PulseFrequency::FREQ_64MHZ) {
                rangeBiasHigh = (rxPowerBaseHigh < BIAS_900_64_ZERO ? -BIAS_900_64[rxPowerBaseHigh] : BIAS_900_64[rxPowerBaseHigh]);
                rangeBiasHigh <<= 1;
                rangeBiasLow  = (rxPowerBaseLow < BIAS_900_64_ZERO ? -BIAS_900_64[rxPowerBaseLow] : BIAS_900_64[rxPowerBaseLow]);
                rangeBiasLow <<= 1;
            }
        } else {
            // 500 MHz receiver bandwidth
            if(currentPRF == PulseFrequency::FREQ_16MHZ) {
                rangeBiasHigh = (rxPowerBaseHigh < BIAS_500_16_ZERO ? -BIAS_500_16[rxPowerBaseHigh] : BIAS_500_16[rxPowerBaseHigh]);
                rangeBiasLow  = (rxPowerBaseLow < BIAS_500_16_ZERO ? -BIAS_500_16[rxPowerBaseLow] : BIAS_500_16[rxPowerBaseLow]);
            } else if(currentPRF == PulseFrequency::FREQ_64MHZ) {
                rangeBiasHigh = (rxPowerBaseHigh < BIAS_500_64_ZERO ? -BIAS_500_64[rxPowerBaseHigh] : BIAS_500_64[rxPowerBaseHigh]);
                rangeBiasLow  = (rxPowerBaseLow < BIAS_500_64_ZERO ? -BIAS_500_64[rxPowerBaseLow] : BIAS_500_64[rxPowerBaseLow]);
            }
        }
        // linear interpolation of bias values
        double rangeBias = rangeBiasLow+(rxPowerBase-rxPowerBaseLow)*(rangeBiasHigh-rangeBiasLow);
        // range bias [mm] to timestamp modification value conversion
        range -= (rangeBias*0.001);
        return range;
    }
    */
}
