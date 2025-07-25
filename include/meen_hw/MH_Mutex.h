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

#ifndef MEEN_HW_MH_MUTEX_H
#define MEEN_HW_MH_MUTEX_H

#ifdef PICO_BOARD
	#include <pico/mutex.h>
	using mh_mutex = mutex_t;

	#define MH_MUTEX_INIT(m) mutex_init(&m)
	#define MH_MUTEX_LOCK(m) mutex_enter_blocking(&m)
	#define MH_MUTEX_TRY_LOCK(m) mutex_try_enter(&m, nullptr)
	#define MH_MUTEX_UNLOCK(m) mutex_exit(&m)
#else // use std::mutex
	#include <mutex>
	using mh_mutex = std::mutex;

	#define MH_MUTEX_INIT(m)
	#define MH_MUTEX_LOCK(m) m.lock()
	#define MH_MUTEX_TRY_LOCK(m) m.try_lock()
	#define MH_MUTEX_UNLOCK(m) m.unlock()
#endif // PICO_BOARD

namespace meen_hw
{
	/** Mutex wrapper

		A class which wraps all the supported mutex types.
		The mutex type (mh_mutex) is dependent on the platform being targeted.
		Supported mutex types are std::mutex and pico mutex.
	*/
	class MH_Mutex
	{
	private:
		mh_mutex mtx_;
	public:
		/** Default constructor

			Construct a mutex whose type is platform dependent.
		*/
		MH_Mutex()
		{
			MH_MUTEX_INIT(mtx_);
		}

		/** Destructor

			Use a default destructor.
		*/
		~MH_Mutex() = default;

		/** Acquire the mutex

			This will block until the mutex is acquired.
		*/
		void lock()
		{
			MH_MUTEX_LOCK(mtx_);
		}

		/** Acquire the mutex

			This is a non-blocking version of lock.

			@return		True if the mutex was acquired, false otherwise.
		*/
		bool try_lock()
		{
			return MH_MUTEX_TRY_LOCK(mtx_);
		}

		/** Release the mutex

			Allow other threads a change to acquire this mutex.
		*/
		void unlock()
		{
			MH_MUTEX_UNLOCK(mtx_);
		}
	};

	/** A simple lock guard implementation

		This could be a templated class, however, since it
		is only used for MH_Mutex, we keep the implementation
		specific to that.
	*/
	class MH_LockGuard
	{
	private:
		MH_Mutex& mtx_;
	public:
		/** Initialisation constructor

			Lock the assigned mutex until this instance is destructed.

			@param	mtx		The mutex to lock.
		*/
		explicit MH_LockGuard(MH_Mutex& mtx)
			: mtx_{ mtx }
		{
			mtx_.lock();
		}

		/** Destructor

			Unlock the mutex assigned in the constructor.
		*/
		~MH_LockGuard()
		{
			mtx_.unlock();
		}

		/** Copy constructor

			This constructor is deleted.
		*/
		MH_LockGuard(const MH_LockGuard&) = delete;

		/** Assignment operator

			This operator is deleted.
		*/
		MH_LockGuard& operator=(const MH_LockGuard&) = delete;
	};
} // namespace meen_hw

#endif // MEEN_HW_MH_MUTEX_H