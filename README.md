# Firmware for 3 Phase Motor Driver

3 Phase motor controller driver firmware.

## Current Implementation

This is very much a work in progress. The following list is of currently implemented and planned features.

- [ ] Sensors
  - [x] Magnetometer
  - [ ] Back EMF Sensing
  - [ ] External Sensor
- [x] Motor movement
  - [ ] Full Torque
- [x] Commands
  - [x] Stepping function
  - [x] Amplitude
  - [x] Velocity
  - [x] Position
- [ ] I2C Communication
- [x] USB Communication
- [x] Serial Communication

## Getting Started

### Board

This code is meant to run on the [ATmega32U4](http://www.atmel.com/Images/Atmel-7766-8-bit-AVR-ATmega16U4-32U4_Datasheet.pdf) on [3 Phase Driver Board](https://github.com/cinderblock/3-Phase-Driver).

## Initial Programming and Calibration Procedure

1. Program the Bootloader
   1. Open a terminal in this project
   1. cd into `bootloader` directory
   1. Connect programmer to motor
   1. Power the device (either option)
      - Set switch on programmer to "5v"
      - Provide 12V power to the board
   1. Run `make`
   1. Verify that the red LED flashes at 1 Hz
1. Program with the main program
   1. Plug in a USB (internal or external plug)
   1. `cd ..` up to main project folder
   1. Plug in DC power (generally 12v)
   1. Verify that the red LED flashes at 1 Hz
      1. If not flashing or if it is not visible, do a reset
      1. If still does not work, reload the bootloader
         - requires the motor to be opened to use the programing board
   1. Run `make`
   1. Verify that the red LED on solid
1. Run Calibration procedure
   1. `cd Calibration-Tool`
   1. ensure yarn dependencies are up to date: `yarn`
   1. `yarn mlx` to set the gain to a constant. (26 seems appropriate. Max raw is about 4000.)
      1. reset device
      1. cycle USB connection
      1. return
      1. set LowGain and HighGain to 26
      1. enter to accept hex value
   1. Run Calibration Procedure: `yarn start`
      1. Power cycle the motor
      ```txt
      Data file? [data.csv]: <enter>
      Capture fresh? [No]: y<enter>
      Fresh: true
      Cycles per Rev: 15
      Revolutions: 3
      Amplitude: 65
      Smooth Control - info: Started watching for USB devices
      Device attached: 4:38:25 PM None
      Serial Number [None]: <enter>
      New serial number [1c28c9e0-b57f-11e9-b138-9de272fac789]: (Must be clamped into test fixture at this point) <enter>
      ```
   1. Look at data.html to verify that the curves are right. Max X and Y should be under 3800.
   1. Run `yarn mlx` again to reset the gain to a better value iff there is a problem.
1. Load calibration data onto device
   1. Reset device
   1. `dfu-programmer.exe ATmega32u4 flash --force <SerialNumber>.hex`
1. Test the motor
   1. power cycle the supply or USB
   1. `yarn test`
      1. Check for devices present and present the serial number
      1. `<enter>` starts a sine test
      1. `cX <enter>` to to constant X force.

## Motor

Currently Testing with [Quanum MT5206](https://hobbyking.com/en_us/quanum-mt-series-5206-320kv-brushless-multirotor-motor-built-by-dys.html).
Also playing with Hoverboard motors.

## Git Submodules

Don't forget about the submodules we're using.
Modern git checks out submodules correctly but older git requires some extra commands:

```bash
git submodule init
git submodule update
```

## Build Requirements

Only real requirement is running the main `Makefile`.
All build steps are configurable on a per machine hostname basis.
Create a file named `local.<hostname>.mk` in the project root, next to the main `Makefile`.
This file will be automatically loaded and can be used to override any make variables.
The primary use for this is to enable the ability to not have all the needed binaries in the PATH.

#### Compiler

While we endeavour to be standards compliant, the reality is that it is difficult to ensure all possible compilers work.

Known working compilers:

- [AVR 8-bit Toolchain](https://www.microchip.com/mplab/avr-support/avr-and-arm-toolchains-c-compilers) v3.6.2

#### Common `local.<hostname>.mk` options

```make
GCC_RootDir = C:/Program Files (x86)/avr8-gnu-toolchain
mkdirCommand = mkdir
```

#### Windows

All paths in uMaker use `/` as a directory separator.

Windows's `mkdir`/`md` does not support `/` directory separator.

Installing Cygwin is the easiest way to get building on Windows working.
Make sure `make` is "selected for installation".

#### Troubleshooting

Try `make clean` first.

### Bootloader

The current Makefile is setup to program an AVR via USB DFU. It is possible to switch to using an ISP or other bootloader/programmer easily as well.

### VSCode

To get VSCode IntelliSense to work, `local.avr-gcc.compilerDir` must be set in your User Settings.
Likely something like `"/path/to/avr8-gnu-toolchain"` or `"C:/Program Files/avr8-win64"` is needed.
This directory should have a `bin` and `include` directory inside of it, among others.

## Associated documentation

- [Technical Docs](docs/README.md)
- [3 Phase sine generation](docs/3%20Phase%20Sine%20Wave%20Generation.md)
- [Triple Buffer](libCameron/Triple%20Buffer.png)

### Communication Diagram

[![Communication Diagram](docs/Control%20Sequence.svg)](docs/Control%20Sequence.svg)
