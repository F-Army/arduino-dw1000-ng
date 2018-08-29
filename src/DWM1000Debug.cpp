#include <Arduino.h>
#include "DWM1000.h"

namespace DWM1000Debug {
    void getPrettyBytes(byte data[], char msgBuffer[], uint16_t n) {
        uint16_t i, j, b;
        b = sprintf(msgBuffer, "Data, bytes: %d\nB: 7 6 5 4 3 2 1 0\n", n); // TODO - type
        for(i = 0; i < n; i++) {
            byte curByte = data[i];
            snprintf(&msgBuffer[b++], 2, "%d", (i+1));
            msgBuffer[b++] = (char)((i+1) & 0xFF);
            msgBuffer[b++] = ':';
            msgBuffer[b++] = ' ';
            for(j = 0; j < 8; j++) {
                msgBuffer[b++] = ((curByte >> (7-j)) & 0x01) ? '1' : '0';
                if(j < 7) {
                    msgBuffer[b++] = ' ';
                } else if(i < n-1) {
                    msgBuffer[b++] = '\n';
                } else {
                    msgBuffer[b++] = '\0';
                }
            }
        }
        msgBuffer[b++] = '\0';
    }

    void getPrettyBytes(byte cmd, uint16_t offset, char msgBuffer[], uint16_t n) {
        uint16_t i, j, b;
        byte* readBuf = (byte*)malloc(n);
        DWM1000Class::readBytes(cmd, offset, readBuf, n);
        b     = sprintf(msgBuffer, "Reg: 0x%02x, bytes: %d\nB: 7 6 5 4 3 2 1 0\n", cmd, n);  // TODO - tpye
        for(i = 0; i < n; i++) {
            byte curByte = readBuf[i];
            snprintf(&msgBuffer[b++], 2, "%d", (i+1));
            msgBuffer[b++] = (char)((i+1) & 0xFF);
            msgBuffer[b++] = ':';
            msgBuffer[b++] = ' ';
            for(j = 0; j < 8; j++) {
                msgBuffer[b++] = ((curByte >> (7-j)) & 0x01) ? '1' : '0';
                if(j < 7) {
                    msgBuffer[b++] = ' ';
                } else if(i < n-1) {
                    msgBuffer[b++] = '\n';
                } else {
                    msgBuffer[b++] = '\0';
                }
            }
        }
        msgBuffer[b++] = '\0';
        free(readBuf);
    }
}