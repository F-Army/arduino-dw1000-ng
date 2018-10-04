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
#include "DW1000NgRTLSStandardMessages.hpp"
#include "DW1000NgUtils.hpp"

namespace DW1000NgRTLSStandardMessages {

    static byte seq_number = 0;

    void transmitBlink() {
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
        DW1000Ng::startTransmit(TransmitMode::IMMEDIATE);
    }
    
    void transmitData(message_addressing_settings_t &addressing_settings, message_data_settings_t &data_settings) {
        size_t data_length = 5;
        size_t source_address_offset = 0;
        size_t application_data_offset = 0;
        data_length += addressing_settings.useShortDestinationAddress ? 2 : 8;
        data_length += addressing_settings.useShortSourceAddress ? 2 : 8;
        data_length += (data_settings.dataLength) + 1;
        byte dataFrame[data_length];
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
            source_address_offset += 6;
        }

        if(addressing_settings.useShortSourceAddress) {
            //Carico il mio short in dataFrame[7+source_address_offset] application_offset = source_address_offset 
        } else {
            //Carico il mio eui in dataFrame[7+source_address_offset] application_offset = source_address_offset + 6
        }

        //Carico la application data a partire da dataFrame[9+application_offset]

        //Carico i dati e trasmetto
        

        

        

    }
}