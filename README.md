# ArduinoMessengerServer

Sketch prepared for **Arduino Leonardo** with **ENC28J60** as a TCP/IP server with IR receiver and transmitter.

##Prerequisites

Download and extract following libraries in yours IDE/libraries directory:
- [IRremote]  for Infrared communication,
- [UIPEthernet] for handling TCP/IP connection by ENC28J60,
- [ArduinoMessenger] to parse incoming messages.

**Important**:
To fit this skecht in Arduino Leonardo flash memory edit `IRremote.h` from IRremote library.
In `Supported IR protocols`section disable `DECODE_` and `SEND_` includes for all protocols.


### Wiring diagram
![Sketch wiring diagram](/wiring/remote.png)

This skecht uses Tsop2236 as IR receiver.

![Tsop2236 pinout](/wiring/tsop2236.png)


### Issues management
Issues are managed at the GitHub [project issues page][issues].

Everybody is allowed to report bugs or ask for features.

### Getting the code
The latest version of the code can be found at the [GitHub project page][scm].

Feel free to fork it, and share the changes.
## Collaborate

The project is still under ongoing development, and any help will be well received.

There are two ways to help: reporting errors and asking for extensions through the issues management, or forking the repository and extending the project.

## Authors

* **Kamil Tokarzewski** - *Initial work* - [ktokarzewski](https://github.com/ktokarzewski)

See also the list of [contributors](https://github.com/ktokarzewski/ArduinoMessengerServer/contributors) who participated in this project.
## License
The project is released under the [MIT License][license].

[issues]:https://github.com/ktokarzewski/ArduinoMessengerServer/issues
[scm]:https://github.com/ktokarzewski/ArduinoMessengerServer
[license]: http://www.opensource.org/licenses/mit-license.php

[IRremote]:https://github.com/z3t0/Arduino-IRremote
[UIPEthernet]:https://github.com/ntruchsess/arduino_uip
[ArduinoMessenger]:https://github.com/ktokarzewski/ArduinoMessenger
