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

#include "DW1000Ng.hpp"
#include "DW1000NgRegisters.hpp"
#include "DW1000NgRTLS.hpp"
#include "DW1000NgUtils.hpp"

constexpr size_t FRAME_CONTROL_LEN = 2;
constexpr size_t SEQUENCE_NUMBER_LEN = 1;
constexpr size_t APPLICATION_ID_LEN = 2;
constexpr size_t FUNCTION_CODE_LEN = 1;

namespace DW1000Ng {
    
    void encodeMinimalBlink() {
        byte blink[] = {
                        0xC5, 
                        DW1000Ng::getTransmitSequenceNumber(),
                        0,0,0,0,0,0,0,0, //EUI will be inserted here
                        };
                        
        /* inserts eui into blink */
        byte device_eui[LEN_EUI];
        DW1000Ng::getEUI(device_eui);
        memcpy(&blink[2], device_eui, LEN_EUI);

        DW1000Ng::setTransmitData(blink, sizeof(blink));
    }
    /* //TODO
    void encodeLongBlink(blink_settings_t &settings) {
        size_t BLINK_SIZE = 11;
        if(!settings.sensor_settings->temperatureUnknown)
            BLINK_SIZE++;
        
        if(settings.exId_settings != nullptr)
            BLINK_SIZE += settings.exId_settings->exIdLength + 2;
    }
    */
    
    void encodeData(byte source[], addressType src_len, byte destination[], addressType dest_len, message_data_settings_t &data_settings) {
        size_t SRC_LEN = static_cast<size_t>(src_len);
        size_t DEST_LEN = static_cast<size_t>(dest_len);
        size_t len = FRAME_CONTROL_LEN + SEQUENCE_NUMBER_LEN + APPLICATION_ID_LEN + DEST_LEN + SRC_LEN + FUNCTION_CODE_LEN + (data_settings.dataLength);
        byte dataFrame[len];
        dataFrame[FRAME_CONTROL_OFFSET] = 0x41;
        dataFrame[FRAME_CONTROL_OFFSET+1] = 0x00 | (dest_len == addressType::SHORT ? 0x08 : 0x0C);
        dataFrame[FRAME_CONTROL_OFFSET+1] |= (src_len == addressType::SHORT ? 0x80 : 0xC0);
        dataFrame[SEQUENCE_NUMBER_OFFSET] = DW1000Ng::getTransmitSequenceNumber();
        dataFrame[APPLICATION_ID_OFFSET] = 0x9A;
        dataFrame[APPLICATION_ID_OFFSET+1] = 0x60;
        memcpy(&dataFrame[DESTINATION_OFFSET], destination, DEST_LEN);
        memcpy(&dataFrame[DESTINATION_OFFSET+DEST_LEN], source, SRC_LEN);


        dataFrame[DESTINATION_OFFSET+DEST_LEN+SRC_LEN] = data_settings.functionCode;
        if(data_settings.dataLength > 0)
            memcpy(&dataFrame[6+DEST_LEN+SRC_LEN], data_settings.data, data_settings.dataLength);
            
        DW1000Ng::setTransmitData(dataFrame, len);
    }

    frameType getFrameType(byte frame[]) {
        if(frame[FRAME_CONTROL_OFFSET] == 0xC5) 
            return frameType::BLINK;
        else if(frame[FRAME_CONTROL_OFFSET] == 0x41) {
            if( frame[FRAME_CONTROL_OFFSET+1] == 0x88 || 
                frame[FRAME_CONTROL_OFFSET+1] == 0x8C || 
                frame[FRAME_CONTROL_OFFSET+1] == 0xC8 || 
                frame[FRAME_CONTROL_OFFSET+1] == 0xCC) {
                
                    return frameType::DATA;
            } else {
                return frameType::OTHER;
            }
        } else {
            return frameType::OTHER;
        }
    }

    data_offset_t getDataOffsets(byte frame[]) {
        data_offset_t offsets;
        size_t DEST_LEN;
        size_t SRC_LEN;

        if((frame[FRAME_CONTROL_OFFSET+1] & 0x0C) == 0x0C) {
            DEST_LEN = 8;
        } else {
            DEST_LEN = 2;
        }
        
        offsets.source_offset = DESTINATION_OFFSET + DEST_LEN;

        if((frame[FRAME_CONTROL_OFFSET+1] & 0xC0) == 0xC0) {
            SRC_LEN = 8;
        } else {
            SRC_LEN = 2;
        }

        offsets.application_offset = DESTINATION_OFFSET + DEST_LEN + SRC_LEN;

        return offsets;
    }
}