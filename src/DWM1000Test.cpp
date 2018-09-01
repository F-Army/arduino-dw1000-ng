#include "DWM1000Constants.hpp"
#include "DWM1000.hpp"
#include "DWM1000Test.hpp"
#include "DWM1000Utils.hpp"

namespace DWM1000 {

    namespace {        
        void disableSequencing() {
            enableClock(XTI_CLOCK);
            byte zero[2];
            DWM1000Utils::writeValueToBytes(zero, 0x0000, 2);
            writeBytes(PMSC, PMSC_CTRL1_SUB, zero, 2); // To re-enable write 0xE7
        }

        void enableRfPllTx() {
            byte enable_mask[4] = { 0x00, 0xFF, 0x5F, 0x00 }; // TXFEN, PLLFEN, LDOFEN
            writeBytes(RF_CONF, RF_CONF_SUB, enable_mask, LEN_RX_CONF_SUB);
        }
    }
    
    /* Used for regulatory tx power testing */
    void setTransmitPowerSpectrumTestMode(int32_t repeat_interval) {
        disableSequencing();
        enableRfPllTx();
        enableClock(PLL_CLOCK);
        enableClock(PLL_TX_CLOCK);

        if(repeat_interval < 4) 
            repeat_interval = 4;

        byte values[4];
        DWM1000Utils::writeValueToBytes(values, repeat_interval, 4);
        writeBytes(DX_TIME, NO_SUB, values, LEN_DX_TIME);
        //TODO Check if TXDLYS is activated

        byte transmitTestBytes[2];
        DWM1000Utils::writeValueToBytes(transmitTestBytes, 0x10, LEN_DIAG_TMC);
        writeBytes(DIG_DIAG, DIAG_TMC_SUB, transmitTestBytes, LEN_DIAG_TMC);
    }
}