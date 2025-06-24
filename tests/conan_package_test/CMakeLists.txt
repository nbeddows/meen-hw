# Copyright (c) 2021-2025 Nicolas Beddows <nicolas.beddows@gmail.com>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.25)
set(project_name MH_PackageTest)

if(${enable_rp2040} STREQUAL ON)
  include ($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
endif()

project(${project_name} LANGUAGES C CXX ASM)

if(NOT BUILD_SHARED_LIBS STREQUAL ON)
  if(DEFINED WIN32)
    set(static_lib meen_hw_STATIC)
  endif()
endif()

if(${enable_rp2040} STREQUAL ON)
  add_executable(Pioasm IMPORTED)
  set_property(TARGET Pioasm PROPERTY IMPORTED_LOCATION $ENV{PICO_SDK_PATH}/build/pioasm/pioasm)
  set(Pioasm_FOUND 1)

  add_executable(ELF2UF2 IMPORTED)
  set_property(TARGET ELF2UF2 PROPERTY IMPORTED_LOCATION $ENV{PICO_SDK_PATH}/build/elf2uf2/elf2uf2)
  set(ELF2UF2_FOUND 1)

  pico_sdk_init()
endif()

find_package(meen_hw CONFIG REQUIRED)

add_executable(${project_name} source/${project_name}.cpp)

#if(DEFINED artifactsDir)
#    if(MSVC)
#        set(buildType $<CONFIG>)
#    else()
#        set(buildType ${CMAKE_BUILD_TYPE})
#    endif()

#    include(${CMAKE_SOURCE_DIR}/../conan_package_test/pythonTestDeps.cmake)
#    generatePythonUnitTestDeps(${artifactsDir} testControllersPathNotUsed testProgramsPathNoUsed)

#    add_custom_command(
#        TARGET ${project_name} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/MH_TestDeps${buildType}.py ${CMAKE_CURRENT_SOURCE_DIR}/source/MH_TestDeps.py
#    )
#endif()

target_compile_definitions(${project_name} PRIVATE ${static_lib})
target_link_libraries(${project_name} meen_hw::meen_hw)

if(${enable_rp2040} STREQUAL ON)
  target_compile_definitions(${project_name} PRIVATE ENABLE_MH_RP2040)
  target_compile_options(${project_name} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti>)
  pico_enable_stdio_usb(${project_name} 1)
  pico_enable_stdio_uart(${project_name} 0)
  pico_set_program_url(${project_name} "https://github.com/neddows/meen-hw")
  pico_add_extra_outputs(${project_name})
  target_link_libraries(${project_name} pico_stdlib)
endif()
