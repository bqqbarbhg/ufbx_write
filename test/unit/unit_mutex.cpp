#include "unit_test.h"

#define UFBXW_UNIT_TEST 1
#define UFBXWI_FEATURE_ATOMICS 1
#define UFBXWI_FEATURE_THREAD_POOL 1
#include "../../ufbx_write.c"

#include "util_threads.h"

#define UFBXWT_UNIT_CATEGORY "mutex"

UFBXWT_UNIT_TEST(mutex_lock)
{
	ufbxwt_thread_pool thread_pool;
	ufbxwi_thread_pool &tp = thread_pool.tp;

	uint32_t value = 0;
	ufbxwi_mutex mutex = { };

	const size_t num_threads = 16;
	const size_t num_iters = 100000;
	fork_threads(num_threads, num_iters, [&](size_t id, size_t index) {
		ufbxwi_mutex_lock(&tp, &mutex);
		value++;
		ufbxwi_mutex_unlock(&tp, &mutex);
	});

	ufbxwt_assert(value == num_threads * num_iters);
}

UFBXWT_UNIT_TEST(mutex_try_lock)
{
	ufbxwt_thread_pool thread_pool;
	ufbxwi_thread_pool &tp = thread_pool.tp;

	uint32_t value = 0;
	ufbxwi_mutex mutex = { };

	const size_t num_threads = 16;
	const size_t num_iters = 100000;
	fork_threads(num_threads, num_iters, [&](size_t id, size_t index) {
		for (;;) {
			if (!ufbxwi_mutex_try_lock(&tp, &mutex)) {
				continue;
			}

			value++;
			ufbxwi_mutex_unlock(&tp, &mutex);
			break;
		}
	});

	ufbxwt_assert(value == num_threads * num_iters);
}
