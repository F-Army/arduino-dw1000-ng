#pragma once

#include <Arduino.h>

namespace DWM1000Debug {
    void getPrettyBytes(byte data[], char msgBuffer[], uint16_t n);
    void getPrettyBytes(byte cmd, uint16_t offset, char msgBuffer[], uint16_t n);
}
