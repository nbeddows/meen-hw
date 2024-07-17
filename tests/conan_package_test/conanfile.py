import os

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.build import can_run

class MH_PackageTestReceipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires(self.tested_reference_str)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)

        # Need to inform the python test app where the dependent shared libraries are located
        #if self.dependencies["meen_hw"].options.with_python:
        #    if self.dependencies["meen_hw"].options.shared:
        #        tc.variables["artifactsDir"] = self.dependencies["meen_hw"].cpp_info.bindirs[0].replace("\\", "/")
        #    tc.variables["MH_PackageTest"] = True

        tc.generate()

    def layout(self):
        cmake_layout(self)

    def test(self):
        if can_run(self):
            cmd = os.path.join(self.cpp.build.bindir, "MH_PackageTest")
            self.run(cmd, env="conanrun")

            # don't test the python module if we didn't configure it
            #if self.dependencies["meen_hw"].options.with_python:
            #    cmd = os.path.join(self.source_folder, "source/MH_PackageTest.py")
            #    self.run("python " + cmd, env="conanrun")