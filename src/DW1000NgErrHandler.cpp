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

#include <Arduino.h>
#include <string.h>
#include "DW1000NgErrHandler.hpp"
#include "ArduinoLog.h"

namespace DW1000NgErrHandler {

    DW1000NgStatus getState() {
        return _errState;
    }

    void setState(DW1000NgStatus status) {
        _errState = status;
    }

    void setOutput(Print* logOutput) {
        _logOutput = logOutput;
    }

    void logErr(DW1000NgStatus status, char msg[], Print* logOutput) {
    
    setOutput(logOutput);
    setState(status);
    
    #if defined(ARDUINO)
        // Initialize with log level and log output.
        Log.begin(LOG_LEVEL_VERBOSE, &logOutput, true);

        if ( status == DW1000NgStatus::CONFIGURATION_ERROR) {
            Log.notice (F(" Log as Notice %d  with hexadecimal values: %s " CR ), getState(), msg );
        }
        else
        {
            Log.fatal (F(" Log as Fatal %d  with string value from Flash: %s " CR ) , getState(), msg );
        }
    #else
        // TO-DO altre classi per 
    #endif

}

};