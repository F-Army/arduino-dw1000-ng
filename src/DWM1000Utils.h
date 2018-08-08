/*
 * Copyright (c) 2018 by Michele Biondi <michelebiondi01@gmail.com>, Andrea Salvatori <andrea.salvatori92@gmail.com>
 * Copyright (c) 2015 by Thomas Trojer <thomas@trojer.net>; Arduino-DW1000
 * Decawave DWM1000 library for arduino.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file DWM1000Utils.h
 * Helper functions.
 */

#pragma once

#include "Arduino.h"

namespace DWM1000Utils {
    boolean getBit(byte data[], uint16_t n, uint16_t bit);
    void setBit(byte data[], uint16_t n, uint16_t bit, boolean val);
    void writeValueToBytes(byte data[], int32_t val, uint16_t n);
}

