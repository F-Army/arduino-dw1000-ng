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
#include "DW1000NgRTLS.hpp"
#include "DW1000Ng.hpp"
#include "DW1000NgUtils.hpp"
#include "DW1000NgTime.hpp"
#include "DW1000NgRanging.hpp"

static byte SEQ_NUMBER = 0;

namespace DW1000NgRTLS {

    byte increaseSequenceNumber(){
        return ++SEQ_NUMBER;
    }

    void transmitTwrShortBlink() {
        byte Blink[] = {BLINK, SEQ_NUMBER++, 0,0,0,0,0,0,0,0, NO_BATTERY_STATUS | NO_EX_ID, TAG_LISTENING_NOW};
        DW1000Ng::getEUI(&Blink[2]);
        DW1000Ng::setTransmitData(Blink, sizeof(Blink));
        DW1000Ng::startTransmit();
    }

    void transmitRangingInitiation(byte tag_eui[], byte tag_short_address[]) {
        byte RangingInitiation[] = {DATA, SHORT_SRC_LONG_DEST, SEQ_NUMBER++, 0,0, 0,0,0,0,0,0,0,0,  0,0, RANGING_INITIATION, 0,0};
        DW1000Ng::getNetworkId(&RangingInitiation[3]);
        memcpy(&RangingInitiation[5], tag_eui, 8);
        DW1000Ng::getDeviceAddress(&RangingInitiation[13]);
        memcpy(&RangingInitiation[16], tag_short_address, 2);
        DW1000Ng::setTransmitData(RangingInitiation, sizeof(RangingInitiation));
        DW1000Ng::startTransmit();
    }

    void transmitPoll(byte anchor_address[]){
        byte Poll[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0 , RANGING_TAG_POLL};
        DW1000Ng::getNetworkId(&Poll[3]);
        memcpy(&Poll[5], anchor_address, 2);
        DW1000Ng::getDeviceAddress(&Poll[7]);
        DW1000Ng::setTransmitData(Poll, sizeof(Poll));
        DW1000Ng::startTransmit();
    }

    void transmitResponseToPoll(byte tag_short_address[]) {
        byte pollAck[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0, ACTIVITY_CONTROL, RANGING_CONTINUE, 0, 0};
        DW1000Ng::getNetworkId(&pollAck[3]);
        memcpy(&pollAck[5], tag_short_address, 2);
        DW1000Ng::getDeviceAddress(&pollAck[7]);
        DW1000Ng::setTransmitData(pollAck, sizeof(pollAck));
        DW1000Ng::startTransmit();
    }

    void transmitFinalMessage(byte anchor_address[], uint16_t reply_delay, uint64_t timePollSent, uint64_t timeResponseToPollReceived) {
        /* Calculation of future time */
        byte futureTimeBytes[LENGTH_TIMESTAMP];

	    uint64_t timeFinalMessageSent = DW1000Ng::getSystemTimestamp();
	    timeFinalMessageSent += DW1000NgTime::microsecondsToUWBTime(reply_delay);
        DW1000NgUtils::writeValueToBytes(futureTimeBytes, timeFinalMessageSent, LENGTH_TIMESTAMP);
        DW1000Ng::setDelayedTRX(futureTimeBytes);
        timeFinalMessageSent += DW1000Ng::getTxAntennaDelay();

        byte finalMessage[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0, RANGING_TAG_FINAL_RESPONSE_EMBEDDED, 
            0,0,0,0,0,0,0,0,0,0,0,0
        };

        DW1000Ng::getNetworkId(&finalMessage[3]);
        memcpy(&finalMessage[5], anchor_address, 2);
        DW1000Ng::getDeviceAddress(&finalMessage[7]);

        DW1000NgUtils::writeValueToBytes(finalMessage + 10, (uint32_t) timePollSent, 4);
        DW1000NgUtils::writeValueToBytes(finalMessage + 14, (uint32_t) timeResponseToPollReceived, 4);
        DW1000NgUtils::writeValueToBytes(finalMessage + 18, (uint32_t) timeFinalMessageSent, 4);
        DW1000Ng::setTransmitData(finalMessage, sizeof(finalMessage));
        DW1000Ng::startTransmit(TransmitMode::DELAYED);
    }

    void transmitRangingConfirm(byte tag_short_address[], byte next_anchor[]) {
        byte rangingConfirm[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0, ACTIVITY_CONTROL, RANGING_CONFIRM, next_anchor[0], next_anchor[1]};
        DW1000Ng::getNetworkId(&rangingConfirm[3]);
        memcpy(&rangingConfirm[5], tag_short_address, 2);
        DW1000Ng::getDeviceAddress(&rangingConfirm[7]);
        DW1000Ng::setTransmitData(rangingConfirm, sizeof(rangingConfirm));
        DW1000Ng::startTransmit();
    }

