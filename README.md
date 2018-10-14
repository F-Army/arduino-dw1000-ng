<br/>
<div align="center">
  <img src="img/logo.png">
</div>
<br/>
<div align="center">

### Arduino library to use Decawave's [DW1000](https://www.decawave.com/product/dw1000-radio-ic/) IC and relative modules.

![c++11](https://img.shields.io/badge/C%2B%2B-11-brightgreen.svg?&style=for-the-badge)
![Arduino](https://img.shields.io/badge/Arduino-%3E%3D1.6.6-blue.svg?&style=for-the-badge)
[![Discord](https://img.shields.io/badge/Discord-Arduino%20DW1000Ng-7289da.svg?&style=for-the-badge&logo=discord)](https://discord.gg/VJWYhJq)
</div>
<br/>

Status
------------
This is still work in progress, expect breaking changes until the first release.

Installation
------------
**Requires c++11 support**, Arduino IDE >= 1.6.6 support c++11.

 1. Get a ZIP file of the master branch or the latest release and save somewhere on your machine.
 2. Open your Arduino IDE and goto _Sketch_ / _Include Library_ / _Add .ZIP Library..._
 3. Select the downloaded ZIP file of the DW1000 library
 4. You should now see the library in the list and have access to the examples in the dedicated section of the IDE

Usage
-----
Check examples for basic usage.

Tests
-----
To run tests open them with Arduino and upload them to a device. <br />
Be careful to activate the necessary compile options inside [DW1000NgCompileOptions.hpp](https://github.com/F-Army/arduino-dw1000Ng/blob/master/src/DW1000NgCompileOptions.hpp)

License
-------
The project is under MIT (see [LICENSE.md](https://github.com/F-Army/arduino-dw1000Ng/blob/master/LICENSE.md)) <br />
Some files are under a Dual Apache 2.0/MIT because this is a fork of [thotro/arduino-dw1000](https://github.com/thotro/arduino-dw1000)
