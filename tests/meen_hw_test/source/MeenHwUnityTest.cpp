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

#include <bit>
#ifdef ENABLE_MH_RP2040
#include <pico/stdlib.h>
#endif
#include <unity/unity.h>
#include <vector>

#include "meen_hw/MH_Factory.h"
#include "meen_hw/MH_ResourcePool.h"

void setUp(){}
void tearDown(){}

namespace meen_hw::tests
{
	static std::unique_ptr<MH_II8080ArcadeIO> i8080ArcadeIO;
	static int resourceCounter = 0;

	static void suiteSetUp()
    {
		i8080ArcadeIO = MakeI8080ArcadeIO();

#ifdef ENABLE_MH_I8080ARCADE
		TEST_ASSERT_NOT_NULL(i8080ArcadeIO);
#else
		TEST_ASSERT_NULL(i8080ArcadeIO);
#endif
	}

	static int suiteTearDown(int numFailures)
	{
		return numFailures;
	}

	static void test_Version()
	{
		TEST_ASSERT_NOT_NULL(Version());
	}

	static void test_ResourcePool()
	{
		resourceCounter = 0;

		struct ResourceDeleter
		{
			void operator()(int*)
			{
				resourceCounter++;
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
				TEST_ASSERT_NOT_NULL(r1);
				auto r2 = pool.GetResource();
				TEST_ASSERT_NOT_NULL(r2);
				auto r3 = pool.GetResource();
				TEST_ASSERT_NOT_NULL(r3);

				// Pool should now be empty
				auto r4 = pool.GetResource();
				TEST_ASSERT_NULL(r4);
			}

			// All resources should be returned to the pool now
			{
				auto r1 = pool.GetResource();
				TEST_ASSERT_NOT_NULL(r1);
				auto r2 = pool.GetResource();
				TEST_ASSERT_NOT_NULL(r2);
				auto r3 = pool.GetResource();
				TEST_ASSERT_NOT_NULL(r3);

				// Pool should now be empty
				auto r4 = pool.GetResource();
				TEST_ASSERT_NULL(r4);
			}

			// All resources should be available
			outlivePool = pool.GetResource();
			TEST_ASSERT_NOT_NULL(outlivePool);
			*outlivePool = 42;
		}

		// The pool is dead, resource should be valid
		TEST_ASSERT_EQUAL(42, *outlivePool);
		// The pool is dead, the resource should be destroyed
		outlivePool = nullptr;

		//Check to total number of deletions
		TEST_ASSERT_EQUAL(3, resourceCounter);
	}

#ifdef ENABLE_MH_I8080ARCADE
	void test_ReadPort0()
	{
		auto value = i8080ArcadeIO->ReadPort(0);
		//https://www.reddit.com/r/EmuDev/comments/mvpt4w/space_invaders_part_ii_deluxe_emulator/
		TEST_ASSERT_EQUAL_UINT8(0x40, value);
	}

	void test_WriteAudioPorts()
	{
		/*
			Writing to ports 3 and 5 will write to the audio hardware.

			Each bit of data written to ports 3 and 5 represents a
			sound effect. Identical bits of consecutive port writes
			are ignored (no audio sound effect shall be repeated).

			The only exception to this is bit 0 on port 3 which is
			repeated.
		*/

		auto writePort = [](int port)
		{
			// Try to play audio sound effect 2
			auto value = i8080ArcadeIO->WritePort(port, 0x02);
			// We should get audio sound effect 2
			TEST_ASSERT_EQUAL_UINT8(0x02, value);
			// Try to play the audio sound effect 2 again
			value = i8080ArcadeIO->WritePort(port, 0x02);
			// The audio sound effect does not repeat
			TEST_ASSERT_EQUAL_UINT8(0x00, value);
			// Try to play audio sound effects 2 and 3
			value = i8080ArcadeIO->WritePort(port, 0x06);
			// We should get audio sound effect 3 but not 2
			TEST_ASSERT_EQUAL_UINT8(0x04, value);
			// Try to play audio sound effect 3 (note this does not include 2)
			value = i8080ArcadeIO->WritePort(port, 0x04);
			// The audio sound effect does not repeat
			TEST_ASSERT_EQUAL_UINT8(0x00, value);
			// Try to play audio sound effects 2 and 3
			value = i8080ArcadeIO->WritePort(port, 0x06);
			// We should get the audio sound effect 2 but not 3
			TEST_ASSERT_EQUAL_UINT8(0x02, value);
			// Don't play any audio sound effects
			value = i8080ArcadeIO->WritePort(port, 0x00);
			TEST_ASSERT_EQUAL_UINT8(0x00, value);
		};

		writePort(3);
		writePort(5);

		// Try to play audio sound effect 1 and 2 on port 3
		auto value = i8080ArcadeIO->WritePort(3, 0x03);
		// We should get audio sound effects 1 and 2
		TEST_ASSERT_EQUAL_UINT8(0x03, value);
		// Try to play the audio sound effects 1 and 2 again
		value = i8080ArcadeIO->WritePort(3, 0x03);
		// We should get audio sound effect 1 again but not 2
		TEST_ASSERT_EQUAL_UINT8(0x01, value);
		// End the repeating effect (it should play out one more time)
		value = i8080ArcadeIO->WritePort(3, 0x00);
		// We should get audio sound effect 1 again
		TEST_ASSERT_EQUAL_UINT8(0x01, value);
		// Don't play any audio sound effects
		value = i8080ArcadeIO->WritePort(3, 0x00);
		// We should get nothing back
		TEST_ASSERT_EQUAL_UINT8(0x00, value);
	}

