/*
Copyright (c) 2021-2025 Nicolas Beddows <nicolas.beddows@gmail.com>

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

		- Port IO read/writes.
		- Video ram blitting.
		- Screen Interrupt generation.
	*/
	class MH_II8080ArcadeIO
	{
	public:
		/** Read from the specified port

			Read the value from the input device (keyboard for example)
			and set the relevant bit in the return value according to
			the following table:

			<table>
			  <thead>
			    <tr>
				  <th>Port</th>
				  <th>Bit</th>
			      <th>Remarks</th>
			    </tr>
			  </thead>
			  <tbody>
			    <tr>
			      <td rowspan=8>0</td>
			      <td>0</td>
			      <td>DIP4 (Seems to be self-test-request read at power up)</td>
    		    </tr>
			    <tr>
			      <td>1</td>
			      <td>Always set (1)</td>
    		    </tr>
    		    <tr>
      		      <td>2</td>
	              <td>ALways Set (1)</td>
    		    </tr>
    		    <tr>
      		      <td>3</td>
			      <td>Always Set (1)</td>
			    </tr>
			    <tr>
			      <td>4</td>
			      <td>Fire</td>
			    </tr>
			    <tr>
			      <td>5</td>
			      <td>Left</td>
			    </tr>
  			    <tr>
			      <td>6</td>
			      <td>Right</td>
			    </tr>
			    <tr>
			      <td>7</td>
			      <td>? tied to demux port 7 ?</td>
			    </tr>
			    <tr>
			      <td rowspan=8>1</td>
			      <td>0</td>
			      <td>CREDIT (1 if deposit)</td>
    		    </tr>
  			    <tr>
			      <td>1</td>
			      <td>2P start (1 if pressed)</td>
			    </tr>
  			    <tr>
			      <td>2</td>
			      <td>1P start (1 if pressed)</td>
			    </tr>
  			    <tr>
			      <td>3</td>
			      <td>Always 1</td>
			    </tr>
  			    <tr>
			      <td>4</td>
			      <td>1P shot (1 if pressed)</td>
			    </tr>
  			    <tr>
			      <td>5</td>
			      <td>1P left (1 if pressed)</td>
			    </tr>
  			    <tr>
			      <td>6</td>
			      <td>1P right (1 if pressed)</td>
			    </tr>
  			    <tr>
			      <td>7</td>
			      <td>Not connected</td>
			    </tr>
			    <tr>
			      <td rowspan=8>2</td>
			      <td>0</td>
			      <td>DIP3 00 = 3 ships  10 = 5 ships</td>
    		    </tr>
  			    <tr>
			      <td>1</td>
			      <td>DIP5 01 = 4 ships  11 = 6 ships</td>
			    </tr>
  			    <tr>
			      <td>2</td>
			      <td>Tilt</td>
			    </tr>
  			    <tr>
			      <td>3</td>
			      <td>DIP6 0 = extra ship at 1500, 1 = extra ship at 1000</td>
			    </tr>
  			    <tr>
			      <td>4</td>
			      <td>2P shot (1 if pressed)</td>
			    </tr>
  			    <tr>
			      <td>5</td>
			      <td>2P left (1 if pressed)</td>
			    </tr>
  			    <tr>
			      <td>6</td>
			      <td>2P right (1 if pressed)</td>
			    </tr>
  			    <tr>
			      <td>7</td>
			      <td>DIP7 Coin info displayed in demo screen 0=ON</td>
			    </tr>
			    <tr>
			      <td>3</td>
			      <td>0-7</td>
			      <td>Shift register data</td>
    		    </tr>
	   		  </tbody>
			</table>

			@param		port		The input device to read from.

			@remark		Must be called from the same thread as WritePort.

			@return		Non zero if the port was read from, zero otherwise.
		*/
		virtual uint8_t ReadPort(uint16_t port) = 0;

		/** Write to the specified port

			Write the data to the relevant output device
			according to the following table:

			<table>
			  <thead>
			    <tr>
				  <th>Port</th>
				  <th>Bit</th>
			      <th>Remarks</th>
			    </tr>
			  </thead>
			  <tbody>
			    <tr>
			      <td>2</td>
			      <td>0,1,2</td>
			      <td>Shift amount</td>
    		    </tr>
			    <tr>
			      <td rowspan=8>3 (sounds)</td>
			      <td>0</td>
			      <td>UFO (repeats)</td>
    		    </tr>
  			    <tr>
			      <td>1</td>
			      <td>Shot</td>
			    </tr>
  			    <tr>
			      <td>2</td>
			      <td>Flash (player die)</td>
			    </tr>
  			    <tr>
			      <td>3</td>
			      <td>Invader die</td>
			    </tr>
  			    <tr>
			      <td>4</td>
			      <td>Extended play</td>
			    </tr>
  			    <tr>
			      <td>5</td>
			      <td>AMP enable</td>
			    </tr>
  			    <tr>
			      <td>6</td>
			      <td>NC (not wired)</td>
			    </tr>
  			    <tr>
			      <td>7</td>
			      <td>NC (not wired)</td>
			    </tr>
			    <tr>
			      <td>4</td>
			      <td>0-7</td>
			      <td>Shift data (LSB on 1st write, MSB on 2nd)</td>
    		    </tr>
			    <tr>
			      <td rowspan=8>3 (sounds)</td>
			      <td>0</td>
			      <td>Fleet movement 1</td>
    		    </tr>
  			    <tr>
			      <td>1</td>
			      <td>Fleet movement 2</td>
			    </tr>
  			    <tr>
			      <td>2</td>
			      <td>Fleet movement 3</td>
			    </tr>
  			    <tr>
			      <td>3</td>
			      <td>Fleet movement 4</td>
			    </tr>
  			    <tr>
			      <td>4</td>
			      <td>UFO Hit</td>
			    </tr>
  			    <tr>
			      <td>5</td>
			      <td>NC (Cocktail mode control to flip screen)</td>
			    </tr>
  			    <tr>
			      <td>6</td>
			      <td>NC (not wired)</td>
			    </tr>
  			    <tr>
			      <td>7</td>
			      <td>NC (not wired)</td>
			    </tr>
			    <tr>
			      <td>6</td>
			      <td>0-7</td>
			      <td>Watchdog: read or write to reset</td>
    		    </tr>
			  </tbody>
			</table>

			@param	port		The output device to write to.
			@param	data		The data to write to the output device.

			@remark				Must be called from the same thread as ReadPort.

			@return				Audio that requires rendering as described above.
		*/
		virtual uint8_t WritePort(uint16_t port, uint8_t data) = 0;

		/** Generate the screen interrupt

			Return (interrupt service routine) 1 and
			(interrupt service routine) 2 at 60hz intervals.
			This informs the ROM that it is safe to draw to the
			top and bottom of the video ram.

			@param	currTime	The current elapsed time in nanoseconds.
			@param	cycles		The current number of elapsed ticks.

			@remark				Must be called from the same thread as Reset.

			@return				0: no interrupt has occured.
								1: the 'beam' is near the centre of the screen.
								2: the 'beam' is at the end (vBlank). 
		*/
		virtual uint8_t GenerateInterrupt(uint64_t currTime, uint64_t cycles) = 0;

		/** Reset the non-configurable state

			Reset the internal state (not the state that can be configured via the SetOptions method)
			to that of when this instance was first instantiated.

			@remark				Must be called from the same thread as calls to WritePort, ReadPort and
								GenerateInterrupt.
		*/
		virtual void Reset() = 0;

		/** Blit options

			The options applied to the output buffer when BlitVRAM is called:

			<table>
			  <thead>
			    <tr>
			      <th>Option</th>
				  <th>Value</th>
				  <th>Remarks</th>
				</tr>
			  </thead>
			  <tbody>
			    <tr>
				  <td rowspan=3>"bpp"</td>
				  <td>1(default)</td>
				  <td>1 bit per pixel</td>
				</tr>
				<tr>
				  <td>8</td>
				  <td>8 bits per pixel</td>
				</tr>
				<tr>
				  <td>16</td>
				  <td>16 bits per pixel</td>
				</tr>
			    <tr>
				  <td rowspan=6>"colour"</td>
				  <td>"white"(default)</td>
				  <td>White on black background</td>
				</tr>
				<tr>
				  <td>"red"</td>
				  <td>Red on black background</td>
				</tr>
				<tr>
				  <td>"green"</td>
				  <td>Green on black background</td>
				</tr>
				<tr>
				  <td>"blue"</td>
				  <td>Blue on black background</td>
				</tr>
				<tr>
				  <td>"random"</td>
				  <td>A random colour on black background</td>
				</tr>
				<tr>
				  <td>"hex"</td>
				  <td>A custom on black background</td>
				</tr>
			    <tr>
				  <td rowspan=2>"orientation"</td>
				  <td>"cocktail"(default)</td>
				  <td>Native orientation</td>
				</tr>
				<tr>
				  <td>"upright"</td>
				  <td>cocktail orientation rotated 90 degrees counter clockwise</td>
				</tr>
			  </tbody>
			</table>

			@param	options			The i8080 arcade blit options in json format according to the previous table.<br>
									For example: {"bpp":8,"colour":"green"}

			@remark					Must be called from the same thread as BlitVRAM.
		*/
		virtual std::error_code SetOptions(const char* options) = 0;

		/** Write the i8080 arcade vram to a destination buffer.

			How the vram is blitted is dictated by the options specifed by SetOptions.

			@param	dst				The video memory to write to (texture memory).
			@param	dstWidth		The width of the dst vram in pixels.
			@param	dstRowBytes		The width of each dst vram scanline in bytes.
			@param	src				The video ram to blit in the native pixel format.
			@param	srcWidth		The width of the src vram in pixels.

			@remark					The src is assumed to be contiguous without padding.
			@remark					Must be called from the same thread as SetOptions.
		*/
		virtual void BlitVRAM(std::span<uint8_t> dst, int dstWidth, int dstRowBytes, std::span<uint8_t> src, int srcWidth) = 0;

		/** Destructor

			Destroy the i8080 arcade hardware io emulator.
		*/
		virtual ~MH_II8080ArcadeIO() = default;
	};
} // namespace meen_hw

#endif // MEEN_HW_MH_I8080ARCADEIO_H