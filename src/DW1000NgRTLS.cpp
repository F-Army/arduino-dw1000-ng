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

namespace DW1000Ng {

    static byte seq_number = 0;

    void encodeShortBlink() {
        byte blink[] = {
                        0xC5, 
                        seq_number++, 
                        0,0,0,0,0,0,0,0, //EUI will be inserted here
                        };
                        
        /* inserts eui into blink */
        byte device_eui[LEN_EUI];
        DW1000Ng::getEUI(device_eui);
        memcpy(&blink[2], device_eui, LEN_EUI);

        DW1000Ng::setTransmitData(blink, sizeof(blink));
    }

    void encodeLongBlink(blink_settings_t &settings) {
        size_t BLINK_SIZE = 11;
        if(!settings.sensor_settings->temperatureUnknown)
            BLINK_SIZE++;
        
        if(settings.exId_settings != nullptr)
            BLINK_SIZE += settings.exId_settings->exIdLength + 2;
    }
    
    void encodeData(message_addressing_settings_t &addressing_settings, message_data_settings_t &data_settings) {
        size_t DATA_LEN = 5;
        size_t SRC_OFFSET = 0;
        size_t APP_OFFSET = 0;
        DATA_LEN += addressing_settings.useShortDestinationAddress ? 2 : 8;
        DATA_LEN += addressing_settings.useShortSourceAddress ? 2 : 8;
        /* Application data length + 1 for the mandatory function code */
        DATA_LEN += (data_settings.dataLength) + 1;

        byte dataFrame[DATA_LEN];

        dataFrame[0] = 0x41;
        dataFrame[1] = 0x00;
        dataFrame[1] |= addressing_settings.useShortDestinationAddress ? 0x08 : 0x0C;
        dataFrame[1] |= addressing_settings.useShortSourceAddress ? 0x80 : 0xC0;

        dataFrame[2] = seq_number++;

        dataFrame[3] = 0x9A;
        dataFrame[4] = 0x60;

        if(addressing_settings.useShortDestinationAddress) {
            dataFrame[5] = static_cast<byte>(addressing_settings.shortDestinationAddress & 0xFF);
            dataFrame[6] = static_cast<byte>((addressing_settings.shortDestinationAddress >> 8) & 0xFF);
        } else {
            byte eui[LEN_EUI];
            DW1000NgUtils::writeValueToBytes(eui, addressing_settings.longDestinationAddress, LEN_EUI);
            memcpy(&dataFrame[5], eui, LEN_EUI);
            SRC_OFFSET += 6;
            APP_OFFSET = SRC_OFFSET;
        }

        if(addressing_settings.useShortSourceAddress) {
            //Carico il mio short in dataFrame[7+SRC_OFFSET] 
            byte shortAddress[LEN_PANADR - 2];
            DW1000Ng::getDeviceAddress(shortAddress);
            dataFrame[7+SRC_OFFSET] = shortAddress[0];
            dataFrame[8+SRC_OFFSET] = shortAddress[1];
        } else {
            byte eui[LEN_EUI];
            DW1000Ng::getEUI(eui);
            memcpy(&dataFrame[7+SRC_OFFSET], eui, LEN_EUI);
            APP_OFFSET += 6;
        }

        dataFrame[9+APP_OFFSET] = data_settings.functionCode;
        if(data_settings.dataLength > 0) {
            memcpy(&dataFrame[10+APP_OFFSET], data_settings.data, data_settings.dataLength);
        }
        
        DW1000Ng::setTransmitData(dataFrame, DATA_LEN);              
    }
}