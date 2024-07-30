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

#include "meen_hw/MH_Error.h"

namespace meen_hw
{
	namespace detail
	{
		class category : public std::error_category
		{
		public:
			//cppcheck-suppress unusedFunction
			virtual const char* name() const noexcept override
			{
				return "meen_hw::category";
			}

			//cppcheck-suppress unusedFunction
			virtual std::string message(int ec) const override
			{
				switch(ec)
				{
					case errc::no_error:
						return "Success";
					case errc::bpp:
						return "The bpp configuration option is invalid";
					case errc::colour:
						return "The colour configuration option is invalid";
					case errc::orientation:
						return "The orientation configuration parameter is invalid";
					case errc::json_parse:
						return "A json parse error occurred while processing the configuration file";
					default:
						return "Unknown error code";
				}
			}
		};
	} // namespace detail

	//cppcheck-suppress unusedFunction
	const std::error_category& category()
	{
		static detail::category instance;
		return instance;
	}
} // namespace meen_hw