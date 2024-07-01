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

#ifndef MEEN_HW_MH_I8080ARCADEIO_H
#define MEEN_HW_MH_I8080ARCADEIO_H

#include "meen_hw/MH_II8080ArcadeIO.h"

namespace meen_hw::i8080_arcade
{
	/** i8080 arcade hardware emulation.

		This is an implementation of the MH_II8080ArcadeIO interface.

		@see MH_II8080ArcadeIO
	*/
	class MH_I8080ArcadeIO final : public MH_II8080ArcadeIO
	{
	private:
		/** Render modes

			The different formats that the video ram can be presented for rendering.
		*/
		enum BlitFlags
		{
			Native		= 0 << 0,				/**< Native pixel format (1bpp) and resolution (256 x 224). */
			Rgb332		= 1 << 0,				/**< 8 bits per pixel with native resolution. */
			Upright		= 1 << 1,				/**< Native pixel format with a resolution of 224 x 256. */
			Upright8bpp = Upright | Rgb332		/**< 8 bits per pixel with a resolution of 224 x 256. */
		};

		/** The next interrupt to execute

			nextInterrupt_ holds the next interrupt service routine that will be sent to the cpu.

			The i8080 hardware listens for two interrupts, one and two.
			Interrupt one is issued when the 'CRT beam' is near the center of the screen.
			Interrupt two is issued when the 'CRT beam' is at the end of the screen (VBLANK start).
		*/
		uint8_t nextInterrupt_{ 1 };
		
		/** Last cpu time

			lastTime_ holds the previous cpu time that was passed to `GenerateInterrupt`.

			In order to emulate a 60hz display we need to fire interrupt one and interrupt two
			at 60hz intervals.
		*/
		uint64_t lastTime_{};
		
		/** Dedicated Shift Hardware

			The 8080 instruction set does not include opcodes for shifting.
			An 8-bit pixel image must be shifted into a 16-bit word for the desired bit-position on the screen.
			The i8080 arcade hardware adds a shift register to help with the math.

			shiftIn_, shiftAmount_ and shiftData_ help emulate this.
		*/
		//cppcheck-suppress unusedStructMember
		uint8_t shiftIn_{};
		//cppcheck-suppress unusedStructMember
		uint8_t shiftAmount_{};
		//cppcheck-suppress unusedStructMember
		uint16_t shiftData_{};

		/**	Backup port 3 and port 5 bytes.
		
			These ports are used for writing to an output audio device.
			We only want to write the audio when the required port bit
			changes from off to on, hence we need to backup these bytes
			to make that comparison.
		*/
		//cppcheck-suppress unusedStructMember
		uint8_t port3Byte_{};
		//cppcheck-suppress unusedStructMember
		uint8_t port5Byte_{};

		/** Render mode

			A combination of flags that determine how the video ram
			will be blitted. The blit mode can be set using the "bpp"
			and "orientation" properties in the config.json.

			@remark	The default mode is set to native.

			@see BlitFlags
		*/
		uint8_t blitMode_{ BlitFlags::Native };

		/** Foreground colour
		
			The default colour is white. The value can be set using the "colour"
			property in the config.json.

			@remark	The background colour is always black.
			@remark	This value is ignored when blitMode_ does not have BlitFlags::Rgb332 set.
		
			@see blitMode_
			@see BlitFlags
		*/
		uint8_t colour_{ 0xFF };

		/** Output video width
		
			The width of the video is dictated by the blitMode in use.

			@remark The width is in pixels.

			@see blitMode_
			@see BlitFlags
		*/
		int width_{ 256 };
		
		/** Output video height
		
			The height of the video is dictated by the blitMode in use.

			@remark The height is in pixels.

			@see blitMode_
			@see BlitFlags
		*/
		int height_{ 224 };

	public:
		/** Read from the specified port

			@see MH_II8080ArcadeIO::ReadPort
		*/
		uint8_t ReadPort(uint16_t port) final;

		/** Write to the specified port

			@see MH_II8080ArcadeIO::WritePort
		*/
		uint8_t WritePort(uint16_t port, uint8_t data) final;

		/** Service io interrupts

			@see MH_II8080ArcadeIO::GenerateInterrupt
		*/
		uint8_t GenerateInterrupt(uint64_t currTime, uint64_t cycles) final;

		/** Write i8080 arcade vram to texture
		
			@see MH_II8080ArcadeIO::BlitVRAM
		*/
		void BlitVRAM(uint8_t* dst, uint8_t rowBytes, uint8_t* src) final;

		/** Blit options

			@see MH_II8080ArcadeIO::BlitVRAM
		*/
		void SetOptions(const char* options) final;

		/** Output video width

			@see MH_II8080ArcadeIO::GetVideoWidth
		*/
		int GetVideoWidth() const final;

		/** Output video height

			@see MH_II8080ArcadeIO::GetVideoHeight
		*/
		int GetVideoHeight() const final;
	};
} // namespace meen_hw::i8080_arcade

#endif // MEEN_HW_MH_I8080ARCADEIO_H