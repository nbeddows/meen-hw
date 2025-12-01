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

cmake_minimum_required (VERSION 3.25)
set(include_dir include)
set(resource_dir resource)
set(source_dir source)
set(lib_name meen_hw)
set(major 0)
set(minor 4)
set(bugfix 0)

if(NOT ${enable_board} STREQUAL "none")
  set(PICO_BOARD ${enable_board})
  include ($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
endif()

project(${lib_name} VERSION ${major}.${minor}.${bugfix} LANGUAGES C CXX ASM)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(NOT ${enable_board} STREQUAL "none")
  pico_sdk_init()
endif()

if(DEFINED MSVC)
  set(build_type $<CONFIG>)
else()
  set(build_type ${CMAKE_BUILD_TYPE})
endif()

if(NOT DEFINED build_arch)
  set(build_arch ${CMAKE_SYSTEM_PROCESSOR})
endif()

if(NOT DEFINED build_os)
  set(build_os ${CMAKE_SYSTEM_NAME})
endif()

if(NOT DEFINED archive_dir)
  set(archive_dir "lib")
endif()

if(NOT DEFINED runtime_dir)
  set(runtime_dir "bin")
endif()

if(${BUILD_SHARED_LIBS} STREQUAL ON)
  set(lib_type SHARED)

  if(DEFINED WIN32)
    set(ext dll)
  else()
    set(ext so)
  endif()
else()
  set(lib_type STATIC)

  if(DEFINED WIN32)
    set(ext lib)
  else()
    set(ext a)
  endif()
endif()

# TODO: rp2040 does not output all artifacts correctly when setting this - investigate me
if(${enable_board} STREQUAL "none")
  set(artifacts_dir $<1:${CMAKE_SOURCE_DIR}/artifacts/${build_type}/${build_arch}>)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${artifacts_dir}/${archive_dir})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${artifacts_dir}/${runtime_dir})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${artifacts_dir}/${runtime_dir})
endif()

# if any emulated hardware is enabled
if(${enable_i8080_arcade} STREQUAL ON)
    find_package(ArduinoJson REQUIRED)
endif()

#if(enable_python_module STREQUAL ON)
#  find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
#  find_package(pybind11 REQUIRED)
#endif()

set (${lib_name}_private_include_files
	${include_dir}/${lib_name}/utils/MH_ErrorCode.h
)

set (${lib_name}_public_include_files
  ${include_dir}/${lib_name}/MH_ConditionVariable.h
  ${include_dir}/${lib_name}/MH_Error.h
  ${include_dir}/${lib_name}/MH_Factory.h
  ${include_dir}/${lib_name}/MH_II8080ArcadeIO.h
  ${include_dir}/${lib_name}/MH_Mutex.h
  ${include_dir}/${lib_name}/MH_ResourcePool.h
)

if(DEFINED MSVC)
  include(${resource_dir}/generate_product_version.cmake)

  generate_product_version(
    versionResource
    COMPANY_NAME "Nicolas Beddows"
    FILE_DESCRIPTION "Machine Emulator ENgine HardWare"
    NAME "MEEN Hardware"
    ORIGINAL_FILE_NAME "${lib_name}.dll"
    PRODUCT_ICON ""
    VERSION_MAJOR ${major}
    VERSION_MINOR ${minor}
    VERSION_PATCH ${bugfix}
  )

  set (${lib_name}_resource_files
    ${versionResource}
  )
endif()

set (${lib_name}_source_files
  ${source_dir}/MH_Error.cpp
  ${source_dir}/MH_Factory.cpp
)

if(${enable_i8080_arcade} STREQUAL ON)
  set (${lib_name}_i8080_arcade_include_files
    ${include_dir}/${lib_name}/i8080_arcade/MH_I8080ArcadeIO.h
  )

  set (${lib_name}_i8080_arcade_source_files
    ${source_dir}/i8080_arcade/MH_I8080ArcadeIO.cpp
  )

  SOURCE_GROUP(i8080_arcade/${include_dir} FILES ${${lib_name}_i8080_arcade_include_files})
  SOURCE_GROUP(i8080_arcade/${source_dir} FILES ${${lib_name}_i8080_arcade_source_files})
endif()

