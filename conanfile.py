from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.build import can_run
import os

class MeenHwRecipe(ConanFile):
    name = "meen_hw"
    version = "0.1.0"
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
        "LICENSE",\
        "README.md",\
        "pico_sdk_import.cmake",\
        "include/*",\
        "resource/*",\
        "source/*",\
        "tests/meen_hw_test/*"

    def requirements(self):
        self.requires("nlohmann_json/3.11.3")
        if self.options.get_safe("with_python", False):
            self.requires("pybind11/2.12.0")

    def build_requirements(self):
        if not self.conf.get("tools.build:skip_test", default=False):
            if self.options.get_safe("with_rp2040", False):
                self.test_requires("unity/2.6.0")
            else:
                self.test_requires("gtest/1.14.0")

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")
            # Currently not supported (maybe in the future)
            self.options.rm_safe("with_rp2040")
            if "arm" in self.settings.arch:
                self.output.error("Compiling for ARM under Windows OS is currently not supported, use Linux OS")

        if "arm" in self.settings.arch:
            self.options.rm_safe("with_python")
    
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
        #    if self.options.with_python:
        #        cmd = os.path.join(self.source_folder, "tests/meen_hw/source/test_MeenHw.py -v ")
        #        self.run("python " + cmd)

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = [self.name]
