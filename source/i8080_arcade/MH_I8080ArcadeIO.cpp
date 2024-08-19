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

#include <algorithm>
#include <assert.h>
#include <bitset>
#include <charconv>
#include <ctime>
#include <cstring>
#ifdef ENABLE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#else
#include <ArduinoJson.h>
#endif

#include "meen_hw/i8080_arcade/MH_I8080ArcadeIO.h"
#include "meen_hw/MH_Error.h"

namespace meen_hw::i8080_arcade
{
	uint8_t MH_I8080ArcadeIO::ReadPort(uint16_t port)
	{
		if (port == 3)
		{
			return (shiftData_ >> (8 - shiftAmount_)) & 0xFF;
		}
		else if (port == 0)
		{
			// https://www.reddit.com/r/EmuDev/comments/mvpt4w/space_invaders_part_ii_deluxe_emulator/
			return 0x40;
		}

		return 0;
	}

	uint8_t MH_I8080ArcadeIO::WritePort(uint16_t port, uint8_t data)
	{
		std::bitset<8> audio = 0;

		if (port == 2)
		{
			//Writing to port 2 (bits 0, 1, 2) sets the offset for the 8 bit result
			shiftAmount_ = data & 0x07; //we are only interested in the first 3 bits
		}
		else if (port == 3)
		{
			// Ufo audio repeats, so we'll handle that as a separate case
			audio[0] = (data & 1) | (port3Byte_ & 1);

			for (int i = 1; i < 8; i++)
			{
				audio[i] = (data & (1 << i)) > (port3Byte_ & (1 << i));
			}

			port3Byte_ = data;
		}
		else if (port == 4)
		{
			shiftData_ = (shiftData_ >> 8) | (static_cast<uint16_t>(data) << 8);
		}
		else if (port == 5)
		{
			for (int i = 0; i < 8; i++)
			{
				audio[i] = (data & (1 << i)) > (port5Byte_ & (1 << i));
			}

			port5Byte_ = data;
		}
		else if (port == 6)
		{
			//printf("Watch-dog: %d\n", data);
		}
		else
		{
			// Force a failure
			assert(port >= 2 && port <= 6);
			//printf("Unknown device: %d\n", data);
		}

		return audio.to_ulong();
	}

	uint8_t MH_I8080ArcadeIO::GenerateInterrupt(uint64_t currTime, uint64_t cycles)
	{
		uint8_t isr = 0;

		if (currTime != lastTime_)
		{
			isr = nextInterrupt_;

			//Check last interrupt, if it is One then we are at the start of the vBlank
			if (isr == 1)
			{
				//Signal vBlank interrupt.
				nextInterrupt_ = 2;
			}
			else
			{
				//Signal that the 'crt beam' is about half was down the screen.
				nextInterrupt_ = 1;
			}

			lastTime_ = currTime;
		}

		return isr;
	}

	void MH_I8080ArcadeIO::BlitVRAM(std::span<uint8_t> dst, int rowBytes, std::span<uint8_t> src)
	{
		assert(dst.size() >= src.size());

		auto decompressVram = [src, dst, rb = rowBytes, colour = colour_](uint8_t* nextCol, bool cocktail)
		{
			auto vramStart = src.begin();
			auto vramEnd = src.end();
			int8_t shift = 0;
			auto ptr = nextCol;

			while (vramStart < vramEnd)
			{
				//Decompress the vram from 1bpp to 8bpp.
				*ptr = ((*vramStart >> shift) & 0x01) * colour;
				//Cycle the shift value between 0-7.
				shift = ++shift & 0x07;
				//Move to the next vram byte if we have done a full cycle.
				vramStart += shift == 0;

				if(cocktail == true)
				{
					if (++ptr - nextCol >= 256)
					{
						nextCol += rb;
						ptr = nextCol;
					}
				}
				else
				{
					//If we are not at the first row, move to the previous row, otherwise move to the next column.
					ptr - rb < dst.data() ? ptr = ++nextCol : ptr -= rb;
				}
			}
		};

		switch (blitMode_)
		{
			case BlitFlags::Upright:
			{
				static constexpr int srcWidth = 32;
				static constexpr int srcWidthMinus1 = srcWidth - 1;
				// Need to skip an additional 7 rows once the vertical sampling is complete.
				static constexpr int srcRowSkip = srcWidth * 7;

				auto begin = src.begin();
				auto end = src.end();
				auto start = dst.data() + rowBytes * (256 - 1);
				auto ptr = start;

				while (begin < end)
				{
					for (int i = 0; i < 8; i++)
					{
						uint8_t byte = 0;

						// Transpose the compressed pixels (sample from 8 pixels vertically)
						for (int j = 0; j < 8; j++)
						{
							byte |= (((begin[j * srcWidth] >> i) & 0x01) << j);
						}

						*ptr = byte;

						// Move to the previous row, else the next column
						ptr - rowBytes >= dst.data() ? ptr -= rowBytes : ptr = ++start;
					}

					begin++;

					// Since we sample 8 vertical pixels we need to skip another 7 rows when we get to the end of the current row.
					// TODO: mem pool frames need to be 32 bit aligned, then we don't have to subtract src, ie; just for (begin & (width_ - 1)) == 0
					begin += (((begin - src.begin() & srcWidthMinus1) == 0) * srcRowSkip);
				}
				break;
			}
			case BlitFlags::Native:
			{
				if(rowBytes == 32)
				{
					std::copy(src.begin(), src.end(), dst.begin());
				}
				else
				{
					auto d = dst.begin();;
					auto s = src.begin();

					// copy out each scanline
					while(s < src.end())
					{
						std::copy_n(s, 32, d);
						d += rowBytes;
						s += 32;
					}
				}
				break;
			}
			case BlitFlags::Rgb332:
			{
				decompressVram(dst.data(), true);
				break;
			}
			case BlitFlags::Upright8bpp:
			{
				decompressVram(dst.data() + rowBytes * (256 - 1), false);
				break;
			}
			default:
			{
				// todo: log invalid blit mode
				assert(blitMode_ == BltFlags::Upright || blitMode_ == BltFlags::Native || blitMode_ == BlitFlags::Rgb332 || blitMode_ == BlitFlags::Upright8bpp);
			}
		}
	}

