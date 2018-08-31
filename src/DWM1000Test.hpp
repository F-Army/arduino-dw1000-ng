#pragma once

#include "DWM1000.hpp"

namespace DWM1000 {
    /* Used for regulatory tx power testing */
    void setTransmitPowerSpectrumTestMode(uint32_t repeat_interval);
}