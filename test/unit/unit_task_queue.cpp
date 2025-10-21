#include "unit_test.h"

#define UFBXW_UNIT_TEST 1
#define UFBXWI_FEATURE_ATOMICS 1
#define UFBXWI_FEATURE_THREAD_POOL 1
#define UFBXWI_FEATURE_ERROR 1
#define UFBXWI_FEATURE_ALLOCATOR 1
#define UFBXWI_FEATURE_LIST 1
#define UFBXWI_FEATURE_TASK_QUEUE 1
#include "../../ufbx_write.c"

#include "util_allocator.h"
#include "util_threads.h"
#include "util_task_queue.h"

#include <atomic>
#include <mutex>

#define UFBXWT_UNIT_CATEGORY "task_queue"

struct ufbxwt_empty_context { };

struct ufbxwt_mutex_context
{
	std::mutex mutex;
};

UFBXWT_UNIT_TEST(task_queue_simple)
{
	std::atomic_uint32_t value = 0;
	const size_t task_count = 100000;

	{
		ufbxwi_error error = { };
		ufbxwt_allocator ator { error };
		ufbxwt_thread_pool tp;
		ufbxwt_task_queue<ufbxwt_empty_context> tq { tp.tp, ator.ator, task_count };

		for (size_t i = 0; i < task_count; i++) {
			tq.run([&](ufbxwt_empty_context) {
				value.fetch_add(1u, std::memory_order_relaxed);
			});
		}
	}

	ufbxwt_assert(value.load() == task_count);
}

UFBXWT_UNIT_TEST(task_queue_mutex)
{
	std::atomic_uint32_t value = 0;
	const size_t task_count = 1000;

	{
		ufbxwi_error error = { };
		ufbxwt_allocator ator { error };
		ufbxwt_thread_pool tp { 100 };
		ufbxwt_task_queue<ufbxwt_mutex_context> tq { tp.tp, ator.ator, task_count };

		for (size_t i = 0; i < task_count; i++) {
			tq.run([&](ufbxwt_mutex_context &ctx) {
				std::lock_guard<std::mutex> lg { ctx.mutex };

				std::this_thread::sleep_for(std::chrono::milliseconds{10});
				value.fetch_add(1u, std::memory_order_relaxed);
			});
		}
	}

	ufbxwt_assert(value.load() == task_count);
}

UFBXWT_UNIT_TEST(task_queue_wait)
{
	const size_t task_count = 20;
	bool flags[task_count] = { };

	{
		ufbxwi_error error = { };
		ufbxwt_allocator ator { error };
		ufbxwt_thread_pool tp { 50 };
		ufbxwt_task_queue<ufbxwt_empty_context> tq { tp.tp, ator.ator, task_count };

		std::vector<ufbxwi_task_id> task_ids;

		for (size_t i = 0; i < task_count; i++) {
			ufbxwi_task_id task_id = tq.run([&flags, i](ufbxwt_empty_context) {
				std::this_thread::sleep_for(std::chrono::milliseconds{5 * i});
				flags[i] = true;
			});

			ufbxwt_assert(task_id > 0);

			task_ids.push_back(task_id);
		}

		for (size_t i = 0; i < task_count; i++) {
			tq.complete(task_ids[i]);
			ufbxwt_assert(flags[i]);
		}
	}
}
