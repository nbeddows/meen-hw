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

#include <assert.h>
#include <bitset>
#include <charconv>
#include <cstring>
#include <nlohmann/json.hpp>

#include "meen_hw/i8080_arcade/MH_I8080ArcadeIO.h"

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

	void MH_I8080ArcadeIO::BlitVRAM(uint8_t* dst, uint8_t rowBytes, uint8_t* src)
	{
		constexpr int vramWidth = 256;
		constexpr int vramSize = 7168;

		auto decompressVram = [src, dst, rb = rowBytes, colour = colour_](uint8_t* nextCol, bool cocktail)
		{
			auto vramStart = src;
			auto vramEnd = vramStart + vramSize;
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
				//If we are not at the end, move to the next row, otherwise move to the next column.
				cocktail == true || ptr - rb < dst ? ptr = ++nextCol : ptr -= rb;
			}
		};

		switch (blitMode_)
		{
			case BlitFlags::Upright:
			{
				static constexpr int srcWidth = vramWidth;
				static constexpr int srcWidthMinus1 = vramWidth - 1;
				// Need to skip an additional 7 rows once the vertical sampling is complete.
				static constexpr int srcRowSkip = srcWidth * 7;

				auto begin = src;
				auto end = begin + vramSize;
				auto start = dst + rowBytes * (height_ - 1);
				auto ptr = start;

				while (begin < end)
				{
					for (int i = 0; i < 8; i++)
					{
						uint8_t byte = 0;

						// transpose the compressed pixels (sample from 8 pixels vertically)
						for (int j = 0; j < 8; j++)
						{
							byte |= (((begin[j * srcWidth] >> i) & 0x01) << j);
						}

						*ptr = byte;

						// Move to the previous row, else the next column
						ptr - rowBytes >= dst ? ptr -= rowBytes : ptr = ++start;
					}

					begin++;

					// Since we sample 8 vertical pixels we need to skip another 7 rows when we get to the end of the current row.
					// TODO: mem pool frames need to be 32 bit aligned, then we don't have to subtract src, ie; just for (begin & (width_ - 1)) == 0
					begin += (((begin - src & srcWidthMinus1) == 0) * srcRowSkip);
				}
				break;
			}
			case BlitFlags::Native:
			{
				memcpy(dst, src, vramSize);
				break;
			}
			case BlitFlags::Rgb332:
			{
				decompressVram(dst, true);
				break;
			}
			case BlitFlags::Upright8bpp:
			{
				decompressVram(dst + rowBytes * (height_ - 1), false);
				break;
			}
			default:
			{
				throw std::runtime_error("Invalid blit mode");
			}
		}
	}

	void MH_I8080ArcadeIO::SetOptions(const char* jsonOptions)
	{
		auto options = nlohmann::json::parse(jsonOptions);

		for (const auto& [key, val] : options.items())
		{
			if (key == "bpp")
			{
				switch (val.get<uint8_t>())
				{
					case 1:
					{
						// native bpp, don't set the 8bpp flag
						// TODO: need to turn off BlitFlags::Rgb332
						break;
					}
					case 8:
					{
						blitMode_ |= BlitFlags::Rgb332;
						width_ = 256;
						break;
					}
					default:
					{
						throw std::invalid_argument("Invalid configuration: bpp");
					}
				}
			}
			else if (key == "colour")
			{
				auto colour = val.get<std::string_view>();
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
						throw std::invalid_argument("Invalid configuration: colour");
					}
				}
				else if (*ptr != '\0')
				{
					// we parsed something but there is still left over text
					throw std::invalid_argument("Invalid configuration: colour");
				}
			}
			else if(key == "orientation")
			{
				auto orientation = val.get<std::string>();

				if (orientation == "upright")
				{
					blitMode_ |= BlitFlags::Upright;

					if (blitMode_ & BlitFlags::Rgb332)
					{
						width_ = 224;
					}
					else
					{
						width_ = 28;
					}

					height_ = 256;
				}
				else if (orientation != "cocktail")
				{
					throw std::invalid_argument("Invalid configuration: orientation");
				}
			}
			else
			{
				throw std::invalid_argument("Invalid configuration option");
			}
		}
	}

	int MH_I8080ArcadeIO::GetVRAMWidth() const
	{
		return width_;
	}

	int MH_I8080ArcadeIO::GetVRAMHeight() const
	{
		return height_;
	}
} // namespace meen_hw::i8080_arcade