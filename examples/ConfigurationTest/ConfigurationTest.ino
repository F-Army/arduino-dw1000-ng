/*  
 *	Arduino-DWM1000 - Arduino library to use Decawave's DWM1000 module.
 *	Copyright (C) 2018  Michele Biondi <michelebiondi01@gmail.com>, Andrea Salvatori <andrea.salvatori92@gmail.com>
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DWM1000Configuration.hpp"
#include "DWM1000Constants.hpp"

DWM1000Configuration defaultConfig;
DWM1000Configuration smartPowerConfig(
    TRX_RATE_110KBPS,
    TX_PULSE_FREQ_16MHZ,
    TX_PREAMBLE_LEN_128,
    CHANNEL_5,
    PREAMBLE_CODE_16MHZ_7,
    true,
	true,
	true,
    true,
	true,
	true,
    true,
    true,
    true,
    true,
    true,
    16384
);
DWM1000Configuration manualPowerConfig(
    TRX_RATE_850KBPS,
    TX_PULSE_FREQ_64MHZ,
    TX_PREAMBLE_LEN_1024,
    CHANNEL_2,
    PREAMBLE_CODE_16MHZ_3,
    false,
	false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    false,
    0x25456585,
    0xC0,
    0
);

void setup() {
    Serial.begin(9600);
    Serial.println(F("### DWM1000Configuration-arduino-test ###"));

    Serial.println(F("\nTEST DEFAULT\n"));
    /* test default */
    Serial.print("DATARATE:");
    defaultConfig.getDatarate() == TRX_RATE_6800KBPS ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("PULSE FREQUENCY:");
    defaultConfig.getPulseFrequency() ==  TX_PULSE_FREQ_16MHZ ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("PREAMBLE LENGTH:");
    defaultConfig.getPreambleLength() == TX_PREAMBLE_LEN_128 ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("CHANNEL:");
    defaultConfig.getChannel() == CHANNEL_5 ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("PREAMBLE CODE:");
    defaultConfig.getPreambleCode() == PREAMBLE_CODE_16MHZ_4 ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON SENT:");
    defaultConfig.getInterruptOnSent() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON RECEIVED:");
    defaultConfig.getInterruptOnReceived() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON RECEIVED FAILED:");
    defaultConfig.getInterruptOnReceiveFailed() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON RECEIVE TIMEOUT:");
    defaultConfig.getInterruptOnReceiveTimeout() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON RECEIVE TIMESTAMP AVAILABLE:");
    !defaultConfig.getInterruptOnReceiveTimestampAvailable() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON AAT:");
    defaultConfig.getInterruptOnAutomaticAcknowledgeTrigger() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("FRAME CHECK:");
    defaultConfig.getFrameCheck() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("EXTENDED FRAME LENGTH:");
    !defaultConfig.getExtendedFrameLength() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("AUTO REENABLE:");
    defaultConfig.getReceiverAutoReenable() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("NLOS:");
    !defaultConfig.getNlos() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("SMART POWER:");
    defaultConfig.getSmartPower() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("TX POWER:");
    defaultConfig.getTxPower() == NULL ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("TCPGDELAY:");
    defaultConfig.getTcpgDelay() == NULL ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("ANTENNA DELAY:");    
    defaultConfig.getAntennaDelay() == 16384 ? Serial.println(" OK") : Serial.println(" FAILED");

    /* Test smartpowerconfig */
    Serial.println(F("\nTEST SMARTPOWERCONFING\n"));
    Serial.print("DATARATE:")
    smartPowerConfig.getDatarate() == TRX_RATE_110KBPS ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("PULSE FREQUENCY:");
    smartPowerConfig.getPulseFrequency() == TX_PULSE_FREQ_16MHZ ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("PREAMBLE LENGTH:");
    smartPowerConfig.getPreambleLength() == TX_PREAMBLE_LEN_128 ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("CHANNEL:");
    smartPowerConfig.getChannel() == CHANNEL_5 ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("PREAMBLE CODE:");
    smartPowerConfig.getPreambleCode() == PREAMBLE_CODE_16MHZ_7 ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON SENT:");
    smartPowerConfig.getInterruptOnSent() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON RECEIVED:");
    smartPowerConfig.getInterruptOnReceived() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON RECEIVED FAILED:");
    smartPowerConfig.getInterruptOnReceiveFailed() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON RECEIVE TIMEOUT:");
    smartPowerConfig.getInterruptOnReceiveTimeout() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON RECEIVE TIMESTAMP AVAILABLE:");
    smartPowerConfig.getInterruptOnReceiveTimestampAvailable() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON AAT:");
    smartPowerConfig.getInterruptOnAutomaticAcknowledgeTrigger() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("FRAME CHECK:");
    smartPowerConfig.getFrameCheck() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("EXTENDED FRAME LENGTH");
    smartPowerConfig.getExtendedFrameLength() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("AUTO REENABLE:");
    smartPowerConfig.getReceiverAutoReenable() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("NLOS:");
    smartPowerConfig.getNlos() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("SMART POWER:");
    smartPowerConfig.getSmartPower() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("TX POWER:");
    smartPowerConfig.getTxPower() == NULL ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("TCPGDELAY:");
    smartPowerConfig.getTcpgDelay() == NULL ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("ANTENNA DELAY:");    
    smartPowerConfig.getAntennaDelay() == 16384 ? Serial.println(" OK") : Serial.println(" FAILED");

    /* Test manual power config */
    Serial.println(F("\nTEST MANUALPOWERCONFIG\n"));

    Serial.print("DATARATE:");
    manualPowerConfig.getDatarate() == TRX_RATE_850KBPS ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("PULSE FREQUENCY:");
    manualPowerConfig.getPulseFrequency() == TX_PULSE_FREQ_64MHZ ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("PREAMBLE LENGTH:");
    manualPowerConfig.getPreambleLength() == TX_PREAMBLE_LEN_1024 ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("CHANNEL:");
    manualPowerConfig.getChannel() == CHANNEL_2 ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("PREAMBLE CODE:");
    manualPowerConfig.getPreambleCode() == PREAMBLE_CODE_16MHZ_3  ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON SENT:");
    !manualPowerConfig.getInterruptOnSent() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON RECEIVED:");
    !manualPowerConfig.getInterruptOnReceived() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON RECEIVED FAILED:");
    !manualPowerConfig.getInterruptOnReceiveFailed() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON RECEIVE TIMEOUT:");
    !manualPowerConfig.getInterruptOnReceiveTimeout() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON RECEIVE TIMESTAMP AVAILABLE:");
    !manualPowerConfig.getInterruptOnReceiveTimestampAvailable() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("INTERRUPT ON AAT:");
    !manualPowerConfig.getInterruptOnAutomaticAcknowledgeTrigger() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("FRAME CHECK:");
    !manualPowerConfig.getFrameCheck() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("EXTENDED FRAME LENGTH");
    !manualPowerConfig.getExtendedFrameLength() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("AUTO REENABLE:");
    !manualPowerConfig.getReceiverAutoReenable() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("NLOS:");
    !manualPowerConfig.getNlos() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("SMART POWER:");
    !manualPowerConfig.getSmartPower() ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("TX POWER:");
    manualPowerConfig.getTxPower() == 0x25456585 ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("TCPGDELAY:");
    manualPowerConfig.getTcpgDelay() == 0xC0 ? Serial.println(" OK") : Serial.println(" FAILED");
    Serial.print("ANTENNA DELAY:");    
    manualPowerConfig.getAntennaDelay() == 0 ? Serial.println(" OK") : Serial.println(" FAILED"); 
}   

void loop() { }