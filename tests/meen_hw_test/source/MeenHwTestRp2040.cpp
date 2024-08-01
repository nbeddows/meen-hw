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

#include <pico/stdlib.h>

#include "meen_hw/MH_Factory.h"

namespace meen_hw::tests
{
	static std::unique_ptr<MH_II8080ArcadeIO> i8080ArcadeIO_;

        static int SetUpTestCase()
        {
            i8080ArcadeIO_ = MakeI8080ArcadeIO();
#ifdef ENABLE_MH_I8080ARCADE
               if (i8080ArcadeIO_ == nullptr)
			{
				printf("SetUpTestCase FAILED: expected i8080Arcade NOT EQUAL nullptr\n");
				return -1;
			}
#else
    		if (i8080ArcadeIO_ != nullptr)
			{
				printf("SetUpTestCase FAILED: expected i8080Arcade EQUAL nullptr\n");
				return -1;
			}
#endif
			printf("SetUpTestCase: SUCCESS\n");
			return 0;
        }

        static int VersionStr()
        {
            if (Version() == nullptr)
			{
				printf("VersionStr FAILED: expected Version NOT EQUAL nullptr");
			}
			else
			{
				printf("VersionStr: SUCCESS\n");
			}

			return 0;
        }
}

int main(void)
{
	setup_default_uart();

	auto checkErr = [](int err)
	{
		if (err < 0)
		{
			printf("FATAL error, halting tests");
			exit(0);
		}
	};

	auto err = meen_hw::tests::SetUpTestCase();
	checkErr(err);

	err = meen_hw::tests::VersionStr();
	checkErr(err);

	return 0;
}
