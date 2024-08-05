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

#ifdef ENABLE_MH_RP2040
#include <pico/stdlib.h>
#endif
#include <unity/unity.h>

#include "meen_hw/MH_Factory.h"
#include "meen_hw/MH_ResourcePool.h"

void setUp(){}
void tearDown(){}

namespace meen_hw::tests
{
	static std::unique_ptr<MH_II8080ArcadeIO> i8080ArcadeIO;
	static int resourceCounter = 0;

	static void suiteSetUp()
    {
		i8080ArcadeIO = MakeI8080ArcadeIO();
	}

	static int suiteTearDown(int numFailures)
	{
		return numFailures;
	}

	static void test_Version()
	{
		TEST_ASSERT_NOT_NULL(Version());
	}

	static void test_ResourcePool()
	{
		resourceCounter = 0;

		struct ResourceDeleter
		{
			void operator()(int*)
			{
				resourceCounter++;
			};
		};

		meen_hw::MH_ResourcePool<int, ResourceDeleter>::ResourcePtr outlivePool;

		{
			auto pool = meen_hw::MH_ResourcePool<int, ResourceDeleter>();

			// Add 3 resources
			int r1 = 0;
			int r2 = 0;
			int r3 = 0;

			pool.AddResource(&r1);
			pool.AddResource(&r2);
			pool.AddResource(&r3);

			{
				auto r1 = pool.GetResource();
				TEST_ASSERT_NOT_NULL(r1);
				auto r2 = pool.GetResource();
				TEST_ASSERT_NOT_NULL(r2);
				auto r3 = pool.GetResource();
				TEST_ASSERT_NOT_NULL(r3);

				// Pool should now be empty
				auto r4 = pool.GetResource();
				TEST_ASSERT_NULL(r4);
			}

			// All resources should be returned to the pool now
			{
				auto r1 = pool.GetResource();
				TEST_ASSERT_NOT_NULL(r1);
				auto r2 = pool.GetResource();
				TEST_ASSERT_NOT_NULL(r2);
				auto r3 = pool.GetResource();
				TEST_ASSERT_NOT_NULL(r3);

				// Pool should now be empty
				auto r4 = pool.GetResource();
				TEST_ASSERT_NULL(r4);
			}

			// All resources should be available
			outlivePool = pool.GetResource();
			TEST_ASSERT_NOT_NULL(outlivePool);
			*outlivePool = 42;
		}
		
		// The pool is dead, resource should be valid
		TEST_ASSERT_EQUAL(42, *outlivePool);
		// The pool is dead, the resource should be destroyed
		outlivePool = nullptr;

		//Check to total number of deletions
		TEST_ASSERT_EQUAL(3, resourceCounter);
	}
} // namespace meen_hw::tests

int main(void)
{
	int err = 0;
#ifdef ENABLE_MH_RP2040
	stdio_init_all();

	while(true)
#endif
	{
		meen_hw::tests::suiteSetUp();
		UNITY_BEGIN();
		RUN_TEST(meen_hw::tests::test_Version);
		RUN_TEST(meen_hw::tests::test_ResourcePool);
		
		if(meen_hw::tests::i8080ArcadeIO != nullptr)
		{
			//run i8080 arcade tests
		}
		
		err = meen_hw::tests::suiteTearDown(UNITY_END());

#ifdef ENABLE_MH_RP2040
		sleep_ms(1000);
#endif
	}

	return err;
}