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

#ifndef MEEN_HW_MH_ERROR_H
#define MEEN_HW_MH_ERROR_H

#include <system_error>

namespace meen_hw
{
	/** Meen HW Error codes

		Used to define error values that are compatible with std::error_code.
	*/
	enum errc
	{
		no_error,		//< No error has occurred.
		bpp,			//< The configuration value of bpp is invalid.
		colour,			//< The configuration value of colour is invalid.
		orientation,	//< The configuration value of orientation is invalid.
		json_parse		//< The JSON configuration file is malformed.
	};

	/** The custom meen_hw error category
	
		Defines the name of the error category and the messages that each meen_hw::errc returns.
	*/
	const std::error_category& category();

	/** std::error_code wrapper
	
		A simple convenience wrapper

		@param	ec	The meen_hw::errc to use to create a meen_hw std::error_code	
	*/
	inline std::error_code make_error_code(errc ec) { return {ec, meen_hw::category()}; }
} // namespace meen_hw

namespace std
{
	/** Boilerplate
	
		Inform std that our meen_hw::errc enum is a std::error_code enum.
	*/
	template<>
	struct is_error_code_enum<meen_hw::errc> : public std::true_type {};
} // namespace std

#endif // MEEN_HW_MH_ERROR_H