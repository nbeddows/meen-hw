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

#ifdef ENABLE_MH_I8080ARCADE
	TEST_F(MeenHwTest, ReadPort0)
	{
		auto value = i8080ArcadeIO_->ReadPort(0);
		//https://www.reddit.com/r/EmuDev/comments/mvpt4w/space_invaders_part_ii_deluxe_emulator/
		EXPECT_EQ(0x40, value);
	}

	TEST_F(MeenHwTest, WritePorts3And5)
	{
		/*
			Writing to ports 3 and 5 will write to the audio hardware.

			Each bit of data written to ports 3 and 5 represents a
			sound effect. Identical bits of consecutive port writes
			are ignored (no audio sound effect shall be repeated).

			The only exception to this is bit 0 on port 3 which is
			repeated.
		*/

		auto writePort = [this](int port)
		{
			// Try to play audio sound effect 2
			auto value = i8080ArcadeIO_->WritePort(port, 0x02);
			// We should get audio sound effect 2
			EXPECT_EQ(0x02, value);
			// Try to play the audio sound effect 2 again
			value = i8080ArcadeIO_->WritePort(port, 0x02);
			// The audio sound effect does not repeat
			EXPECT_EQ(0x00, value);
			// Try to play audio sound effects 2 and 3
			value = i8080ArcadeIO_->WritePort(port, 0x06);
			// We should get audio sound effect 3 but not 2 
			EXPECT_EQ(0x04, value);
			// Try to play audio sound effect 3 (note this does not include 2)
			value = i8080ArcadeIO_->WritePort(port, 0x04);
			// The audio sound effect does not repeat
			EXPECT_EQ(0x00, value);
			// Try to play audio sound effects 2 and 3
			value = i8080ArcadeIO_->WritePort(port, 0x06);
			// We should get the audio sound effect 2 but not 3 
			EXPECT_EQ(0x02, value);
			// Don't play any audio sound effects
			value = i8080ArcadeIO_->WritePort(port, 0x00);
			EXPECT_EQ(0x00, value);
		};

		writePort(3);
		writePort(5);

		// Try to play audio sound effect 1 and 2 on port 3
		auto value = i8080ArcadeIO_->WritePort(3, 0x03);
		// We should get audio sound effects 1 and 2
		EXPECT_EQ(0x03, value);
		// Try to play the audio sound effects 1 and 2 again
		value = i8080ArcadeIO_->WritePort(3, 0x03);
		// We should get audio sound effect 1 again but not 2
		EXPECT_EQ(0x01, value);
		// End the repeating effect (it should play out one more time)
		value = i8080ArcadeIO_->WritePort(3, 0x00);
		// We should get audio sound effect 1 again
		EXPECT_EQ(0x01, value);
		// Don't play any audio sound effects
		value = i8080ArcadeIO_->WritePort(3, 0x00);
		// We should get nothing back
		EXPECT_EQ(0x00, value);
	}
#endif

} // namespace meen_hw::tests

int main(int argc, char** argv)
{
	std::cout << "Running main() from MeenHwTest.cpp" << std::endl;
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
