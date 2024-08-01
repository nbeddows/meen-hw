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

#include <bit>
#include <vector>

#include "meen_hw/MH_Factory.h"
#include "meen_hw/MH_ResourcePool.h"

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

	TEST_F(MeenHwTest, ResourcePool)
	{		
		static int counter = 0;

		struct ResourceDeleter
		{
			void operator()(int*)
			{
				counter++;
			};
		};

		meen_hw::MH_ResourcePool<int, ResourceDeleter>::ResourcePtr outlivePool;

		{
			auto pool = meen_hw::MH_ResourcePool<int, ResourceDeleter>();

			// Add 3 resources
			int r1 = 0;
			int r2 = 0;
			int r3 = 0;

			pool.AddResource(&r1);
			pool.AddResource(&r2);
			pool.AddResource(&r3);

			{
				auto r1 = pool.GetResource();
				EXPECT_NE(nullptr, r1);
				auto r2 = pool.GetResource();
				EXPECT_NE(nullptr, r2);
				auto r3 = pool.GetResource();
				EXPECT_NE(nullptr, r3);

				// Pool should now be empty
				auto r4 = pool.GetResource();
				EXPECT_EQ(nullptr, r4);
			}

			// All resources should be returned to the pool now
			{
				auto r1 = pool.GetResource();
				EXPECT_NE(nullptr, r1);
				auto r2 = pool.GetResource();
				EXPECT_NE(nullptr, r2);
				auto r3 = pool.GetResource();
				EXPECT_NE(nullptr, r3);

				// Pool should now be empty
				auto r4 = pool.GetResource();
				EXPECT_EQ(nullptr, r4);
			}

			// All resources should be available
			outlivePool = pool.GetResource();
			ASSERT_NE(nullptr, outlivePool);
			*outlivePool = 42;
		}
		
		// The pool is dead, resource should be valid
		EXPECT_EQ(42, *outlivePool);
		// The pool is dead, the resource should be destroyed
		outlivePool = nullptr;

		//Check to total number of deletions
		EXPECT_EQ(3, counter);
	}

