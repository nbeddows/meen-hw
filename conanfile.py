from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.build import can_run
import os

class MeenHwRecipe(ConanFile):
    name = "meen_hw"
    version = "0.2.1"
    package_type = "library"
    test_package_folder = "tests/conan_package_test"

    # Optional metadata
    license = "MIT"
    author = "Nicolas Beddows <nicolas.beddows@gmail.com>"
    url = "https://github.com/nbeddows"
    description = "Machine Emulator Engine Hardware"
    topics = ("emulator", "i8080")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False], "with_i8080_arcade": [True, False], "with_python": [True, False], "with_rp2040": [True, False]}
    default_options = {"gtest*:build_gmock": False, "shared": True, "fPIC": True, "with_i8080_arcade": False, "with_python": False, "with_rp2040": False}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt",\
        "CHANGELOG.md",\
        "LICENSE",\
        "README.md",\
        "include/*",\
        "resource/*",\
        "source/*",\
        "tests/meen_hw_test/*"

    def requirements(self):
        # if any hardware has been set
        if self.options.with_i8080_arcade:
            if self.settings.os == "baremetal":
                self.requires("arduinojson/7.0.1")
            else:
                self.requires("nlohmann_json/3.11.3")

        if self.options.get_safe("with_python", False):
            self.requires("pybind11/2.12.0")

    def build_requirements(self):
        if not self.conf.get("tools.build:skip_test", default=False):
            if self.settings.os == "baremetal":
                self.test_requires("unity/2.6.0")
            else:
                self.test_requires("gtest/1.14.0")

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

            if "arm" in self.settings.arch:
                self.output.error("Compiling for Windows ARM is not supported")
    
            if self.settings_build.os == "Linux" or self.settings_build == "baremetal":
                self.output.error("Cross compiling from Linux or baremetal to Windows is not supported")                

        if "arm" in self.settings.arch:
            self.output.info("Python ARM module not supported, removing option with_python.")
            self.options.rm_safe("with_python")

        if self.settings_build.os == "Windows":
            self.output.info("Cross compiling with RP2040 support from Windows is not supported, removing option with_rp2040")
            self.options.rm_safe("with_rp2040")

            if self.settings.os == "Linux" or self.settings.os == "baremetal":
                self.output.error("Cross compiling from Windows to Linux or baremetal is not supported")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")
            self.options.rm_safe("with_rp2040")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.cache_variables["enable_python_module"] = self.options.get_safe("with_python", False)
        tc.cache_variables["enable_i8080_arcade"] = self.options.with_i8080_arcade
        tc.cache_variables["enable_rp2040"] = self.options.get_safe("with_rp2040", False)
        tc.variables["build_os"] = self.settings.os
        tc.variables["build_arch"] = self.settings.arch
        tc.variables["archive_dir"] = self.cpp_info.libdirs[0]
        tc.variables["runtime_dir"] = self.cpp_info.bindirs[0]
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

        if can_run(self) and not self.conf.get("tools.build:skip_test", default=False):
            testsDir = os.path.join(self.source_folder, "artifacts", str(self.settings.build_type), str(self.settings.arch), self.cpp_info.bindirs[0])
            self.run(os.path.join(testsDir, "meen_hw_test "))
        #    if self.options.get_safe("with_python", False):
        #        cmd = os.path.join(self.source_folder, "tests/meen_hw/source/test_MeenHw.py -v ")
        #        self.run("python " + cmd)

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = [self.name]