	// This tests port 2 and 4 writes and port 3 reads (shifting register)
	void test_ShiftRegister()
	{
		// Write bytes 0xFF and 0x00 into port 4 to store the 16bit value 0x00FF

		auto result = i8080ArcadeIO->WritePort(4, 0xFF);
		// Writing to port 2 should always return 0.
		TEST_ASSERT_EQUAL_UINT8(0x00, result);
		result = i8080ArcadeIO->WritePort(4, 0x00);
		// Writing to port 2 should always return 0.
		TEST_ASSERT_EQUAL_UINT8(0x00, result);

		// Bit shift from the high byte (0x00) to the low byte (0xFF)
		for (int i = 0; i < 8; i++)
		{
			result = i8080ArcadeIO->WritePort(2, i);
			// Writing to port 2 should always return 0.
			TEST_ASSERT_EQUAL_UINT8(0x00, result);
			// Shifting down the 16 bits should give 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
			result = i8080ArcadeIO->ReadPort(3);
			TEST_ASSERT_EQUAL_UINT8((1 << i) - 1, result);
		}
	}

	// Cycle through interrupts 1 and 2 when the clock changes
	// Return no interrupt when the clock does not change
	void test_GenerateInterrupt()
	{
		auto isr = i8080ArcadeIO->GenerateInterrupt(8333333, 16666);
		TEST_ASSERT_EQUAL_UINT8(1, isr);

		isr = i8080ArcadeIO->GenerateInterrupt(16666666, 33333);
		TEST_ASSERT_EQUAL_UINT8(2, isr);

		isr = i8080ArcadeIO->GenerateInterrupt(25000000, 50000);
		TEST_ASSERT_EQUAL_UINT8(1, isr);

		isr = i8080ArcadeIO->GenerateInterrupt(33333333, 66666);
		TEST_ASSERT_EQUAL_UINT8(2, isr);

		isr = i8080ArcadeIO->GenerateInterrupt(33333333, 74999);
		TEST_ASSERT_EQUAL_UINT8(0, isr);

		isr = i8080ArcadeIO->GenerateInterrupt(50000000, 100000);
		TEST_ASSERT_EQUAL_UINT8(1, isr);

		isr = i8080ArcadeIO->GenerateInterrupt(50000000, 183333);
		TEST_ASSERT_EQUAL_UINT8(0, isr);
	}

	void test_SetOptions()
	{
		auto checkErrc = [](const std::error_code& ec, bool success, const char* expectedMsg)
		{
			if (success == true) TEST_ASSERT_FALSE(ec); else TEST_ASSERT_TRUE(ec);
			TEST_ASSERT_EQUAL_STRING(expectedMsg, ec.message().c_str());
		};

		checkErrc(i8080ArcadeIO->SetOptions("{\"bpp\":2}"), false, "The bpp configuration option is invalid");
		checkErrc(i8080ArcadeIO->SetOptions("{\"colour\":\"black\" }"), false, "The colour configuration option is invalid");
		checkErrc(i8080ArcadeIO->SetOptions("{\"orientation\":\"up\"}"), false, "The orientation configuration parameter is invalid");
		checkErrc(i8080ArcadeIO->SetOptions("syntax-error"), false, "A json parse error occurred while processing the configuration file");
		checkErrc(i8080ArcadeIO->SetOptions("{\"bpp\":8,\"colour\":\"random\",\"orientation\":\"cocktail\"}"), true, "Success");
	}

	void test_GetVRAMDimensions()
	{
		TEST_ASSERT_FALSE(i8080ArcadeIO->SetOptions("{\"orientation\":\"cocktail\"}"));
		TEST_ASSERT_EQUAL_UINT16(256, i8080ArcadeIO->GetVRAMWidth());
		TEST_ASSERT_EQUAL_UINT16(224, i8080ArcadeIO->GetVRAMHeight());

		TEST_ASSERT_FALSE(i8080ArcadeIO->SetOptions("{\"orientation\":\"upright\"}"));
		TEST_ASSERT_EQUAL_UINT16(224, i8080ArcadeIO->GetVRAMWidth());
		TEST_ASSERT_EQUAL_UINT16(256, i8080ArcadeIO->GetVRAMHeight());
	}

