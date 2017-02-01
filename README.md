# Bluetooth firmware for the SensorWeb board

This repository contains the firmware which runs on the nRF51822 (bluetooth)
processor on the SensorWeb board.

# Prerequisites
- Install a [gcc toolchain](https://launchpad.net/gcc-arm-embedded)

## Install nRF5x-Command-Line-Tools-Linux64 (needed if flashing via nRF51-DK)
- Download (nRF5x-Command-Line-Tools-Linux64)[https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Linux64/51386].
- untar the downloaded tarball
- The tarball expands to 2 directories, nrfjproj and mergehex. Add the nrfjprog directory to your PATH.
- Install the JLink DLL
  - Visit the (SEGGER Download page)[https://www.segger.com/downloads/jlink]
  - In the second section titled "J-Link Software and Documentation Pack" click on the "Click for downloads" and download and install the appropriate package.

# Building the firmware
Nordic requires that the SDK be downloaded from their site, so the build script
will do that automatically.

To build the firmware, type `make`.

# Flashing the firmware
Connect the nRF51-DK to the SensorWeb board by following (these instructions)[https://github.com/mozilla-sensorweb/sensorweb-wiki/wiki/Programming-the-SensorWeb-nRF51822-using-an-nRF51DK]

## Flash the softdevice
This should only have to be done once.
```
make flash_softdevice
```

## Flash the sensorweb firmware
```
make flash
```

# Operation
The blue LED should flash about once per second when the firmware is discoverable.
When connected, the blue LED will be on solid.
After being discoverable (referred to as advertising mode) for 3 minutes, it
goes into a sleep mode and needs to be reset to go back into advertising mode.
This behaviour will change shortly.





