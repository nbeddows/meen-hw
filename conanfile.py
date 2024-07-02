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
    options = {"shared": [True, False], "fPIC": [True, False], "with_python": [True, False]}
    default_options = {"gtest*:build_gmock": False, "shared": True, "fPIC": True, "with_python": False}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt",\
        "LICENSE",\
        "include/*",\
        "source/*"

    def requirements(self):
        self.requires("nlohmann_json/3.11.3")
        if self.options.with_python:
            self.requires("pybind11/2.12.0")

    def build_requirements(self):
        if not self.conf.get("tools.build:skip_test", default=False):
            self.test_requires("gtest/1.14.0")

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.cache_variables["enablePythonModule"] = self.options.with_python
        tc.variables["buildArch"] = self.settings.arch
        tc.variables["archiveDir"] = self.cpp_info.libdirs[0]
        tc.variables["runtimeDir"] = self.cpp_info.bindirs[0]
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

        #if can_run(self) and not self.conf.get("tools.build:skip_test", default=False):
        #    testsDir = os.path.join(self.source_folder, "artifacts", str(self.settings.build_type), str(self.settings.arch), self.cpp_info.bindirs[0])
        #    self.run(os.path.join(testsDir, "MeenHwTest "))
        #    if self.options.with_python:
        #        cmd = os.path.join(self.source_folder, "tests/meen_hw/source/test_MeenHw.py -v ")
        #        self.run("python " + cmd)

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = [self.name]
