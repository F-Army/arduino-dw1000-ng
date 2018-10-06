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
#include "DW1000NgUtils.hpp"
#include "DW1000NgRanging.hpp"
#include "DW1000NgConstants.hpp"
#include "DW1000NgTime.hpp"

namespace {
    byte PollTxTime[4];
    byte RespRxTime[4];
}

namespace DW1000NgRanging {

    void encodePoll(byte source[], addressType src_len, byte destination[], addressType dest_len) {
        message_data_settings_t poll {
            0x21,
            nullptr,
            0
        };

        DW1000Ng::encodeData(source, src_len, destination, dest_len, poll);
    }

    void encodeResponseToPoll(byte source[], addressType src_len, byte destination[], addressType dest_len) {
        byte data[3] = {0x02, 0, 0};
        message_data_settings_t pollAck {
            0x10,
            data,
            sizeof(data)
        };

        DW1000Ng::encodeData(source, src_len, destination, dest_len, pollAck);
    }

    void encodeFinalMessage(byte source[], addressType src_len, byte destination[], addressType dest_len, uint16_t replyDelayTimeUS) {
        byte embedFinalTxTime[LENGTH_TIMESTAMP];

	    uint64_t timeRangeSent = DW1000Ng::getSystemTimestamp();
	    timeRangeSent += DW1000NgTime::microsecondsToUWBTime(replyDelayTimeUS);
        DW1000NgUtils::writeValueToBytes(embedFinalTxTime, timeRangeSent, LENGTH_TIMESTAMP);
        DW1000Ng::setDelayedTRX(embedFinalTxTime);
        timeRangeSent += DW1000Ng::getTxAntennaDelay();
        DW1000NgUtils::writeValueToBytes(embedFinalTxTime, static_cast<uint32_t>(timeRangeSent), 4);

        byte data[12];
        memcpy(data, PollTxTime, 4);
        memcpy(&data[4], RespRxTime, 4);
        memcpy(&data[8], embedFinalTxTime, 4);

        message_data_settings_t finalMessage {
            0x23,
            data,
            sizeof(data)
        };

        DW1000Ng::encodeData(source, src_len, destination, dest_len, finalMessage);
    }

    void encodeFinalMessageNoEmbedding(byte source[], addressType src_len, byte destination[], addressType dest_len) {
        byte data[8];
        memcpy(data, PollTxTime, 4);
        memcpy(&data[4], RespRxTime, 4);

        message_data_settings_t finalMessageNoEmbedding {
            0x25,
            data,
            sizeof(data)
        };

        DW1000Ng::encodeData(source, src_len, destination, dest_len, finalMessageNoEmbedding);
    }

    void encodeFinalSendTimeMessage(byte source[], addressType src_len, byte destination[], addressType dest_len) {
        /* It is assumed there is no message in between as of standard ISO/IEC 24730-62_2013 */
        byte FinalTxTime[4];
        DW1000NgUtils::writeValueToBytes(FinalTxTime, static_cast<uint32_t>(DW1000Ng::getTransmitTimestamp()), 4);

        message_data_settings_t finalSendTimeMessage {
            0x27,
            FinalTxTime,
            sizeof(FinalTxTime)
        };

        DW1000Ng::encodeData(source, src_len, destination, dest_len, finalSendTimeMessage);
    }

    void encodeRangingConfirm(byte source[], addressType src_len, byte destination[], addressType dest_len, ranging_confirm_settings_t &settings) {
        byte data[3] = {static_cast<byte>(settings.activity), static_cast<byte>(settings.value & 0xFF), static_cast<byte>((settings.value >> 8) & 0xFF)};
        message_data_settings_t rangingConfirm {
            0x10,
            data,
            sizeof(data)
        };

        DW1000Ng::encodeData(source, src_len, destination, dest_len, rangingConfirm);
    }

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
        Channel currentChannel = DW1000Ng::getChannel();
        double rxPower = -(static_cast<double>(DW1000Ng::getReceivePower()));
        
        size_t index = DW1000Ng::getPulseFrequency() == PulseFrequency::FREQ_16MHZ ? 1 : 2;
        if(currentChannel == Channel::CHANNEL_4 || currentChannel == Channel::CHANNEL_7)
            index+=2;
        
        if (rxPower < BIAS_TABLE[0][0]) {
            return range += BIAS_TABLE[0][index]*0.001;
        } else if (rxPower >= BIAS_TABLE[17][0]) {
            return range += BIAS_TABLE[17][index]*0.001;
        } else {
            for(auto i=0; i < 17; i++) {
                if (rxPower >= BIAS_TABLE[i][0] && rxPower < BIAS_TABLE[i+1][0]){
                    return range += BIAS_TABLE[i][index]*0.001;
                }
            }
        }
    }

}
