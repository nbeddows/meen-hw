0.2.1 [04/09/24]
* Updated the install instructions for new meen
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