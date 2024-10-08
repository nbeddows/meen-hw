### Introduction

Machine Emulator ENgine Hardware is a collection of 8bit emulated hardwares designed to be used as helper modules when building controllers that fit into the [Machine Emulator ENgine framework](http://github.com/nbeddows/mach-emu/).

Supported hardwares:

- i8080 arcade - hardware emulation based on the 1978 Midway/Taito Space Invaders arcade machine. Along with the original Space Invaders title, this emulated hardware is also compatible with Lunar Rescue (1979), Balloon Bomber (1980) and Space Invaders Part II/Deluxe (1980). **NOTE**: currently does not support emulated audio, the consuming application needs to provide audio samples.

### Compilation

This project uses [CMake (minimum version 3.23)](https://cmake.org/) for its build system and [Conan (minimum version 2.0)](https://conan.io/) for it's dependency package management. Supported compilers are GCC (minimum version 12), MSVC(minimum version 16). The platform used for compilation is assumed to be Windows/Linux x86_64.

#### Pre-requisites

##### Linux

- [Install Conan](https://conan.io/downloads/).
- `sudo apt install cmake`.
- cross compilation:
  - armv7hf:
    - `sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf`.
  - aarch64:
    - `sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu`.
  - rp2040:
    - `sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential libstdc++-arm-none-eabi-newlib`. # Needs to be installed for the 2.x SDK (for picotool) - libusb-1.0-0-dev`.
    - `git clone https://github.com/raspberrypi/pico-sdk.git --branch 1.5.1`
    - `cd pico-sdk`
    - `git submodule update --init`
    - build the Raspberry Pi Pico SDK:
      - Conan and the Raspberry Pi Pico Sdk seem to have an issue with conflicting use of the cmake toolchain file
        which results in test programs not being able to be compiled during the conan build process as outlined [here](https://github.com/raspberrypi/pico-sdk/issues/1693).
        At this point we need to build the sdk so that we have the required tools pre-built so the Conan build process will succeed:
        - `mkdir build`<br>
           **NOTE**: Conan will assume that the build tools are located in the `build` directory, **do not** use a different directory name.
        - `cd build`
        - `cmake ..`
        - `make`
    - Set the Raspberry Pi Pico SDK Path:
        -`export PICO_SDK_PATH=${PATH_TO_PICO_SDK}`
        To avoid having to export it on every session, add it to the end of your .bashrc file instead:
        - `nano ~/.bashrc`
        - `export PICO_SDK_PATH=${PATH_TO_PICO_SDK}`
	- save, close and re-open shell.

##### Windows

- [Install Conan](https://conan.io/downloads).
- [Install CMake](https://cmake.org/download/).

**1.** Install the supported meen conan configurations (v0.1.0) (if not done so already):
- `conan config install -sf profiles -tf profiles https://${token}@github.com/nbeddows/meen-conan-config.git --args "--branch v0.1.0"`

**2.** Install dependencies:
- Windows x86_64 build and host: `conan install . --build=missing --profile:all=Windows-x86_64-msvc-193`.
- Windows x86_64 build and host with unit tests: `conan install . --build=missing --profile:all=Windows-x86_64-msvc-193-gtest`.
- Linux x86_64 build and host: `conan install . --build=missing --profile:all=Linux-x86_64-gcc-13`.
- Linux x86_64 build and host with unit tests: `conan install . --build=missing --profile:all=Linux-x86_64-gcc-13-gtest`.
- Linux x86_64 build, Linux armv7hf host: `conan install . --build=missing -profile:build=Linux-x86_64-gcc-13 -profile:host=Linux-armv7hf-gcc-13`.
- Linux x86_64 build, Linux armv7hf host with unit tests: `conan install . --build=missing -profile:build=Linux-x86_64-gcc-13 -profile:host=Linux-armv7hf-gcc-13-gtest`.
- Linux x86_64 build, Linux aarch64 host: `conan install . --build=missing -profile:build=Linux-x86_64-gcc-13 -profile:host=Linux-armv8-gcc-13`.
- Linux x86_64 build, Linux aarch64 host with unit tests: `conan install . --build=missing -profile:build=Linux-x86_64-gcc-13 -profile:host=Linux-armv8-gcc-13-gtest`.
- Linux x86_64 build, RP2040 microcontroller (baremetal armv6-m) host: `conan install . --build=missing -profile:build=Linux-x86_64-gcc-13 -profile:host=rp2040-armv6-gcc-13`.
- Linux x86_64 build, RP2040 microcontroller (baremetal armv6-m) host with unit tests: `conan install . --build=missing -profile:build=Linux-x86_64-gcc-13 -profile:host=rp2040-armv6-gcc-13-unity`.<br>

**NOTE**: when performing a cross compile using a host profile you must install the requisite toolchain of the target architecture, [see pre-requisites](#pre-requisites).

The following additional install options are supported:
- enable/disable i8080 arcade support: `--options=with_i8080_arcade=[True|False(default)]`

The following will enable i8080 arcade support: `conan install . --build=missing --profile:all=Windows-x86_64-msvc-193 --options=with_i8080_arcade=True`

The following dependent packages will be installed if required:

- ArduinoJson (for baremetal platforms)
- nlohmann_json (for all other platforms)

**3.** Run cmake to configure and generate the build system.

- Multi configuration generators (MSVC for example): `cmake --preset conan-default [-Wno-dev]`.
- Single configuration generators (make for example): `cmake --preset conan-release [-Wno-dev]`.

**4.** Run cmake to compile meen-hw: `cmake --build --preset conan-release`.

**5.** Run the unit tests:

**Linux/Windows (x86_64)**:
- `artifacts/Release/x86_64/bin/meen_hw_test`.

**Linux (armv7hf, armv8)**:

When running a cross compiled build the binaries need to be uploaded to the host machine before they can be executed.
1. Create an Arm Linux binary distribution: see building a binary development package. 
2. Copy the distribution to the arm machine: `scp build/Release/Sdk/meen-hw-v0.1.0-Linux-6.2.0-39-generic-armv7hf-bin.tar.gz ${user}@raspberrypi:meen-hw-v0.1.0.tar.gz`.
3. Ssh into the arm machine: `ssh ${user}@raspberrypi`.
4. Extract the meen-hw archive copied over via scp: `tar -xzf meen-hw-v0.1.0.tar.gz`.
5. Change directory to meen-hw `cd meen-hw`.
6. Run the unit tests: `./run-meen_hw-tests.sh`.<br>

**RP2040 (armv6-m)**

When running a cross compiled build the binaries need to be uploaded to the host machine before they can be executed.
This example will assume you are deploying the UF2 file from a Raspberry Pi.
1. Create an Arm Linux binary distribution: see building a binary development package.
2. Copy the distribution to the arm machine: `scp build/MinSizeRel/meen-hw-v0.1.0-Linux-6.2.0-39-generic-armv6-bin.tar.gz ${user}@raspberrypi:meen-hw-v0.1.0.tar.gz`.
3. Ssh into the arm machine: `ssh ${user}@raspberrypi`.
4. Extract the meen-hw archive copied over via scp: `tar -xzf meen-hw-v0.1.0.tar.gz`.
5. Hold down the `bootsel` button on the pico and plug in the usb cable into the usb port of the Raspberry Pi then release the `bootsel` button.
6. Echo the attached `/dev` device (this should show up as `sdb1` for example): `dmesg | tail`.
7. Create a mount point (if not done already): `sudo mkdir /mnt/pico`.
8. Mount the device: `sudo mount /dev/sdb1 /mnt/pico`. Run `ls /mnt/pico` to confirm it mounted.
9. Copy the uf2 image to the pico: `cp meen-hw-v0.1.0-Linux-6.2.0-39-generic-armv6-bin/bin/meen_hw_test.uf2 /mnt/pico`
10. You should see a new device `ttyACM0`: `ls /dev` to confirm.
11. Install minicom (if not done already): `sudo apt install minicom`.
12. Run minicom to see test output: `minicom -b 115200 -o -D /dev/ttyACM0`.
    You should see the test output running at 1 second intervals.
13. Quit minicom once done: `ctrl-a, x, enter`
14. Unmount the device: `sudo umount /mnt/pico`.

#### Building a binary development package

A standalone binary package can be built via the `package` target that can be distributed and installed:

- `cmake --build --preset conan-release --target=package`

This will also create doxygen generated documentation (todo) and perform static analysis.

The `package` target as defined by the install targets in the root CMakeLists.txt will build a tar gzipped package which can be replicated by the following cpack command:
- `cpack --config build\CPackConfig.cmake -C ${build_type} -G TGZ`

The underlying package generator used to build the package (in this case `tar`) must be installed otherwise this command will fail.

**NOTE**: the `-G` option can be specifed to overwrite the default `TGZ` cpack generator if a different packaging method is desired:

- `cpack --config build\CPackConfig.cmake -C ${build_type} -G ZIP`

This will build a binary package using the `zip` utility.

Run `cpack --help` for a list available generators.

The final package can be stripped by running the meen_hw_strip_pkg target (defined only for platforms that support strip):
- `cmake --build --preset conan-release --target=meen_hw_strip_pkg`.

For non rp2040 distrubutions the package will contain a script in the root directory called `run-meen_hw-unit-tests` when the unit tests are enabled which can be used to test the development package:
- `./run-meen_hw-unit-tests.sh [--gtest_filter ${gtest_filter}]`.
- `start run-meen_hw-unit-tests.bat [--gtest_filter ${gtest_filter}]`.

### Acknowledgements

Special thanks to the following sites:

[brentradio](http://www.brentradio.com/SpaceInvaders.htm)<br>
[computerarcheology](https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html)<br>
