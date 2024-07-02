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

#include <gtest/gtest.h>

#include "meen_hw/MH_Factory.h"

namespace meen_hw::tests
{
	class MeenHwTest : public testing::Test
	{
	protected:
		static std::unique_ptr<MH_II8080ArcadeIO> i8080ArcadeIO_; 
	public:

		static void SetUpTestCase();
	};

	std::unique_ptr<MH_II8080ArcadeIO> MeenHwTest::i8080ArcadeIO_;

	void MeenHwTest::SetUpTestCase()
	{
		i8080ArcadeIO_ = MakeI8080ArcadeIO();

#ifdef ENABLE_MH_I8080ARCADE
		ASSERT_NE(nullptr, i8080ArcadeIO_);
#else
		ASSERT_EQ(nullptr, i8080ArcadeIO_);
#endif
	}

	TEST_F(MeenHwTest, Version)
	{
		EXPECT_NE(nullptr, Version());	
	}
} // namespace meen_hw::tests

int main(int argc, char** argv)
{
	std::cout << "Running main() from MeenHwTest.cpp" << std::endl;
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
