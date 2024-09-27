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

#ifndef MEEN_HW_MH_II8080ARCADEIO_H
#define MEEN_HW_MH_II8080ARCADEIO_H

#include <span>
#include <system_error>

namespace meen_hw
{
	/** Intel 8080 arcade hardware emulation.

		Designed to be used as a helper class for use
		in writing an i8080 arcade emulator.

		Emulates for the following hardware features:

		- Port IO read/writes
		- Video ram blitting
		- Screen Interrupt generation.
	*/
	class MH_II8080ArcadeIO
	{
	public:
		/** Read from the specified port

			Read the value from the input device (keyboard for example)
			and set the relevant bit in the return value according to
			the following:

			Port 0
				 bit 0 DIP4 (Seems to be self-test-request read at power up)
				 bit 1 Always 1
				 bit 2 Always 1
				 bit 3 Always 1
				 bit 4 Fire
				 bit 5 Left
				 bit 6 Right
				 bit 7 ? tied to demux port 7 ?

			Port 1
				 bit 0 = CREDIT (1 if deposit)
				 bit 1 = 2P start (1 if pressed)
				 bit 2 = 1P start (1 if pressed)
				 bit 3 = Always 1
				 bit 4 = 1P shot (1 if pressed)
				 bit 5 = 1P left (1 if pressed)
				 bit 6 = 1P right (1 if pressed)
				 bit 7 = Not connected

			Port 2
				 bit 0 = DIP3 00 = 3 ships  10 = 5 ships
				 bit 1 = DIP5 01 = 4 ships  11 = 6 ships
				 bit 2 = Tilt
				 bit 3 = DIP6 0 = extra ship at 1500, 1 = extra ship at 1000
				 bit 4 = P2 shot (1 if pressed)
				 bit 5 = P2 left (1 if pressed)
				 bit 6 = P2 right (1 if pressed)
				 bit 7 = DIP7 Coin info displayed in demo screen 0=ON

			Port 3
				bit 0-7 Shift register data

			@param		port		The input device to read from.

			@return		uint8_t		Non zero if the port was read from, zero otherwise.
		*/
		virtual uint8_t ReadPort(uint16_t port) = 0;

		/** Write to the specified port

			Write the data to the relevant output device
			according to the following:

			Port 2:
				bit 0,1,2 Shift amount

			Port 3: (discrete sounds)
				bit 0=UFO (repeats)        SX0 0.raw
				bit 1=Shot                 SX1 1.raw
				bit 2=Flash (player die)   SX2 2.raw
				bit 3=Invader die          SX3 3.raw
				bit 4=Extended play        SX4
				bit 5= AMP enable          SX5
				bit 6= NC (not wired)
				bit 7= NC (not wired)
			Port 4:
				bit 0-7 shift data (LSB on 1st write, MSB on 2nd)

			Port 5:
				bit 0=Fleet movement 1     SX6 4.raw
				bit 1=Fleet movement 2     SX7 5.raw
				bit 2=Fleet movement 3     SX8 6.raw
				bit 3=Fleet movement 4     SX9 7.raw
				bit 4=UFO Hit              SX10 8.raw
				bit 5= NC (Cocktail mode control ... to flip screen)
				bit 6= NC (not wired)
				bit 7= NC (not wired)

			Port 6:
				Watchdog ... read or write to reset

			@param	port		The output device to write to.
			@param	data		The data to write to the output device.

			@return				Audio that requires rendering as described above.
		*/
		virtual uint8_t WritePort(uint16_t port, uint8_t data) = 0;

		/** Generate the screen interrupt

			Return (interrupt service routine) 1 and
			(interrupt service routine) 2 at 60hz intervals.
			This informs the ROM that it is safe to draw to the
			top and bottom of the video ram.

			@return				0: no interrupt has occured.
								1: the 'beam' is near the centre of the screen.
								2: the 'beam' is at the end (vBlank). 
		*/
		virtual uint8_t GenerateInterrupt(uint64_t currTime, uint64_t cycles) = 0;

		/** Blit options

			The options applied to the output buffer when `BlitVRAM` is called.

			@param	options			The i8080 arcade blit options in json format.

									Configuration options are as follows:

									bpp: [1(default)|8|16] - the pixel format is determined by the calling application. 
									colour: ["white"(default)|"red"|"green"|"blue"|"random"|hex]
									orientation: ["cocktail"(default)|"upright"]
		*/
		virtual std::error_code SetOptions(const char* options) = 0;

		/** Write the i8080 arcade vram to a destination buffer.
		
			How the vram is blitted is dictated by the options specifed by `SetOptions`.

			@param	dstVRAM			The video memory to write to (texture memory).
			@param	dstVRAMRowBytes	The width of each dst vram scanline in bytes.
			@param	srcVRAM			The video ram to copy.

			@remark					No boundry checks are performed. It is expected that
									the dstVRAM is allocted using the GetVideoWidth and
									GetVideoHeight methods to determine a minimum allocaion
									size.
		*/
		virtual void BlitVRAM(std::span<uint8_t> dstVRAM, int dstVRAMRowBytes, std::span<uint8_t> srcVRAM) = 0;

		/** Output video width in pixels

			The option `blit-orientation` will determine this value. For a cocktail
			orientation (default), it will be 256 pixels. For an upright orientation
			it will be 224 pixels.

			@return				The width of the dstVRAM passed to BlitVRAM. 
		*/
		virtual int GetVRAMWidth() const = 0;

		/** Output video height in pixels

			The option `blit-orientation` will determine this value. For a cocktail
			orientation (default), it will be 224 pixels. For an upright orientation
			it will be 256 pixels.

			@return				The height of the dstVRAM passed to BlitVRAM.
		*/
		virtual int GetVRAMHeight() const = 0;

		virtual ~MH_II8080ArcadeIO() = default;
	};
} // namespace meen_hw

#endif // MEEN_HW_MH_I8080ARCADEIO_H