cmake_minimum_required(VERSION 3.25)
set(project_name MH_PackageTest)
project(${project_name} CXX)

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

target_link_libraries(${project_name} meen_hw::meen_hw)
