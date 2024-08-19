#ifndef MEEN_HW_MH_MUTEX_H
#define MEEN_HW_MH_MUTEX_H

#ifdef ENABLE_MH_RP2040
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
#endif // ENABLE_MH_RP2040

namespace meen_hw
{
	class MH_Mutex
	{
	private:
		mh_mutex mtx_;
	public:
		MH_Mutex()
		{
			MH_MUTEX_INIT(mtx_);
		}

		~MH_Mutex() = default;

		void lock()
		{
			MH_MUTEX_LOCK(mtx_);
		}

		bool try_lock()
		{
			return MH_MUTEX_TRY_LOCK(mtx_);
		}

		void unlock()
		{
			MH_MUTEX_UNLOCK(mtx_);
		}
	};

	// Probably need a CritSec too

	class MH_LockGuard
	{
	private:
		MH_Mutex& mtx_;
	public:
		explicit MH_LockGuard(MH_Mutex& mtx)
			: mtx_{ mtx }
		{
			mtx_.lock();
		}

		~MH_LockGuard()
		{
			mtx_.unlock();
		}

		MH_LockGuard(const MH_LockGuard&) = delete;
		MH_LockGuard& operator=(const MH_LockGuard&) = delete;
	};
} // namespace meen_hw

#endif // MEEN_HW_MH_MUTEX_H