	std::error_code MH_I8080ArcadeIO::SetOptions(const char* jsonOptions)
	{
		auto err = make_error_code(errc::no_error);
#ifdef ENABLE_NLOHMANN_JSON
		auto options = nlohmann::json::parse(jsonOptions, nullptr, false);

		if(options.is_discarded() == true)
		{
			return make_error_code(errc::json_parse);
		}

		for (const auto& [key, val] : options.items())
		{
#else
		JsonDocument options;

		auto e = deserializeJson(options, jsonOptions);

		if(e)
		{
			return make_error_code(errc::json_parse);
		}

		for(auto kv : options.as<JsonObject>())
		{
			auto key = kv.key();
#endif
			if (key == "bpp")
			{
#ifdef ENABLE_NLOHMANN_JSON
				auto value = val.get<uint8_t>();
#else
				auto value = kv.value().as<uint8_t>();
#endif
				switch (value)
				{
					case 1:
					{
						blitMode_ &= ~BlitFlags::Rgb332;
						break;
					}
					case 8:
					{
						blitMode_ |= BlitFlags::Rgb332;
						break;
					}
					default:
					{
						err = meen_hw::make_error_code(errc::bpp);
						break;
					}
				}
			}
			else if (key == "colour")
			{
#ifdef ENABLE_NLOHMANN_JSON
				auto colour = val.get<std::string_view>();
#else
				auto colour = kv.value().as<std::string_view>();
#endif
				auto [ptr, errc] = std::from_chars(colour.data(), colour.data() + colour.size(), colour_, 16);

				if (errc != std::errc())
				{
					if (colour == "red")
					{
						colour_ = 0x80;
					}
					else if (colour == "green")
					{
						colour_ = 0x14;
					}
					else if (colour == "blue")
					{
						colour_ = 0x07;
					}
					else if (colour == "white")
					{
						colour_ = 0xFF;
					}
					else if (colour == "random")
					{
						srand(time(nullptr));
						colour_ = rand() % 255;
					}
					else
					{
						err = meen_hw::make_error_code(errc::colour);
					}
				}
				else if (*ptr != '\0')
				{
					// we parsed something but there is still left over text
					err = meen_hw::make_error_code(errc::colour);
				}
			}
			else if(key == "orientation")
			{
#ifdef ENABLE_NLOHMANN_JSON
				auto orientation = val.get<std::string>();
#else
				auto orientation = kv.value().as<std::string>();
#endif

				if (orientation == "upright")
				{
					blitMode_ |= BlitFlags::Upright;
				}
				else if (orientation == "cocktail")
				{
					blitMode_ &= ~BlitFlags::Upright;
				}
				else
				{
					err = meen_hw::make_error_code(errc::orientation);
				}
			}
			else
			{
				//todo: log unknown option
			}
		}

		return err;
	}

	int MH_I8080ArcadeIO::GetVRAMWidth() const
	{
		return blitMode_ & BlitFlags::Upright ? 224 : 256;
	}

	int MH_I8080ArcadeIO::GetVRAMHeight() const
	{
		return blitMode_ & BlitFlags::Upright ? 256 : 224;
	}
} // namespace meen_hw::i8080_arcade