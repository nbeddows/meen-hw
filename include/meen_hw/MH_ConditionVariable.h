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

#ifndef MEEN_HW_MH_CONDITIONVARIABLE_H
#define MEEN_HW_MH_CONDITIONVARIABLE_H

#include <functional>

#ifdef PICO_BOARD
	using mh_cv = int;
	// We have no condition variable on pico, so we just use a spin lock
	#define MH_CV_WAIT(ul, p) while(p() == false);
	#define MH_CV_NOTIFY_ONE()
#else // use std::mutex
	#include <condition_variable>

	#include "meen_hw/MH_Mutex.h"

	using mh_cv = std::condition_variable;

	#define MH_CV_WAIT(ul, p) cv_.wait(ul, p); // to ignore spurious awakenings: while (!p()) cv_.wait(m);
	#define MH_CV_NOTIFY_ONE() cv_.notify_one();
#endif // PICO_BOARD

namespace meen_hw
{
	/** Condition Variable wrapper

		A class which wraps all the supported condition variable types.
		The condition variable type (mh_cv) is dependent on the platform being targeted.
		Supported condition variable types are std::condition_variable and spin lock on pico (no condition variable support ... I don't think).
	*/
	class MH_ConditionVariable
	{
	private:
		mh_cv cv_;
	public:
		/** Default constructor

			Construct a condition variable whose type is platform dependent.
		*/
		MH_ConditionVariable() = default;

		/** Destructor

			Use a default destructor.
		*/
		~MH_ConditionVariable() = default;

		/** Wait on the condition being satisfied

			Block the current thread until the condition variable is notified or a spurious wakeup occurs.
		*/
		void wait(mh_unique_lock& lock, std::function<bool()>&& predicate)
		{
			MH_CV_WAIT(lock, predicate);
		}

		/** Unblock one waiting thread

			If any threads are waiting on *this, calling notify_one unblocks one of the waiting threads.
		*/
		void notify_one()
		{
			return MH_CV_NOTIFY_ONE();
		}
	};
} // namespace meen_hw

#endif // MEEN_HW_MH_CONDITIONVARIABLE_H