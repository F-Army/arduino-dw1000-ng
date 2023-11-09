<br/>
<div align="center">
  <img src="img/logo.png">
</div>
<br/>
<div align="center">

### Arduino library to use Decawave's [DW1000](https://www.decawave.com/product/dw1000-radio-ic/) IC and relative modules.

![c++11](https://img.shields.io/badge/C%2B%2B-11-brightgreen.svg?&style=for-the-badge)
![Arduino](https://img.shields.io/badge/Arduino-%3E%3D1.6.6-blue.svg?&style=for-the-badge)

</div>

Status
------------
:warning: We are archiving the repo as we can't support it anymore and we are not currently working with dw1000 hardware, feel free to create your own fork if you intend to support the project, we will be happy to add a link on this project. Thank you everyone for the support you gave this project it was truly appreciated.

Differences with thotro/arduino-dw1000
------------
This is a fork of https://github.com/thotro/arduino-dw1000.<br/>
The main reason for this fork is because the development there is really slow (due to the original author not developing it anymore) and some important features were missing (for example antenna delay calibration). <br/>
We also wanted to change how the ranging logic works.<br/>
Rather than providing a ranging loop, we wanted to provide functions to send the different Two way ranging flow frames, giving the user more control over the program. <br />
The base driver was also re-written in a lot of places, to make every API function more independent from each other.
We also plan to make very high-level abstractions to hack projects fast.

Features
------------
![Ranging](https://img.shields.io/badge/Ranging-done-brightgreen.svg?&style=for-the-badge)
![Localization](https://img.shields.io/badge/Localization-done-brightgreen.svg?&style=for-the-badge)
![NLOS Optimizations](https://img.shields.io/badge/NLOS%20Optimizations-done-brightgreen.svg?&style=for-the-badge)
![Frame filtering](https://img.shields.io/badge/Frame%20filtering-done-brightgreen.svg?&style=for-the-badge)
![Antenna Calibration](https://img.shields.io/badge/Antenna%20calibration-api-yellow.svg?&style=for-the-badge)
![Antenna Delay Arduino EEPROM](https://img.shields.io/badge/Antenna%20Delay%20EEPROM%20\(Arduino\)-test-orange.svg?&style=for-the-badge)
![Transmission power setup](https://img.shields.io/badge/Transmission%20power%20setup-done-brightgreen.svg?&style=for-the-badge)
![Deep sleep](https://img.shields.io/badge/Deep%20sleep%20-done-brightgreen.svg?&style=for-the-badge)
![Sleep](https://img.shields.io/badge/Sleep%20-TODO-red.svg?&style=for-the-badge)
![Double buffering](https://img.shields.io/badge/Double%20buffering-TODO-red.svg?&style=for-the-badge)
![Transmit Power Spectrum Test Mode](https://img.shields.io/badge/Transmit%20Power%20Spectrum%20Test%20Mode-DONE-brightgreen.svg?&style=for-the-badge)
![Continuous wave mode](https://img.shields.io/badge/Continuous%20Wave%20Mode-TODO-red.svg?&style=for-the-badge)
<br/>

Devices
------------
![Arduino](https://img.shields.io/badge/Arduino-done-brightgreen.svg?&style=for-the-badge)
![ESP8266](https://img.shields.io/badge/ESP8266-done-brightgreen.svg?&style=for-the-badge)
![ESP32](https://img.shields.io/badge/ESP32-test-yellow.svg?&style=for-the-badge)
![STM32-BluePill](https://img.shields.io/badge/STM32_BluePill-Experimental-yellow.svg?&style=for-the-badge)

Specific tested devices, whit related pinout, are inside the [Wiki](https://github.com/F-Army/arduino-dw1000-ng/wiki/Tested-hardware-boards).

Installation
------------
**Requires c++11 support**, Arduino IDE >= 1.6.6 support c++11.

 1. Get a ZIP file of the master branch or the latest release and save somewhere on your machine.
 2. Open your Arduino IDE and goto _Sketch_ / _Include Library_ / _Add .ZIP Library..._
 3. Select the downloaded ZIP file of the DW1000 library
 4. You should now see the library in the list and have access to the examples in the dedicated section of the IDE

Usage
-----
Check examples folder for basic usage.

The [Decawave documentation](https://github.com/F-Army/arduino-dw1000-ng/wiki/Decawave's-documentation) helps a lot in understanding this project, especially if you want to go deeper.

Authors
------------
<ul>
  <li><b>Michele 'an4cr0n' Biondi </b>: https://github.com/an4cr0n</li>
  <li><b>Andrea 'Sonic0' Salvatori</b>: https://github.com/Sonic0</li>
</ul>

License
-------
The project is under MIT (see [LICENSE.md](https://github.com/F-Army/arduino-dw1000Ng/blob/master/LICENSE.md)) <br />
Some files are under a Dual Apache 2.0/MIT because this is a fork of [thotro/arduino-dw1000](https://github.com/thotro/arduino-dw1000)
