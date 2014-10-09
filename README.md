mbed BLE
========

This repo contains mbed SDK, mbed BLE API and demos for nRF51822. The [gcc-arm-embedded][] toolchain is used.

This branch uses Nordic SoftDevice V7. If you prefer to use SoftDevice V6, just use [softdevice_v6][] branch

### How To
```
git clone https://github.com/Seeed-Studio/mbed_ble.git
cd mbed_ble/demos/ble_uart_loopback   # or mbed_ble/demos/beacon
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


[gcc-arm-embedded]: https://launchpad.net/gcc-arm-embedded
[softdevice_v6]: https://github.com/Seeed-Studio/mbed_ble/tree/softdevice_v6

[![Analytics](https://ga-beacon.appspot.com/UA-46589105-3/mbed_ble)](https://github.com/igrigorik/ga-beacon)
