mbed BLE
========

This repo contains mbed SDK, mbed BLE API and demos for nRF51822. The [gcc-arm-embedded][] toolchain is used.

This branch uses Nordic SoftDevice V7. If you prefer to use SoftDevice V6, just use [softdevice_v6][] branch

### How To
```
git clone https://github.com/Seeed-Studio/mbed_ble.git
cd mbed_ble
make
```

To merge softdevice hex file and application hex file, please download srecord from http://srecord.sourceforge.net/.
or if Ubuntu is used, run `sudo apt-get install srecord`. Then run:

```
make merge
```



[gcc-arm-embedded]: https://launchpad.net/gcc-arm-embedded
[softdevice_v6]: https://github.com/Seeed-Studio/mbed_ble/tree/softdevice_v6
