* Added MH_ResourcePool::Wait method.
* Added MH_ResourcePool::GetSize method.
* Added MH_ResourcePool::GetAvailable method.
* Added MH_ConditionVariable which wraps supported
  platform implementations.

0.4.0 [26/07/25]
* Added testing and release workflows for GitHub Actions CI/CD.
* Dropped GCC-12 support.
* Added GCC-14 support.
* Added support for the latest pico sdk (2.1.1) while maintaing
  support for sdk version 1.5.1.
* Embed common information in meen_hardware_test.uf2.
* Replaced Conan option `with_rp2040` with `with_board`.
* Added the Conan option `with_framework`.
* Using ArduinoJson exclusively.
* Updated GTest dependency to 1.16.0.

0.3.0 [23/06/25]
* SetOptions option `bpp` now supports `16` for `16bpp`
  blits via `MH_I8080ArcadeIO::BlitVRAM`.
* Change blit flag `Rgb332` to `bpp8`.
* Added blit flags `bpp16` and `Upright16bpp` for
  16bit blit support.
* The `MH_I8080ArcadeIO::BlitVRAM` method can now blit
  from sources to destinations of varying dimensions.
* Fixed a bug where parsing the `colour` option `blue`
  would fail.
* Add the interface method `MH_I8080ArcadeIO::Reset`
  to reset the non-configurable state.
* Added PDF documentation.

0.2.1 [04/09/24]
* Updated the install instructions for new MEEN
  conan config profiles.
* Compiler id and version are now incorporated
  into the package name.
* Improved conan file compatibility checks.

0.2.0 [19/08/24]
* Replaced exceptions with std::error_code.
* Improved conan option handling.
* Added RP2040 support.
* Added support for Unity test framework for
  platforms that don't support GTest.
* Using well defined Conan profiles (not default).
* Updated the README for Conan profiles install.
* Added support for ArduinoJson for baremetal.

0.1.0 [17/07/24]
* Added i8080 arcade and resource pool unit tests.
* Moved and refactored the i8080 arcade hardware code
  from the space-invaders repo into MH_I8080ArcadeIO.
* Added a class factory MH_Factory.h.
* Added a basic resource pool MH_ResourcePool.h. 
* Added CMake and Conan environment support.
* Added a Conan test package to confirm correct
  meen_hw package consumption.
* Added change log and readme documents.
* Initial release.