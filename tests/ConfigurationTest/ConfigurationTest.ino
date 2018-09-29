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
#include <SPI.h>
#include "DW1000NgConfiguration.hpp"
void setup() {
    Serial.begin(115200);
    Serial.println("Default Configuration:");
    DW1000NgConfiguration conf;
    #if DW1000NGCONFIGURATION_H_PRINTABLE
    Serial.print(conf);
    #endif
    Serial.println("\n");

    Serial.println("NLOS Configuration:");
    DW1000NgConfiguration nlos_conf(ConfigurationProfile::DEFAULT_NLOS_PROFILE);
    #if DW1000NGCONFIGURATION_H_PRINTABLE
    Serial.print(nlos_conf);
    #endif
    Serial.println("\n");
}
void loop() { } 