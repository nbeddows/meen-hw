# Copyright (c) 2021-2024 Nicolas Beddows <nicolas.beddows@gmail.com>

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
set(minor 1)
set(bugfix 0)

project(${lib_name} VERSION ${major}.${minor}.${bugfix})

if(MSVC)
  set(build_type $<CONFIG>)
else()
  set(build_type ${CMAKE_BUILD_TYPE})
endif()

if(NOT DEFINED build_arch)
  set(build_arch ${CMAKE_SYSTEM_PROCESSOR})
endif()

if(NOT DEFINED archive_dir)
  set(archive_dir "lib")
endif()

if(NOT DEFINED runtime_dir)
  set(runtime_dir "bin")
endif()

if(${BUILD_SHARED_LIBS} STREQUAL ON)
  set(lib_type SHARED)
else()
  set(lib_type STATIC)
endif()

set(artifacts_dir $<1:${CMAKE_SOURCE_DIR}/artifacts/${build_type}/${build_arch}>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${artifacts_dir}/${archive_dir})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${artifacts_dir}/${runtime_dir})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${artifacts_dir}/${runtime_dir})

find_package(nlohmann_json REQUIRED)

#if(enable_python_module STREQUAL ON)
#    find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
#    find_package(pybind11 REQUIRED)
#endif()

set (${lib_name}_private_include_files
	${include_dir}/${lib_name}/MH_Error.h
)

set (${lib_name}_public_include_files
	${include_dir}/${lib_name}/MH_Factory.h
	${include_dir}/${lib_name}/MH_II8080ArcadeIO.h
	${include_dir}/${lib_name}/MH_ResourcePool.h
)

if(MSVC)
  include(${resource_dir}/generate_product_version.cmake)

  generate_product_version(
    versionResource
    COMPANY_NAME "Nicolas Beddows"
    FILE_DESCRIPTION "Machine Emulator Engine Hardware"
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

if(enable_i8080_arcade STREQUAL ON)
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

target_compile_definitions(${lib_name} PRIVATE
  ${lib_name_toupper}_VERSION=\"${CMAKE_PROJECT_VERSION}\"
)

if(enable_i8080_arcade STREQUAL ON)
  target_compile_definitions(${lib_name} PUBLIC ENABLE_MH_I8080ARCADE)
endif()

if(enable_rp2040 STREQUAL ON)
  target_compile_definitions(${lib_name} PUBLIC ENABLE_MH_RP2040)
  target_compile_options(${lib_name} PUBLIC -fno-exceptions)
  # this emits warnings when compiling the rp2040 source rightfully stating that it is not valid for C code, so we make it private
  target_compile_options(${lib_name} PRIVATE -fno-rtti)
endif()

target_include_directories(${lib_name} PRIVATE
  ${CMAKE_SOURCE_DIR}/${include_dir}
)

set_target_properties(${lib_name} PROPERTIES VERSION ${CMAKE_PROJECT_VERSION} SOVERSION ${CMAKE_PROJECT_VERSION})

target_link_libraries(${lib_name} PRIVATE
  nlohmann_json::nlohmann_json
)

if (NOT BUILD_TESTING STREQUAL OFF)
  add_subdirectory(tests/${lib_name}_test)
endif()

set(CMAKE_INSTALL_PREFIX ./)
set(CPACK_PACKAGE_FILE_NAME ${lib_name}-v${CMAKE_PROJECT_VERSION}-${CMAKE_SYSTEM}-${CMAKE_SYSTEM_PROCESSOR}-${runtime_dir})

target_sources(${lib_name} INTERFACE FILE_SET HEADERS BASE_DIRS ${include_dir} FILES
  ${${lib_name}_public_include_files}
)

install(TARGETS ${lib_name} FILE_SET HEADERS)
install(FILES LICENSE DESTINATION .)
install(FILES README.md DESTINATION .)

if(NOT DEFINED WIN32)
    set(CPACK_SET_DESTDIR ON)
endif()

include(CPack)