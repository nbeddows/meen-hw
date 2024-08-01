### Introduction

Machine Emulator ENgine Hardware is a collection of 8bit emulated hardwares designed to be used as helper modules when building controllers that fit into the [Machine Emulator ENgine framework](http://github.com/nbeddows/mach-emu/).

Supported hardwares:

- i8080 arcade - hardware emulation based on the 1978 Midway/Taito Space Invaders arcade machine. Along with the original Space Invaders title, this emulated hardware is also compatible with Lunar Rescue (1979), Balloon Bomber (1980) and Space Invaders Part II/Deluxe (1980). **NOTE**: currently does not support emulated audio, the consuming application needs to provide audio samples.

### Compilation

This project uses [CMake (minimum version 3.23)](https://cmake.org/) for its build system and [Conan (minimum version 2.0)](https://conan.io/) for it's dependency package management. Supported compilers are GCC (minimum version 12), MSVC(minimum version 16) and Clang (minimum version 16).

#### Pre-requisites

##### Linux

- [Install Conan](https://conan.io/downloads/).
- `sudo apt install cmake`.
- cross compilation:
  - armvv7hf:
    - `sudo apt install gcc-arm-linux-gnueabihf`.
  - aarch64:
    - `sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu`.
  - rp2040:
    - `sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential libstdc++-arm-none-eabi-newlib`.
    - `git clone https://github.com/raspberrypi/pico-sdk.git`
    - `cd pico-sdk`
    - `git submodule update --init`
    - build the Raspberry Pi SDK:
      - Conan and the Raspberry Pi Pico Sdk seem to have an issue with conflicting use of the cmake toolchain file
        which results in test programs not being able to be compiled during the conan build process as outlined [here](https://github.com/raspberrypi/pico-sdk/issues/1693).
        At this point we need to build the sdk so that we have the required tools pre-built so the Conan build process will succeed:
        - `cd pico-sdk`
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

**1.** Create a default profile (if you have no profiles): `conan profile detect`. This will detect the operating system, build architecture, compiler settings and set the build configuration as Release by default. The profile will be named `default` and will reside in $HOME/.conan2/profiles. 

**2.** Install dependencies:
- Using the default build and host profiles: `conan install . --build=missing`.
- Using the default build profile targeting 32 bit Raspberry Pi OS: `conan install . --build=missing -pr:h=profiles/raspberry-32`.<br>
- Using the default build profile targeting 64 bit Raspberry Pi OS: `conan install . --build=missing -pr:h=profiles/raspberry-64`.<br>
- Using the default build profile targeting the Raspberry Pi 2040 microcontroller: `conan install . --build=missing -pr:h=profiles/rp2040`.<br>

**NOTE**: raspberry host profiles can be obtained from the [mach-emu project](https://github.com/nbeddows/mach-emu/tree/main/profiles).

**NOTE**: when performing a cross compile using a host profile you must install the requisite toolchain of the target architecture, [see pre-requisites](#pre-requisites).

The following install options are supported:
- build/don't build the unit tests: `--conf=tools.build:skip_test=[True|False(default)]`
- enable/disable i8080 arcade support: `--options=with_i8080_arcade=[True|False(default)]`
- enable/disable rp2040 support: `--options=with_rp2040=[True|False(default)]`

The following will enable i8080 arcade support: `conan install . --build=missing --options=with_i8080_arcade=True`

The following dependent packages will be installed if required:

- nlohmann_json

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
2. Copy the distribution to the arm machine: `scp build/Release/Sdk/meen-hw-v0.1.0-Linux-armv7hf-bin.tar.gz ${user}@raspberrypi:meen-hw-v0.1.0.tar.gz`.
3. Ssh into the arm machine: `ssh ${user}@raspberrypi`.
4. Extract the meen-hw archive copied over via scp: `tar -xzf meen-hw-v0.1.0.tar.gz`.
5. Change directory to space-invaders `cd meen-hw`.
6. Run the unit tests: `./run-meen_hw-tests.sh`.<br>

**RP2040 (armv6-m)**

# TODO: document running from the rp2040 microcontroller

#### Building a binary development package

A standalone binary package can be built via `cpack` that can be distributed and installed:

- `cmake --build --preset conan-release --target=meen-hw-pkg`

The `meen-hw-pkg` target defined in the root CMakeLists.txt will build a tar gzipped package via the following cpack command:

- `cpack --config build\CPackConfig.cmake -C ${build_type} -G TGZ`

The underlying package generator used to build the package (in this case `tar`) must be installed otherwise this command will fail.

**NOTE**: the `-G` option can be specifed to overwrite the default `TGZ` cpack generator if a different packaging method is desired:

- `cpack --config build\CPackConfig.cmake -C ${build_type} -G ZIP`

This will build a binary package using the `zip` utility.

Run `cpack --help` for a list available generators.

The `meen-hw-pkg` target will also strip all binary files where applicable and will also perform static analysis of the source code via `cppcheck`.

For non rp2040 distrubutions the package will contain a script in the root directory called `run-meen_hw-unit-tests` when the unit tests are enabled which can be used to test the development package:
- `./run-meen_hw-unit-tests.sh [--gtest_filter ${gtest_filter}]`.
- `start run-meen_hw-unit-tests.bat [--gtest_filter ${gtest_filter}]`.

### Acknowledgements

Special thanks to the following sites:

[brentradio](http://www.brentradio.com/SpaceInvaders.htm)<br>
[computerarcheology](https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html)<br>
