#ifndef UFBXW_CPP_THREADS_H_INCLUDED
#define UFBXW_CPP_THREADS_H_INCLUDED

#include <stddef.h>

#if !defined(ufbxw_cpp_threads_abi)
	#if defined(UFBXW_CPP_THREADS_STATIC)
		#define ufbxw_cpp_threads_abi static
	#else
		#define ufbxw_cpp_threads_abi
	#endif
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct ufbxw_cpp_threads_opts {
	size_t num_threads;
	// TODO: Custom task run callback
} ufbxw_cpp_threads_opts;

ufbxw_cpp_threads_abi void ufbxw_cpp_threads_setup(struct ufbxw_thread_pool *pool, const ufbxw_cpp_threads_opts *opts);

#if defined(__cplusplus)
}
#endif

#endif

#ifdef UFBXW_CPP_THREADS_IMPLEMENTATION
#ifndef UFBXW_CPP_THREADS_H_IMPLEMENTED
#define UFBXW_CPP_THREADS_H_IMPLEMENTED

#if !defined(__cplusplus)
	#error "ufbxw_cpp_threads.h should be implemented in a C++ file, though it can be used in another C file"
#endif

#if !defined(UFBXW_VERSION)
	#error "Please include ufbx_write.h before implementing ufbxw_cpp_threads.h"
#endif

#include <stdlib.h>
#include <thread>
#include <mutex>
#include <new>
#include <vector>
#include <memory>

#define UFBXW_CPP_THREADS_NUM_SLOTS 16
#define UFBXW_CPP_THREADS_NUM_ADDRS 4

typedef struct {
	std::condition_variable cv;
	uintptr_t ptr = 0;
	size_t waiters = 0;
} ufbxw_cpp_threads_addr;

typedef struct {
	std::mutex mutex;
	ufbxw_cpp_threads_addr addrs[UFBXW_CPP_THREADS_NUM_ADDRS];
	ufbxw_cpp_threads_addr fallback_addr;
} ufbxw_cpp_threads_slot;

typedef struct {
	ufbxw_cpp_threads_slot slots[UFBXW_CPP_THREADS_NUM_SLOTS];
	std::vector<std::thread> threads;
} ufbxw_cpp_threads_ctx;

static uint32_t ufbxw_cpp_threads_slot_from_ptr(const void *ptr)
{
	uint64_t p = (uint64_t)(uintptr_t)ptr;
	p ^= p >> 30;
    p *= 0xbf58476d1ce4e5b9U;
	p ^= p >> 27;
	uint32_t q = (uint32_t)p;
	q *= 0x846ca68bU;
	q ^= q >> 16;
	return q & (UFBXW_CPP_THREADS_NUM_SLOTS - 1);
}

static_assert(sizeof(std::atomic_uint32_t) == sizeof(uint32_t), "std::atomic_uint32_t must match the size of uint32_t");

static void ufbxw_cpp_threads_run_fn(ufbxw_thread_pool_context ctx, uint32_t thread_id)
{
	ufbxw_thread_pool_blocking_run_tasks(ctx, thread_id, SIZE_MAX);
}

static bool ufbxw_cpp_threads_thread_pool_init_fn(void *user, ufbxw_thread_pool_context ctx)
{
	ufbxw_cpp_threads_opts default_opts;
	const ufbxw_cpp_threads_opts *opts = (const ufbxw_cpp_threads_opts*)user;
	if (!opts) {
		memset(&default_opts, 0, sizeof(default_opts));
		opts = &default_opts;
	}

	size_t total_threads = opts->num_threads ? opts->num_threads : (size_t)std::thread::hardware_concurrency();
	if (total_threads <= 1) {
		return false;
	}

	// Count the main thread as a thread
	const size_t num_threads = total_threads - 1;

	std::unique_ptr<ufbxw_cpp_threads_ctx> tc { new ufbxw_cpp_threads_ctx() };
	if (!tc) return false;

	tc->threads.reserve(total_threads);
	for (size_t i = 0; i < total_threads; i++) {
		tc->threads.emplace_back(ufbxw_cpp_threads_run_fn, ctx, (uint32_t)i);
	}

	ufbxw_thread_pool_set_user_ptr(ctx, tc.release());
	return true;
}