#ifdef ENABLE_MH_I8080ARCADE
	TEST_F(MeenHwTest, ReadPort0)
	{
		auto value = i8080ArcadeIO_->ReadPort(0);
		//https://www.reddit.com/r/EmuDev/comments/mvpt4w/space_invaders_part_ii_deluxe_emulator/
		EXPECT_EQ(0x40, value);
	}

	TEST_F(MeenHwTest, WriteAudioPorts)
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

	// This tests port 2 and 4 writes and port 3 reads (shifting register)
	TEST_F(MeenHwTest, ShiftRegister)
	{
		// Write bytes 0xFF and 0x00 into port 4 to store the 16bit value 0x00FF

		auto result = i8080ArcadeIO_->WritePort(4, 0xFF);
		// Writing to port 2 should always return 0.
		EXPECT_EQ(0x00, result);
		result = i8080ArcadeIO_->WritePort(4, 0x00);
		// Writing to port 2 should always return 0.
		EXPECT_EQ(0x00, result);

		// Bit shift from the high byte (0x00) to the low byte (0xFF)
		for (int i = 0; i < 8; i++)
		{
			result = i8080ArcadeIO_->WritePort(2, i);
			// Writing to port 2 should always return 0.
			EXPECT_EQ(0x00, result);
			// Shifting down the 16 bits should give 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
			result = i8080ArcadeIO_->ReadPort(3);
			EXPECT_EQ((1 << i) - 1, result);
		}
	}

	// Cycle through interrupts 1 and 2 when the clock changes
	// Return no interrupt when the clock does not change
	TEST_F(MeenHwTest, GenerateInterrupt)
	{
		auto isr = i8080ArcadeIO_->GenerateInterrupt(8333333, 16666);
		EXPECT_EQ(1, isr);

		isr = i8080ArcadeIO_->GenerateInterrupt(16666666, 33333);
		EXPECT_EQ(2, isr);

		isr = i8080ArcadeIO_->GenerateInterrupt(25000000, 50000);
		EXPECT_EQ(1, isr);

		isr = i8080ArcadeIO_->GenerateInterrupt(33333333, 66666);
		EXPECT_EQ(2, isr);

		isr = i8080ArcadeIO_->GenerateInterrupt(33333333, 74999);
		EXPECT_EQ(0, isr);

		isr = i8080ArcadeIO_->GenerateInterrupt(50000000, 100000);
		EXPECT_EQ(1, isr);

		isr = i8080ArcadeIO_->GenerateInterrupt(50000000, 183333);
		EXPECT_EQ(0, isr);
	}

	TEST_F(MeenHwTest, SetOptions)
	{
		auto checkErrc = [](const std::error_code& ec, bool success, const char* expectedMsg)
		{
			if (success == true) EXPECT_FALSE(ec); else EXPECT_TRUE(ec);
			EXPECT_EQ(expectedMsg, ec.message());
		};
		
		EXPECT_NO_THROW
		(
			checkErrc(i8080ArcadeIO_->SetOptions("{\"bpp\":2}"), false, "The bpp configuration option is invalid");
			checkErrc(i8080ArcadeIO_->SetOptions("{\"colour\":\"black\" }"), false, "The colour configuration option is invalid");
			checkErrc(i8080ArcadeIO_->SetOptions("{\"orientation\":\"up\"}"), false, "The orientation configuration parameter is invalid");
			checkErrc(i8080ArcadeIO_->SetOptions("syntax-error"), false, "A json parse error occurred while processing the configuration file");
			checkErrc(i8080ArcadeIO_->SetOptions("{\"bpp\":8,\"colour\":\"random\",\"orientation\":\"cocktail\"}"), true, "Success");
		);
	}

	TEST_F(MeenHwTest, GetVRAMDimensions)
	{
		EXPECT_NO_THROW(i8080ArcadeIO_->SetOptions("{\"orientation\":\"cocktail\"}"));
		EXPECT_EQ(256, i8080ArcadeIO_->GetVRAMWidth());
		EXPECT_EQ(224, i8080ArcadeIO_->GetVRAMHeight());

		EXPECT_NO_THROW(i8080ArcadeIO_->SetOptions("{\"orientation\":\"upright\"}"););
		EXPECT_EQ(224, i8080ArcadeIO_->GetVRAMWidth());
		EXPECT_EQ(256, i8080ArcadeIO_->GetVRAMHeight());
	}

	TEST_F(MeenHwTest, BlitVRAM)
	{
		uint8_t srcVRAM[7168]; // 7168 - width * height @ 1bpp
		uint8_t expectedVRAMUpright1[7168]; // 57344 - width * height @ 1pp
		uint8_t expectedVRAMCocktail8[57344]; // 57344 - width * height @ 8pp
		uint8_t expectedVRAMUpright8[57344]; // 57344 - width * height @ 8pp
		
		// We need to output white (0xFF) in the uncompressed case
		i8080ArcadeIO_->SetOptions("{\"colour\":\"white\"}");

		// Set the src vram to be blitted to be an alternating black and white scanline pattern
		// This will act as the expectedVRAM for 1bpp native orientation test
		for (auto data = srcVRAM; data < srcVRAM + 7168; data += 64)
		{
			// 32 - compressed row bytes
			std::fill_n(data, 32, 0x00);
			std::fill_n(data + 32, 32, 0xFF);
		}

		// Vertical black and white bars
		std::fill(expectedVRAMUpright1, expectedVRAMUpright1 + 7168, 0xAA);

		for (auto data = expectedVRAMCocktail8; data < expectedVRAMCocktail8 + 57344; data += 512)
		{
			// 256 - uncompressed row bytes
			std::fill_n(data, 256, 0x00);
			std::fill_n(data + 256, 256, 0xFF);
		}

		auto data = expectedVRAMUpright8;
		std::fill_n(std::bit_cast<uint16_t*>(data), 28672, 0xFF00);

		auto checkVRAM = [this](std::span<uint8_t> VRAMToBlit, std::span<uint8_t> expectedVRAM, int expectedRowBytes, int padding, int compressed, const char* options)
		{
			// Blit to native format
			EXPECT_NO_THROW(i8080ArcadeIO_->SetOptions(options));
			// To get the row bytes we need to shift down 3 (divide by 8) if we are compressed, 0 if we are uncompressed.
			auto actualRowBytes = (i8080ArcadeIO_->GetVRAMWidth() >> compressed) + padding; // add some padding so the row bytes differs from the expected
			auto dstVRAM = std::vector<uint8_t>(actualRowBytes * i8080ArcadeIO_->GetVRAMHeight());
			EXPECT_NO_THROW(i8080ArcadeIO_->BlitVRAM(std::span(dstVRAM), actualRowBytes, VRAMToBlit));

			auto expected = expectedVRAM.data();
			auto actual = dstVRAM.data();
			while(expected < expectedVRAM.data() + expectedVRAM.size())
			{
				EXPECT_EQ(0, memcmp(actual, expected, expectedRowBytes));
				actual += actualRowBytes;
				expected += expectedRowBytes;
			}
		};

		// Native blit without padding
		checkVRAM(std::span(srcVRAM), std::span(srcVRAM), 32, 0, 3, "{\"bpp\":1,\"orientation\":\"cocktail\"}");
		// Native blit with padding
		checkVRAM(std::span(srcVRAM), std::span(srcVRAM), 32, 2, 3, "{\"bpp\":1,\"orientation\":\"cocktail\"}");

		// Native orientation 8pp blit without padding
		checkVRAM(std::span(srcVRAM), std::span(expectedVRAMCocktail8), 256, 0, 0, "{\"bpp\":8,\"orientation\":\"cocktail\"}");
		// Native orientation 8pp blit with padding
		checkVRAM(std::span(srcVRAM), std::span(expectedVRAMCocktail8), 256, 16, 0, "{\"bpp\":8,\"orientation\":\"cocktail\"}");

		// Native bpp blit with upright orientation without padding
		checkVRAM(std::span(srcVRAM), std::span(expectedVRAMUpright1), 28, 0, 3, "{\"bpp\":1,\"orientation\":\"upright\"}");
		// Native bpp blit with upright orientation with padding
		checkVRAM(std::span(srcVRAM), std::span(expectedVRAMUpright1), 28, 2, 3, "{\"bpp\":1,\"orientation\":\"upright\"}");

		// 8 bpp blit with upright orientation without padding
		checkVRAM(std::span(srcVRAM), std::span(expectedVRAMUpright8), 224, 0, 0, "{\"bpp\":8,\"orientation\":\"upright\"}");
		// 8 bpp blit with upright orientation with padding
		checkVRAM(std::span(srcVRAM), std::span(expectedVRAMUpright8), 224, 16, 0, "{\"bpp\":8,\"orientation\":\"upright\"}");
	}
#endif

} // namespace meen_hw::tests

int main(int argc, char** argv)
{
	std::cout << "Running main() from MeenHwTest.cpp" << std::endl;
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
