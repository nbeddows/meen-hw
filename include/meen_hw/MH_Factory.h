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

#ifndef MEEN_HW_MH_FACTORY_H
#define MEEN_HW_MH_FACTORY_H

#include <memory>

#include "MH_II8080ArcadeIO.h"

#ifdef _WINDOWS
#ifdef meen_hw_EXPORTS
#define DLL_EXP_IMP __declspec(dllexport)
#else
#define DLL_EXP_IMP __declspec(dllimport)
#endif
#else
#ifdef meen_hw_EXPORTS
#define DLL_EXP_IMP [[gnu::visibility("default")]]
#else
#define DLL_EXP_IMP
#endif
#endif

namespace meen_hw
{
	/**
		The shared library version

		@return A string containing the current version, in a format
				described by [semantic versioning](https://semver.org/)

				```
				<major>"."<minor>"."<patch>
				<major>"."<minor>"."<patch>"-"<pre-release>
				<major>"."<minor>"."<patch>"+"<build>
				<major>"."<minor>"."<patch>"-"<pre-release>"+"<build>
				```
	*/
	DLL_EXP_IMP const char* Version();

	DLL_EXP_IMP std::unique_ptr<MH_II8080ArcadeIO> MakeI8080ArcadeIO();
} // namespace meen_hw

#endif // MEEN_HW_MH_FACTORY_H