    void transmitActivityFinished(byte tag_short_address[], byte blink_rate[]) {
        /* I send the new blink rate to the tag */
        byte rangingConfirm[] = {DATA, SHORT_SRC_AND_DEST, SEQ_NUMBER++, 0,0, 0,0, 0,0, ACTIVITY_CONTROL, ACTIVITY_FINISHED, blink_rate[0], blink_rate[1]};
        DW1000Ng::getNetworkId(&rangingConfirm[3]);
        memcpy(&rangingConfirm[5], tag_short_address, 2);
        DW1000Ng::getDeviceAddress(&rangingConfirm[7]);
        DW1000Ng::setTransmitData(rangingConfirm, sizeof(rangingConfirm));
        DW1000Ng::startTransmit();
    }

    static uint32_t calculateNewBlinkRate(byte frame[]) {
        uint32_t blinkRate = frame[11] + static_cast<uint32_t>(((frame[12] & 0x3F) << 8));
        byte multiplier = ((frame[12] & 0xC0) >> 6);
        if(multiplier  == 0x01) {
            blinkRate *= 25;
        } else if(multiplier == 0x02) {
            blinkRate *= 1000;
        }

        return blinkRate;
    }

    void waitForTransmission() {
        while(!DW1000Ng::isTransmitDone()) {
            #if defined(ESP8266)
            yield();
            #endif
        }
        DW1000Ng::clearTransmitStatus();
    }

    boolean receiveFrame() {
        DW1000Ng::startReceive();
        while(!DW1000Ng::isReceiveDone()) {
            if(DW1000Ng::isReceiveTimeout() ) {
                DW1000Ng::clearReceiveTimeoutStatus();
                return false;
            }
            #if defined(ESP8266)
            yield();
            #endif
        }
        DW1000Ng::clearReceiveStatus();
        return true;
    }

    static boolean waitForNextRangingStep() {
        DW1000NgRTLS::waitForTransmission();
        if(!DW1000NgRTLS::receiveFrame()) return false;
        return true;
    }

    RangeRequestResult tagRangeRequest() {
        DW1000NgRTLS::transmitTwrShortBlink();
        
        if(!DW1000NgRTLS::waitForNextRangingStep()) return {false, 0};

        size_t init_len = DW1000Ng::getReceivedDataLength();
        byte init_recv[init_len];
        DW1000Ng::getReceivedData(init_recv, init_len);

        if(!(init_len > 17 && init_recv[15] == RANGING_INITIATION)) {
            return { false, 0};
        }

        DW1000Ng::setDeviceAddress(DW1000NgUtils::bytesAsValue(&init_recv[16], 2));
        return { true, static_cast<uint16_t>(DW1000NgUtils::bytesAsValue(&init_recv[13], 2)) };
    }

    static RangeResult tagFinishRange(uint16_t anchor, uint16_t replyDelayUs) {
        RangeResult returnValue;

        byte target_anchor[2];
        DW1000NgUtils::writeValueToBytes(target_anchor, anchor, 2);
        DW1000NgRTLS::transmitPoll(target_anchor);
        /* Start of poll control for range */
        if(!DW1000NgRTLS::waitForNextRangingStep()) {
            returnValue = {false, false, 0, 0};
        } else {

            size_t cont_len = DW1000Ng::getReceivedDataLength();
            byte cont_recv[cont_len];
            DW1000Ng::getReceivedData(cont_recv, cont_len);

            if (cont_len > 10 && cont_recv[9] == ACTIVITY_CONTROL && cont_recv[10] == RANGING_CONTINUE) {
                /* Received Response to poll */
                DW1000NgRTLS::transmitFinalMessage(
                    &cont_recv[7], 
                    replyDelayUs, 
                    DW1000Ng::getTransmitTimestamp(), // Poll transmit time
                    DW1000Ng::getReceiveTimestamp()  // Response to poll receive time
                );

                if(!DW1000NgRTLS::waitForNextRangingStep()) {
                    returnValue = {false, false, 0, 0};
                } else {

                    size_t act_len = DW1000Ng::getReceivedDataLength();
                    byte act_recv[act_len];
                    DW1000Ng::getReceivedData(act_recv, act_len);

                    if(act_len > 10 && act_recv[9] == ACTIVITY_CONTROL) {
                        if (act_len > 12 && act_recv[10] == RANGING_CONFIRM) {
                            returnValue = {true, true, static_cast<uint16_t>(DW1000NgUtils::bytesAsValue(&act_recv[11], 2)), 0};
                        } else if(act_len > 12 && act_recv[10] == ACTIVITY_FINISHED) {
                            returnValue = {true, false, 0, calculateNewBlinkRate(act_recv)};
                        }
                    } else {
                        returnValue = {false, false, 0, 0};
                    }
                }
            } else {
                returnValue = {false, false, 0, 0};
            }
            
        }

        return returnValue;
    }