	void test_BlitVRAM()
	{
		uint8_t srcVRAM[7168]; // 7168 - width * height @ 1bpp
		uint8_t expectedVRAM[57344]; // 57344 - width * height @ 8pp

		auto checkVRAM = [](std::span<uint8_t> VRAMToBlit, std::span<uint8_t> expectedVRAM, int expectedRowBytes, int padding, int compressed, const char* options)
		{
			// Blit to native format
			TEST_ASSERT_FALSE(i8080ArcadeIO->SetOptions(options));
			// To get the row bytes we need to shift down 3 (divide by 8) if we are compressed, 0 if we are uncompressed.
			auto actualRowBytes = (i8080ArcadeIO->GetVRAMWidth() >> compressed) + padding; // add some padding so the row bytes differs from the expected
			auto dstVRAM = std::vector<uint8_t>(actualRowBytes * i8080ArcadeIO->GetVRAMHeight());
			i8080ArcadeIO->BlitVRAM(std::span(dstVRAM), actualRowBytes, VRAMToBlit);

			auto expected = expectedVRAM.data();
			auto actual = dstVRAM.data();
			while(expected < expectedVRAM.data() + expectedVRAM.size())
			{
				TEST_ASSERT_EQUAL_MEMORY(expected, actual, expectedRowBytes);
				actual += actualRowBytes;
				expected += expectedRowBytes;
			}
		};

		// We need to output white (0xFF) in the uncompressed case
		TEST_ASSERT_FALSE(i8080ArcadeIO->SetOptions("{\"colour\":\"white\"}"));

		// Set the src vram to be blitted to be an alternating black and white scanline pattern
		// This will act as the expectedVRAM for 1bpp native orientation test
		for (auto data = srcVRAM; data < srcVRAM + 7168; data += 64)
		{
			// 32 - compressed row bytes
			std::fill_n(data, 32, 0x00);
			std::fill_n(data + 32, 32, 0xFF);
		}

		// Native blit without padding
		checkVRAM(std::span(srcVRAM), std::span(srcVRAM), 32, 0, 3, "{\"bpp\":1,\"orientation\":\"cocktail\"}");
		// Native blit with padding
		checkVRAM(std::span(srcVRAM), std::span(srcVRAM), 32, 2, 3, "{\"bpp\":1,\"orientation\":\"cocktail\"}");

		// Vertical black and white bars
		std::fill(expectedVRAM, expectedVRAM + 7168, 0xAA);

		// Native bpp blit with upright orientation without padding
		checkVRAM(std::span(srcVRAM), std::span(expectedVRAM, 7168), 28, 0, 3, "{\"bpp\":1,\"orientation\":\"upright\"}");
		// Native bpp blit with upright orientation with padding
		checkVRAM(std::span(srcVRAM), std::span(expectedVRAM, 7168), 28, 2, 3, "{\"bpp\":1,\"orientation\":\"upright\"}");

		for (auto data = expectedVRAM; data < expectedVRAM + 57344; data += 512)
		{
			// 256 - uncompressed row bytes
			std::fill_n(data, 256, 0x00);
			std::fill_n(data + 256, 256, 0xFF);
		}
		
		// Native orientation 8pp blit without padding
		checkVRAM(std::span(srcVRAM), std::span(expectedVRAM), 256, 0, 0, "{\"bpp\":8,\"orientation\":\"cocktail\"}");
		// Native orientation 8pp blit with padding
		checkVRAM(std::span(srcVRAM), std::span(expectedVRAM), 256, 16, 0, "{\"bpp\":8,\"orientation\":\"cocktail\"}");

		auto data = expectedVRAM;
		std::fill_n(std::bit_cast<uint16_t*>(data), 28672, 0xFF00);

		// 8 bpp blit with upright orientation without padding
		checkVRAM(std::span(srcVRAM), std::span(expectedVRAM), 224, 0, 0, "{\"bpp\":8,\"orientation\":\"upright\"}");
		// 8 bpp blit with upright orientation with padding
		checkVRAM(std::span(srcVRAM), std::span(expectedVRAM), 224, 16, 0, "{\"bpp\":8,\"orientation\":\"upright\"}");
	}
#endif
} // namespace meen_hw::tests

int main(void)
{
	int err = 0;
#ifdef ENABLE_MH_RP2040
	stdio_init_all();

	while(true)
#endif
	{
		meen_hw::tests::suiteSetUp();
		UNITY_BEGIN();
		RUN_TEST(meen_hw::tests::test_Version);
		RUN_TEST(meen_hw::tests::test_ResourcePool);
#ifdef ENABLE_MH_I8080ARCADE
		RUN_TEST(meen_hw::tests::test_ReadPort0);
		RUN_TEST(meen_hw::tests::test_WriteAudioPorts);
		RUN_TEST(meen_hw::tests::test_ShiftRegister);
		RUN_TEST(meen_hw::tests::test_GenerateInterrupt);
		RUN_TEST(meen_hw::tests::test_SetOptions);
		RUN_TEST(meen_hw::tests::test_GetVRAMDimensions);
		RUN_TEST(meen_hw::tests::test_BlitVRAM);
#endif
		err = meen_hw::tests::suiteTearDown(UNITY_END());

#ifdef ENABLE_MH_RP2040
		sleep_ms(1000);
#endif
	}

	return err;
}