SOURCE_GROUP(${lib_name}/${include_dir} FILES ${${lib_name}_private_include_files} ${${lib_name}_public_include_files})
SOURCE_GROUP(${lib_name}/${resource_dir} FILES ${${lib_name}_resource_files})
SOURCE_GROUP(${lib_name}/${source_dir} FILES ${${lib_name}_source_files})

add_library(${lib_name} ${lib_type}
  ${${lib_name}_private_include_files}
  ${${lib_name}_public_include_files}
  ${${lib_name}_resource_files}
  ${${lib_name}_source_files}
  ${${lib_name}_i8080_arcade_include_files}
  ${${lib_name}_i8080_arcade_source_files}
)

string(TOUPPER ${lib_name} lib_name_toupper)

target_include_directories(${lib_name} PRIVATE ${CMAKE_SOURCE_DIR}/${include_dir})
set_target_properties(${lib_name} PROPERTIES VERSION ${CMAKE_PROJECT_VERSION} SOVERSION ${CMAKE_PROJECT_VERSION})
target_compile_definitions(${lib_name} PRIVATE ${lib_name_toupper}_VERSION=\"${CMAKE_PROJECT_VERSION}\")
target_compile_definitions(${lib_name} PUBLIC ${lib_name}_${lib_type})

if(${build_os} STREQUAL "baremetal")
  target_compile_options(${lib_name} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti>)
endif()

# if any emulated hardware is enabled
if(${enable_i8080_arcade} STREQUAL ON)
  target_compile_definitions(${lib_name} PUBLIC ENABLE_MH_I8080ARCADE)
  target_link_libraries(${lib_name} PRIVATE ArduinoJson)
endif()

if (NOT DEFINED BUILD_TESTING OR NOT ${BUILD_TESTING} STREQUAL OFF)
  add_subdirectory(tests/${lib_name}_test)
endif()

set(CMAKE_INSTALL_PREFIX ./)
set(CPACK_PACKAGE_FILE_NAME ${lib_name}-v${CMAKE_PROJECT_VERSION}-${build_os}-${build_arch}-${CMAKE_C_COMPILER_ID}-${CMAKE_C_COMPILER_VERSION})
set(CPACK_GENERATOR TGZ)

if(DEFINED CMAKE_STRIP)
  add_custom_target(${lib_name}_strip_pkg
    COMMAND ${CMAKE_COMMAND} -E tar -xzf ${CPACK_PACKAGE_FILE_NAME}.tar.gz
    COMMAND ${CMAKE_STRIP} -s ${CPACK_PACKAGE_FILE_NAME}/${runtime_dir}/${lib_name}_test
    COMMAND ${CMAKE_STRIP} -s ${CPACK_PACKAGE_FILE_NAME}/${archive_dir}/lib${lib_name}.${ext}
    COMMAND ${CMAKE_COMMAND} -E tar czf ${CPACK_PACKAGE_FILE_NAME}.tar.gz ${CPACK_PACKAGE_FILE_NAME}/
    COMMAND ${CMAKE_COMMAND} -E rm -r ${CPACK_PACKAGE_FILE_NAME}/
  )
endif()

target_sources(${lib_name} INTERFACE FILE_SET HEADERS BASE_DIRS ${include_dir} FILES ${${lib_name}_public_include_files})

install(CODE "execute_process(COMMAND doxygen ${CMAKE_SOURCE_DIR}/docs/Doxyfile)")
if(DEFINED MSVC)
  # Under Windows Doxygen documentaiton causes LaTeX to go into what looks to be an infinite loop, disable until a solution is found
  # install(CODE "execute_process(COMMAND cmd /c make WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/latex)")
# We are not going to build the documentation on arm platforms
elseif(${build_arch} STREQUAL x86_64)
  install(CODE "execute_process(COMMAND make WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/latex)")
endif()
install(TARGETS ${lib_name} FILE_SET HEADERS)
install(FILES CHANGELOG.md DESTINATION .)
install(FILES LICENSE DESTINATION .)
# Latex to PDF generation under Windows is currently not working, remove the if protection once fixed
if(NOT DEFINED MSVC)
  if(${build_arch} STREQUAL x86_64)
    install(FILES ${CMAKE_BINARY_DIR}/latex/refman.pdf RENAME MEEN_HW_Reference.pdf DESTINATION .)
  endif()
endif()

if(NOT DEFINED WIN32)
  set(CPACK_SET_DESTDIR ON)
endif()

include(CPack)