    RangeInfrastructureResult tagRangeInfrastructure(uint16_t target_anchor, uint16_t finalMessageDelay) {
        RangeInfrastructureResult returnValue;

        RangeResult result = tagFinishRange(target_anchor, finalMessageDelay);
        byte keep_going = 1;

        if(!result.success) {
            keep_going = 0;
            returnValue = {false , 0};
        } else {
            while(result.success && result.next) {
                result = tagFinishRange(result.next_anchor, finalMessageDelay);
                if(!result.success) {
                    keep_going = 0;
                    returnValue = {false , 0};
                    break;
                }

                #if defined(ESP8266)
                if (keep_going == 1) {
                    yield();
                }
                #endif
            }

            if (keep_going == 1) {

                if(result.success && result.new_blink_rate != 0) {
                    keep_going = 0;
                    returnValue = { true, static_cast<uint16_t>(result.new_blink_rate) };
                } else {
                    if(!result.success) {
                        keep_going = 0;
                        returnValue = { false , 0 };
                    } else {
                        // TODO. Handle this condition?
                    }
                }
            }
        }
        return returnValue;
    }

    RangeInfrastructureResult tagTwrLocalize(uint16_t finalMessageDelay) {
        RangeRequestResult request_result = DW1000NgRTLS::tagRangeRequest();

        if(request_result.success) {
            
            RangeInfrastructureResult result = DW1000NgRTLS::tagRangeInfrastructure(request_result.target_anchor, finalMessageDelay);

            if(result.success)
                return result;
        }
        return {false, 0};
    }

    RangeAcceptResult anchorRangeAccept(NextActivity next, uint16_t value) {
        RangeAcceptResult returnValue;

        double range;
        if(!DW1000NgRTLS::receiveFrame()) {
            returnValue = {false, 0};
        } else {

            size_t poll_len = DW1000Ng::getReceivedDataLength();
            byte poll_data[poll_len];
            DW1000Ng::getReceivedData(poll_data, poll_len);

            if(poll_len > 9 && poll_data[9] == RANGING_TAG_POLL) {
                uint64_t timePollReceived = DW1000Ng::getReceiveTimestamp();
                DW1000NgRTLS::transmitResponseToPoll(&poll_data[7]);
                DW1000NgRTLS::waitForTransmission();
                uint64_t timeResponseToPoll = DW1000Ng::getTransmitTimestamp();
                delayMicroseconds(1500);

                if(!DW1000NgRTLS::receiveFrame()) {
                    returnValue = {false, 0};
                } else {

                    size_t rfinal_len = DW1000Ng::getReceivedDataLength();
                    byte rfinal_data[rfinal_len];
                    DW1000Ng::getReceivedData(rfinal_data, rfinal_len);
                    if(rfinal_len > 18 && rfinal_data[9] == RANGING_TAG_FINAL_RESPONSE_EMBEDDED) {
                        uint64_t timeFinalMessageReceive = DW1000Ng::getReceiveTimestamp();

                        byte finishValue[2];
                        DW1000NgUtils::writeValueToBytes(finishValue, value, 2);

                        if(next == NextActivity::RANGING_CONFIRM) {
                            DW1000NgRTLS::transmitRangingConfirm(&rfinal_data[7], finishValue);
                        } else {
                            DW1000NgRTLS::transmitActivityFinished(&rfinal_data[7], finishValue);
                        }
                        
                        DW1000NgRTLS::waitForTransmission();

                        range = DW1000NgRanging::computeRangeAsymmetric(
                            DW1000NgUtils::bytesAsValue(rfinal_data + 10, LENGTH_TIMESTAMP), // Poll send time
                            timePollReceived, 
                            timeResponseToPoll, // Response to poll sent time
                            DW1000NgUtils::bytesAsValue(rfinal_data + 14, LENGTH_TIMESTAMP), // Response to Poll Received
                            DW1000NgUtils::bytesAsValue(rfinal_data + 18, LENGTH_TIMESTAMP), // Final Message send time
                            timeFinalMessageReceive // Final message receive time
                        );

                        range = DW1000NgRanging::correctRange(range);

                        /* In case of wrong read due to bad device calibration */
                        if(range <= 0) 
                            range = 0.000001;

                        returnValue = {true, range};
                    }
                }
            }
        }

        return returnValue;
    }

}