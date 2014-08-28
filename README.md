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
make
```

To merge softdevice hex file and application hex file, please download srecord from http://srecord.sourceforge.net/.
or if Ubuntu is used, run `sudo apt-get install srecord`. Then run:

```
make merge
```



[gcc-arm-embedded toolchain]: https://launchpad.net/gcc-arm-embedded
[BLE Micro]: http://www.seeedstudio.com/depot/Seeed-Micro-BLE-Module-w-CortexM0-Based-nRF51822-SoC-p-1975.html?cPath=19_21
