#pragma once

#include "DWM1000Constants.hpp"
#include "DWM1000.hpp"
#include "DWM1000Test.hpp"

namespace DWM1000 {

    namespace {
        void disableSequencing() {
            enableClock(XTI_CLOCK);
            byte zero[2];
            writeValueToBytes(zero, 0x00, 2);
            writeBytes(PMSC, PMSC_CTRL1_SUB, zero, 2); // To re-enable write 0xE7
        }

        void enableRfPllTx() {
            byte enable_mask[4] = { 0x00, 0xFF, 0x5F, 0x00 }; // TXFEN, PLLFEN, LDOFEN
            writeBytes(RF_CONF, RF_CONF_SUB, enable_mask, LEN_RX_CONF_SUB);
        }
    }
    /* Used for regulatory tx power testing */
    void setTransmitPowerSpectrumTestMode(uint32_t repeat_interval) {
        disableSequencing();
        enableRfPllTx();

        00000000
        11111100 |
        00000010

        00000000
        11001111 |
        00100000

        00100010

        //TODO Enableclocks

        if(repeat_interval < 4) 
            repeat_interval = 4;

        byte values[4];
        writeValueToBytes(values, (int32_t) repeat_interval, 4);
        writeBytes(DX_TIME, NO_SUB, values, DX_TIME_LEN);
        //TODO Check TXDLYS

        byte transmitTestBytes[2];
        writeValueToBytes(transmitTestBytes, 16, LEN_DIAG_TMC);
        writeBytes(DIG_DIAG, DIAG_TMC_SUB)

    }
}