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

#include "meen_hw/MH_Mutex.h"

#ifdef PICO_BOARD
    #include <pico/sync.h>
#else // use std::condition_variable
    #include <condition_variable>
#endif // PICO_BOARD

namespace meen_hw
{
#ifdef PICO_BOARD
    /** Pico Condition Variable implementation

        As of time of writing, the pico sdk (v2.1.1) does not have support for condition variable.
        This is a basic implementation using mutex and semaphore which implements the required
        methods of std::condition_variable_any.
    */
    class MH_ConditionVariable
    {
    private:
        semaphore_t sem;
        critical_section_t cs;
        int waiter_count = 0;
    public:
        MH_ConditionVariable()
        {
            critical_section_init(&cs);
            sem_init(&sem, 0, 1);
        }

        ~MH_ConditionVariable()
        {
            critical_section_deinit(&cs);
        }

        /** Wait on the condition being satisfied

            Block the current thread until the condition variable is notified or a spurious wakeup occurs.

            @param	mutex		A mutex implementation that satisfies BasicLockable.
            @param	predicate	A method that is called to detect spurious wakeups.
        */
        void wait(MH_Mutex& mutex, std::function<bool()>&& predicate)
        {
            while (predicate() == false)
            {
                // Count this waiter
                critical_section_enter_blocking(&cs);
                waiter_count++;
                critical_section_exit(&cs);

                mutex.unlock();
                // Block until signaled
                sem_acquire_blocking(&sem);
                // Reacquire mutex before checking predicate
                mutex.lock();
            }
        }

        /** Unblock one waiting thread

            If any threads are waiting on *this, calling notify_one unblocks one of the waiting threads.
        */
        void notify_one()
        {
            critical_section_enter_blocking(&cs);

            if (waiter_count > 0)
            {
                waiter_count--;
                sem_release(&sem);
            }

            critical_section_exit(&cs);
        }

        /** Unblocks all waiting threads

            If any threads are waiting on *this, calling notify_all unblocks all of the waiting threads.
        */
        void notify_all()
        {
            critical_section_enter_blocking(&cs);

            while (waiter_count > 0)
            {
                waiter_count--;
                sem_release(&sem);
            }

            critical_section_exit(&cs);
        }
    };
#else
    /** Default platform case for condition variable

        The default implementation will be std::condition_variable_any.
    */
    using MH_ConditionVariable = std::condition_variable_any;
#endif // PICO_BOARD
} // namespace meen_hw

#endif // MEEN_HW_MH_CONDITIONVARIABLE_H
