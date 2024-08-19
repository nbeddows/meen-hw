/*
Copyright (c) 2021-2024 Nicolas Beddows <nicolas.beddows@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <cstdio>
#ifdef ENABLE_MH_RP2040
#include <pico/stdlib.h>
#endif
#include "meen_hw/MH_Factory.h"

void printEnvironment(const char* version)
{
#ifdef NDEBUG
    printf("meen_hw/%s: Machine Emulator Engine Hardware Release\n", version);
#else
    printf("meen_hw/%s: Machine Emulator Engine Hardware Debug\n", version);
#endif

// ARCHITECTURES
#ifdef _M_X64
    printf("  meen_hw/%s: _M_X64 defined\n", version);
#endif

#ifdef _M_IX86
    printf("  meen_hw/%s: _M_IX86 defined\n", version);
#endif

#ifdef _M_ARM64
    printf("  meen_hw/%s: _M_ARM64 defined\n", version);
#endif

#if __i386__
    printf("  meen_hw/%s: __i386__ defined\n", version);
#endif

#if __x86_64__
    printf("  meen_hw/%s: __x86_64__ defined\n", version);
#endif

#if __aarch64__
    printf("  meen_hw/%s: __aarch64__ defined\n", version);
#endif

// Libstdc++
#if defined _GLIBCXX_USE_CXX11_ABI
    printf("  meen_hw/%s: _GLIBCXX_USE_CXX11_ABI %d\n", version, _GLIBCXX_USE_CXX11_ABI);
#endif

// MSVC runtime
#if defined(_DEBUG)
    #if defined(_MT) && defined(_DLL)
        printf("  meen_hw/%s: MSVC runtime: MultiThreadedDebugDLL\n", version);
    #elif defined(_MT)
        printf("  meen_hw/%s: MSVC runtime: MultiThreadedDebug\n", version);
    #endif
#else
    #if defined(_MT) && defined(_DLL)
        printf("  meen_hw/%s: MSVC runtime: MultiThreadedDLL\n", version);
    #elif defined(_MT)
        printf("  meen_hw/%s: MSVC runtime: MultiThreaded\n", version);
    #endif
#endif

// COMPILER VERSIONS
#if _MSC_VER
    printf("  meen_hw/%s: _MSC_VER %d\n", version, _MSC_VER);
#endif

#if _MSVC_LANG
    printf("  meen_hw/%s: _MSVC_LANG %d\n", version, _MSVC_LANG);
#endif

#if __cplusplus
    printf("  meen_hw/%s: __cplusplus %d\n", version, __cplusplus);
#endif

#if __INTEL_COMPILER
    printf("  meen_hw/%s: __INTEL_COMPILER %d\n", version, __INTEL_COMPILER);
#endif

#if __GNUC__
    printf("  meen_hw/%s: __GNUC__ %d\n", version, __GNUC__);
#endif

#if __GNUC_MINOR__
    printf("  meen_hw/%s: __GNUC_MINOR__%d\n", version, __GNUC_MINOR__);
#endif

#if __clang_major__
    printf("  meen_hw/%s: __clang_major__ %d\n", version, __clang_major__);
#endif

#if __clang_minor__
    printf("  meen_hw/%s: __clang_minor__ %d\n", version, __clang_minor__);
#endif

#if __apple_build_version__
    printf("  meen_hw/%s: __apple_build_version__ %d", version, __apple_build_version__);
#endif

// SUBSYSTEMS
#if __MSYS__
    printf("  meen_hw/%s: __MSYS__ %d\n", version, __MSYS__);
#endif

#if __MINGW32__
    printf("  meen_hw/%s: __MINGW32__ %d\n", version, __MINGW32__);
#endif

#if __MINGW64__
    printf("  meen_hw/%s: __MINGW64__ %d\n", version, __MINGW64__);
#endif

#if __CYGWIN__
    printf("  meen_hw/%s: __CYGWIN__ %d\n", version, __CYGWIN__);
#endif
}

// Simple program that tests that the meen_hw package is configured correctly.
// It then prints out the meen_hw version along with environment information.
int main()
{
#ifdef ENABLE_MH_RP2040
    stdio_init_all();

    while(true)
#endif
    {
        printEnvironment(meen_hw::Version());

#ifdef ENABLE_MH_RP2040
        sleep_ms(1000);
#endif

    }
    return 0;
}
