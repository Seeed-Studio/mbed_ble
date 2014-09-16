mbed BLE
========

This repo contains mbed SDK, mbed BLE API and demos for nRF51822. The [gcc-arm-embedded toolchain][] is used.

This branch sticks to Nordic SoftDevice V6. It is for [BLE Micro][] to create BLE applications.
SoftDevice V6 and OTA (Over-The-Air) bootloader are pre-flashed on BLE Micro. So we can upgrade BLE Micro through OTA.


### How To
```
git clone https://github.com/Seeed-Studio/mbed_ble.git
cd mbed_ble
git checkout -b softdevice_v6 origin/softdevice_v6
cd demos/color_pixels
make
```

To merge softdevice hex file and application hex file, please download srecord from http://srecord.sourceforge.net/.
or if Ubuntu is used, run `sudo apt-get install srecord`. Then run:

```
make merge
```

The repo is licensed under [The Apache License](http://www.apache.org/licenses/LICENSE-2.0).

Contributing to this software is warmly welcomed. You can do this basically by<br>
[forking](https://help.github.com/articles/fork-a-repo), committing modifications and then [pulling requests](https://help.github.com/articles/using-pull-requests) (follow the links above<br>
for operating guide). Adding change log and your contact into file header is encouraged.<br>
Thanks for your contribution.

Seeedstudio is an open hardware facilitation company based in Shenzhen, China. <br>
Benefiting from local manufacture power and convenient global logistic system, <br>
we integrate resources to serve new era of innovation. Seeedstudio also works with <br>
global distributors and partners to push open hardware movement.<br>


[gcc-arm-embedded toolchain]: https://launchpad.net/gcc-arm-embedded
[BLE Micro]: http://www.seeedstudio.com/depot/Seeed-Micro-BLE-Module-w-CortexM0-Based-nRF51822-SoC-p-1975.html?cPath=19_21

[![Analytics](https://ga-beacon.appspot.com/UA-46589105-3/mbed_ble)](https://github.com/igrigorik/ga-beacon)
