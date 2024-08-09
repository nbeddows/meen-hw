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

set(exe_name ${lib_name}_test)

if(enable_rp2040 STREQUAL ON)
  find_package(Unity REQUIRED)
  set(${exe_name}_source_files ${source_dir}/MeenHwUnityTest.cpp)
  set(${exe_name}_deps pico_stdlib unity::unity)
else()
  find_package(GTest REQUIRED)
  set(${exe_name}_source_files ${source_dir}/MeenHwGTest.cpp)
  set(${exe_name}_deps GTest::GTest)
endif()

SOURCE_GROUP("Source" FILES ${${exe_name}_source_files})

add_executable(${exe_name} ${${exe_name}_source_files})

set_target_properties(${exe_name} PROPERTIES FOLDER tests)
target_link_libraries(${exe_name} PRIVATE
  ${${exe_name}_deps}
  ${lib_name}
)

install(TARGETS ${exe_name} RUNTIME)

if(enable_rp2040 STREQUAL ON)
  pico_enable_stdio_usb(${exe_name} 1)
  pico_enable_stdio_uart(${exe_name} 0)
  pico_set_program_url(${exe_name} "https://github.com/neddows/meen-hw")
  pico_add_extra_outputs(${exe_name})
  install(PROGRAMS ${CMAKE_BINARY_DIR}/tests/${exe_name}/${exe_name}.bin DESTINATION ./bin)
  install(FILES ${CMAKE_BINARY_DIR}/tests/${exe_name}/${exe_name}.uf2 DESTINATION ./bin)
  install(FILES ${CMAKE_BINARY_DIR}/tests/${exe_name}/${exe_name}.dis DESTINATION ./lib)
  install(FILES ${CMAKE_BINARY_DIR}/tests/${exe_name}/${exe_name}.elf.map DESTINATION ./lib)
  install(FILES ${CMAKE_BINARY_DIR}/tests/${exe_name}/${exe_name}.hex DESTINATION ./lib)
else()
  if(DEFINED WIN32)
    set(scriptExt .bat)

    file(WRITE ${CMAKE_BINARY_DIR}/run-${lib_name}-unit-tests${scriptExt}
    "@echo off\n\
set PATH=%cd%\\bin;%PATH%\n\
bin\\${lib_name}_test\n\
pause\n\
exit"
  )
  else()
    set(scriptExt .sh)

    file(WRITE ${CMAKE_BINARY_DIR}/run-${lib_name}-unit-tests${scriptExt}
    "#!/bin/bash\n\
export LD_LIBRARY_PATH=`pwd`/bin:\${LD_LIBRARY_PATH}\n\
bin/${lib_name}_test"
)
  endif()

  install(PROGRAMS ${CMAKE_BINARY_DIR}/run-${lib_name}-unit-tests${scriptExt} DESTINATION .)
endif()