static ufbxw_cpp_threads_addr *ufbxw_cpp_threads_add_addr(ufbxw_cpp_threads_slot &slot, uintptr_t ptr)
{
	ufbxw_cpp_threads_addr *to_insert = nullptr;
	for (ufbxw_cpp_threads_addr &addr : slot.addrs) {
		if (addr.ptr == ptr) {
			return &addr;
		} else if (addr.ptr == 0 && !to_insert) {
			to_insert = &addr;
		}
	}

	if (to_insert) {
		to_insert->ptr = ptr;
		return to_insert;
	}

	return &slot.fallback_addr;
}

static void ufbxw_cpp_threads_thread_pool_wait_fn(void *user, ufbxw_thread_pool_context ctx, uint32_t *p_value, uint32_t ref_value)
{
	ufbxw_cpp_threads_ctx *tc = (ufbxw_cpp_threads_ctx*)ufbxw_thread_pool_get_user_ptr(ctx);
	uint32_t slot_ix = ufbxw_cpp_threads_slot_from_ptr(p_value);
	ufbxw_cpp_threads_slot *slot = &tc->slots[slot_ix];

	std::atomic_uint32_t *p_atomic = (std::atomic_uint32_t*)p_value;

	std::unique_lock<std::mutex> lock { slot->mutex };

	ufbxw_cpp_threads_addr *addr = ufbxw_cpp_threads_add_addr(*slot, (uintptr_t)p_value);
	addr->waiters++;

	while (p_atomic->load(std::memory_order_acquire) == ref_value) {
		addr->cv.wait(lock);
	}

	if (--addr->waiters == 0) {
		addr->ptr = 0;
	}
}

static void ufbxw_cpp_threads_thread_pool_notify_fn(void *user, ufbxw_thread_pool_context ctx, uint32_t *p_value, uint32_t wake_count)
{
	ufbxw_cpp_threads_ctx *tc = (ufbxw_cpp_threads_ctx*)ufbxw_thread_pool_get_user_ptr(ctx);
	uint32_t slot_ix = ufbxw_cpp_threads_slot_from_ptr(p_value);
	ufbxw_cpp_threads_slot *slot = &tc->slots[slot_ix];

	std::unique_lock<std::mutex> lock { slot->mutex };

	uintptr_t ptr = (uintptr_t)p_value;
	for (ufbxw_cpp_threads_addr &addr : slot->addrs) {
		if (addr.ptr == ptr) {
			if (wake_count == 1) {
				addr.cv.notify_one();
			} else {
				addr.cv.notify_all();
			}
		}
	}

	// We cannot be sure if the waiters are relevant to us, just wake everyone up.
	if (slot->fallback_addr.waiters > 0) {
		slot->fallback_addr.cv.notify_all();
	}
}

static void ufbxw_cpp_threads_thread_pool_free_fn(void *user, ufbxw_thread_pool_context ctx)
{
	ufbxw_cpp_threads_ctx *tc = (ufbxw_cpp_threads_ctx*)ufbxw_thread_pool_get_user_ptr(ctx);
	for (std::thread &thread : tc->threads) {
		thread.join();
	}
	delete tc;
}

#if defined(__cplusplus)
extern "C" {
#endif

ufbxw_cpp_threads_abi void ufbxw_cpp_threads_setup(struct ufbxw_thread_pool *pool, const ufbxw_cpp_threads_opts *opts)
{
	pool->init_fn = &ufbxw_cpp_threads_thread_pool_init_fn;
	pool->wait_fn = &ufbxw_cpp_threads_thread_pool_wait_fn;
	pool->notify_fn = &ufbxw_cpp_threads_thread_pool_notify_fn;
	pool->free_fn = &ufbxw_cpp_threads_thread_pool_free_fn;
	pool->user = (void*)opts;
}

#if defined(__cplusplus)
}
#endif

#endif
#endif


