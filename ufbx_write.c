#ifndef UFBXW_UFBX_WRITE_C_INCLUDED
#define UFBXW_UFBX_WRITE_C_INCLUDED

#if defined(UFBX_HEADER_PATH)
	#include UFBX_HEADER_PATH
#else
	#include "ufbx_write.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

// -- Math
// TODO: External
#include <math.h>

// -- Malloc

#if defined(ufbxw_malloc) || defined(ufbxw_realloc) || defined(ufbxw_free)
	// User provided allocators
	#if !defined(ufbxw_malloc) || !defined(ufbxw_realloc) || !defined(ufbxw_free)
		#error Inconsistent custom global allocator
	#endif
#else
	#define ufbxw_malloc(size) malloc((size))
	#define ufbxw_realloc(ptr, old_size, new_size) realloc((ptr), (new_size))
	#define ufbxw_free(ptr, old_size) free((ptr))
#endif

// -- Platform

#if !defined(UFBXW_STANDARD_C) && defined(_MSC_VER)
	#define ufbxwi_noinline __declspec(noinline)
	#define ufbxwi_forceinline __forceinline
	#define ufbxwi_restrict __restrict
	#if defined(_Check_return_)
		#define ufbxwi_nodiscard _Check_return_
	#else
		#define ufbxwi_nodiscard
	#endif
	#define ufbxwi_unused
	#define ufbxwi_unlikely(cond) (cond)
#elif !defined(UFBXW_STANDARD_C) && (defined(__GNUC__) || defined(__clang__))
	#define ufbxwi_noinline __attribute__((noinline))
	#define ufbxwi_forceinline inline __attribute__((always_inline))
	#define ufbxwi_restrict __restrict
	#define ufbxwi_nodiscard __attribute__((warn_unused_result))
	#define ufbxwi_unused __attribute__((unused))
	#define ufbxwi_unlikely(cond) __builtin_expect((cond), 0)
#else
	#define ufbxwi_noinline
	#define ufbxwi_forceinline
	#define ufbxwi_nodiscard
	#define ufbxwi_restrict
	#define ufbxwi_unused
	#define ufbxwi_unlikely(cond) (cond)
#endif

#if defined(UFBXW_STATIC_ANALYSIS)
	bool ufbxwi_analysis_opaque;
	#define ufbxwi_maybe_null(ptr) (ufbxwi_analysis_opaque ? (ptr) : NULL)
	#define ufbxwi_analysis_assert(cond) ufbxw_assert(cond)
#else
	#define ufbxwi_maybe_null(ptr) (ptr)
	#define ufbxwi_analysis_assert(cond) (void)0
#endif

#if defined(__GNUC__) && !defined(__clang__)
	#define ufbxwi_ignore(cond) (void)!(cond)
#else
	#define ufbxwi_ignore(cond) (void)(cond)
#endif

// -- Pointer alignment

#if !defined(UFBXW_STANDARD_C) && defined(__GNUC__) && defined(__has_builtin)
	#if __has_builtin(__builtin_is_aligned)
		#define ufbxwi_is_aligned(m_ptr, m_align) __builtin_is_aligned((m_ptr), (m_align))
		#define ufbxwi_is_aligned_mask(m_ptr, m_align) __builtin_is_aligned((m_ptr), (m_align) + 1)
	#endif
#endif
#ifndef ufbxwi_is_aligned
	#define ufbxwi_is_aligned(m_ptr, m_align) (((uintptr_t)(m_ptr) & ((m_align) - 1)) == 0)
	#define ufbxwi_is_aligned_mask(m_ptr, m_align) (((uintptr_t)(m_ptr) & (m_align)) == 0)
#endif

// -- Bit manipulation

#if defined(__cplusplus)
	#define ufbxwi_extern_c extern "C"
#else
	#define ufbxwi_extern_c
#endif

#if !defined(UFBXW_STANDARD_C) && defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
	ufbxwi_extern_c unsigned char _BitScanReverse(unsigned long * _Index, unsigned long _Mask);
	ufbxwi_extern_c unsigned char _BitScanReverse64(unsigned long * _Index, unsigned __int64 _Mask);
	static ufbxwi_forceinline ufbxwi_unused uint32_t ufbxwi_lzcnt32(uint32_t v) {
		unsigned long index;
		_BitScanReverse(&index, (unsigned long)v);
		return 31 - (uint32_t)index;
	}
	static ufbxwi_forceinline ufbxwi_unused uint32_t ufbxwi_lzcnt64(uint64_t v) {
		unsigned long index;
		#if defined(_M_X64)
			_BitScanReverse64(&index, (unsigned __int64)v);
		#else
			uint32_t hi = (uint32_t)(v >> 32u);
			uint32_t hi_nonzero = hi != 0 ? 1 : 0;
			uint32_t part = hi_nonzero ? hi : (uint32_t)v;
			_BitScanReverse(&index, (unsigned long)part);
			index += hi_nonzero * 32u;
		#endif
		return 63 - (uint32_t)index;
	}
#elif !defined(UFBXW_STANDARD_C) && (defined(__GNUC__) || defined(__clang__))
	#define ufbxwi_lzcnt32(v) ((uint32_t)__builtin_clz((unsigned)(v)))
	#define ufbxwi_lzcnt64(v) ((uint32_t)__builtin_clzll((unsigned long long)(v)))
#else
	// DeBrujin table lookup
	static const uint8_t ufbxwi_lzcnt32_table[] =  {
		31, 22, 30, 21, 18, 10, 29, 2, 20, 17, 15, 13, 9, 6, 28, 1, 23, 19, 11, 3, 16, 14, 7, 24, 12, 4, 8, 25, 5, 26, 27, 0,
	};
	static const uint8_t ufbxwi_lzcnt64_table[] = {
		63, 16, 62, 7, 15, 36, 61, 3, 6, 14, 22, 26, 35, 47, 60, 2, 9, 5, 28, 11, 13, 21, 42,
		19, 25, 31, 34, 40, 46, 52, 59, 1, 17, 8, 37, 4, 23, 27, 48, 10, 29, 12, 43, 20, 32, 41,
		53, 18, 38, 24, 49, 30, 44, 33, 54, 39, 50, 45, 55, 51, 56, 57, 58, 0,
	};
	static ufbxwi_noinline ufbxwi_unused uint32_t ufbxwi_lzcnt32(uint32_t v) {
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		return ufbxwi_lzcnt32_table[(v * 0x07c4acddu) >> 27];
	}
	static ufbxwi_noinline ufbxwi_unused uint32_t ufbxwi_lzcnt64(uint64_t v) {
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v |= v >> 32;
		return ufbxwi_lzcnt64_table[(v * UINT64_C(0x03f79d71b4cb0a89)) >> 58];
	}
#endif

// -- Error

typedef struct {
	bool failed;
	ufbxw_error error;
} ufbxwi_error;

static void ufbxwi_fail_imp_err(ufbxwi_error *error)
{
	error->failed = true;
}

#define ufbxwi_check_err(err, cond) do { if (!(cond)) { ufbxwi_fail_imp_err((err)); return; } } while (0)
#define ufbxwi_check(cond) do { if (!(cond)) { ufbxwi_fail_imp_err(&scene->error); return; } } while (0)

#define ufbxwi_check_return_err(err, cond, ret) do { if (!(cond)) { ufbxwi_fail_imp_err((err)); return ret; } } while (0)
#define ufbxwi_check_return(cond, ret) do { if (!(cond)) { ufbxwi_fail_imp_err(&scene->error); return ret; } } while (0)

// -- Utility

static ufbxwi_forceinline size_t ufbxwi_min_sz(size_t a, size_t b) { return a < b ? a : b; }
static ufbxwi_forceinline size_t ufbxwi_max_sz(size_t a, size_t b) { return a < b ? b : a; }

#define ufbxwi_arraycount(arr) (sizeof(arr) / sizeof(*(arr)))
#define ufbxwi_for(m_type, m_name, m_begin, m_num) for (m_type *m_name = m_begin, *m_name##_end = ufbxwi_add_ptr(m_name, m_num); m_name != m_name##_end; m_name++)
#define ufbxwi_for_ptr(m_type, m_name, m_begin, m_num) for (m_type **m_name = m_begin, **m_name##_end = ufbxwi_add_ptr(m_name, m_num); m_name != m_name##_end; m_name++)

// WARNING: Evaluates `m_list` twice!
#define ufbxwi_for_list(m_type, m_name, m_list) for (m_type *m_name = (m_list).data, *m_name##_end = ufbxwi_add_ptr(m_name, (m_list).count); m_name != m_name##_end; m_name++)
#define ufbxwi_for_ptr_list(m_type, m_name, m_list) for (m_type **m_name = (m_list).data, **m_name##_end = ufbxwi_add_ptr(m_name, (m_list).count); m_name != m_name##_end; m_name++)

// -- Allocator

#if defined(UFBXW_REGRESSION)
static const char ufbxwi_zero_size_buffer[4096] = { 0 };
#else
static const char ufbxwi_zero_size_buffer[64] = { 0 };
#endif

#ifndef UFBXW_MAXIMUM_ALIGNMENT
enum { UFBXW_MAXIMUM_ALIGNMENT = sizeof(void*) > 8 ? sizeof(void*) : 8 };
#endif

static ufbxwi_forceinline size_t ufbxwi_align_to_mask(size_t value, size_t align_mask)
{
	return value + (((size_t)0 - value) & align_mask);
}

static ufbxwi_forceinline size_t ufbxwi_size_align_mask(size_t size)
{
	// Align to the all bits below the lowest set one in `size` up to the maximum alignment.
	return ((size ^ (size - 1)) >> 1) & (UFBXW_MAXIMUM_ALIGNMENT - 1);
}

enum {
	UFBXWI_MIN_SIZE_CLASS_LOG2 = 6,
	UFBXWI_MIN_SIZE_CLASS_SIZE = 1 << UFBXWI_MIN_SIZE_CLASS_LOG2,
	UFBXWI_MAX_SIZE_CLASS_LOG2 = 12,
	UFBXWI_MAX_SIZE_CLASS_SIZE = 1 << UFBXWI_MAX_SIZE_CLASS_LOG2,
	UFBXWI_SIZE_CLASS_COUNT = UFBXWI_MAX_SIZE_CLASS_LOG2 - UFBXWI_MIN_SIZE_CLASS_LOG2,
};

static uint32_t ufbxwi_get_size_class(size_t size)
{
	if (size > UFBXWI_MAX_SIZE_CLASS_SIZE) {
		return UINT32_MAX;
	} else if (size <= UFBXWI_MIN_SIZE_CLASS_SIZE) {
		return 0;
	} else {
		return (32 - ufbxwi_lzcnt32((uint32_t)size - 1)) - UFBXWI_MIN_SIZE_CLASS_LOG2;
	}
}

#define UFBXWI_HUGE_MAGIC  0x68617775
#define UFBXWI_BLOCK_MAGIC 0x62617775u
#define UFBXWI_ALLOC_MAGIC 0x61617775u
#define UFBXWI_FREED_MAGIC 0x66617775u

typedef struct ufbxwi_alloc ufbxwi_alloc;
struct ufbxwi_alloc {
	ufbxwi_alloc *prev;
	ufbxwi_alloc *next;
	size_t size;
	size_t magic;
};

typedef struct {
	ufbxwi_error *error;
	ufbxw_allocator ator;
	size_t max_size;
	size_t num_allocs;
	size_t max_allocs;

	ufbxwi_alloc *free_root[UFBXWI_SIZE_CLASS_COUNT];
	ufbxwi_alloc block_root;

	void *current_block;
	size_t current_pos;
	size_t current_size;

	size_t next_block_size;
} ufbxwi_allocator;

static ufbxwi_forceinline bool ufbxwi_does_overflow(size_t total, size_t a, size_t b)
{
	// If `a` and `b` have at most 4 bits per `size_t` byte, the product can't overflow.
	if (((a | b) >> sizeof(size_t)*4) != 0) {
		if (a != 0 && total / a != b) return true;
	}
	return false;
}

static ufbxwi_noinline ufbxwi_alloc *ufbxwi_alloc_block(ufbxwi_allocator *ator, size_t size)
{
	size_t alloc_size = sizeof(ufbxwi_alloc) + size;
	ufbxwi_alloc *block = NULL;
	if (ator->ator.alloc_fn) {
		block = (ufbxwi_alloc*)ator->ator.alloc_fn(ator->ator.user, size);
	} else {
		block = ufbxw_malloc(alloc_size);
	}
	if (!block) return NULL;

	block->size = size;
	block->prev = &ator->block_root;
	block->next = ator->block_root.next;
	ator->block_root.next = block;

	return block;
}

static ufbxwi_noinline void ufbxwi_free_block(ufbxwi_allocator *ator, ufbxwi_alloc *block)
{
	if (block->prev) block->prev->next = block->next;
	if (block->next) block->next->prev = block->prev;

	size_t alloc_size = block->size + sizeof(ufbxwi_alloc);
	block->magic = UFBXWI_FREED_MAGIC;
	if (ator->ator.alloc_fn) {
		if (ator->ator.free_fn) {
			ator->ator.free_fn(ator->ator.user, block, alloc_size);
		}
	} else {
		ufbxw_free(block, alloc_size);
	}
}

static ufbxwi_noinline void *ufbxwi_alloc_size(ufbxwi_allocator *ator, size_t size, size_t n, size_t *p_alloc_size)
{
	// Always succeed with an empty non-NULL buffer for empty allocations
	ufbxw_assert(size > 0);
	if (n == 0) return (void*)ufbxwi_zero_size_buffer;

	size_t total = size * n;
	ufbxwi_check_return_err(ator->error, !ufbxwi_does_overflow(total, size, n), NULL);

	uint32_t size_class = ufbxwi_get_size_class(total);
	if (size_class == ~0u) {
		ufbxwi_alloc *block = ufbxwi_alloc_block(ator, total);
		ufbxwi_check_return_err(ator->error, block, NULL);
		block->magic = UFBXWI_HUGE_MAGIC;

		if (p_alloc_size) {
			*p_alloc_size = total;
		}
		return block + 1;
	}

	size_t size_class_size = (size_t)UFBXWI_MIN_SIZE_CLASS_SIZE << size_class;
	ufbxwi_alloc *freed = ator->free_root[size_class];
	if (freed) {
		freed->magic = UFBXWI_ALLOC_MAGIC;
		ator->free_root[size_class] = freed->next;
		if (p_alloc_size) {
			*p_alloc_size = size_class_size;
		}
		return freed + 1;
	}

	size_t alloc_size = size_class_size + sizeof(ufbxwi_alloc);
	if (ufbxwi_unlikely(ator->current_size - ator->current_pos < alloc_size)) {
		size_t block_alloc_size = ufbxwi_min_sz(ufbxwi_max_sz(ator->next_block_size * 2, 0x10000), 0x100000);

		ufbxwi_alloc *block = ufbxwi_alloc_block(ator, block_alloc_size - sizeof(ufbxwi_alloc));
		ufbxwi_check_return_err(ator->error, block, NULL);
		block->magic = UFBXWI_BLOCK_MAGIC;

		ator->current_block = block;
		ator->current_pos = sizeof(ufbxwi_alloc);
		ator->current_size = block_alloc_size;

		ator->next_block_size = block_alloc_size;
	}

	size_t offset = ator->current_pos;
	ufbxwi_alloc *alloc = (ufbxwi_alloc*)((char*)ator->current_block + offset);
	ator->current_pos = offset + alloc_size;
	alloc->next = NULL;
	alloc->prev = NULL;
	alloc->size = size_class_size;
	alloc->magic = UFBXWI_ALLOC_MAGIC;
	if (p_alloc_size) {
		*p_alloc_size = size_class_size;
	}
	return alloc + 1;
}

static ufbxwi_noinline void ufbxwi_free(ufbxwi_allocator *ator, void *ptr)
{
	if (!ptr || ptr == ufbxwi_zero_size_buffer) return;

	ufbxwi_alloc *alloc = (ufbxwi_alloc*)ptr - 1;
	ufbxw_assert(alloc->magic == UFBXWI_ALLOC_MAGIC || alloc->magic == UFBXWI_HUGE_MAGIC);
	uint32_t size_class = ufbxwi_get_size_class(alloc->size);

	if (size_class == ~0u) {
		ufbxwi_free_block(ator, alloc);
	} else {
		alloc->magic = UFBXWI_FREED_MAGIC;
		alloc->next = ator->free_root[size_class];
		ator->free_root[size_class] = alloc;
	}
}

static ufbxwi_noinline void ufbxwi_move_allocator(ufbxwi_allocator *dst, ufbxwi_allocator *src)
{
	*dst = *src;
	if (dst->block_root.next) {
		dst->block_root.next->prev = &dst->block_root;
	}
	memset(src, 0, sizeof(ufbxwi_allocator));
}

static ufbxwi_noinline void ufbxwi_free_allocator(ufbxwi_allocator *ator)
{
	ufbxwi_alloc *block = ator->block_root.next;
	while (block) {
		ufbxw_assert(block->magic == UFBXWI_BLOCK_MAGIC || block->magic == UFBXWI_HUGE_MAGIC);

		ufbxwi_alloc *next = block->next;
		ufbxwi_free_block(ator, block);
		block = next;
	}

	if (ator->ator.free_allocator_fn) {
		ator->ator.free_allocator_fn(ator->ator.user);
	}
}

#define ufbxwi_alloc(ator, type, n) ufbxwi_maybe_null((type*)ufbxwi_alloc_size((ator), sizeof(type), (n), NULL))

// -- Dynamic List

typedef struct {
	void *data;
	size_t count;
	size_t capacity;
} ufbxwi_list;

#define UFBXWI_LIST_TYPE(p_name, p_type) typedef struct p_name { p_type *data; size_t count, capacity; } p_name

static ufbxwi_noinline void *ufbxwi_list_push_size_slow(ufbxwi_allocator *ator, ufbxwi_list *list, size_t size, size_t n)
{
	size_t count = list->count;
	size_t new_capacity = ufbxwi_max_sz(count + n, list->capacity * 2);

	size_t alloc_size = 0;
	char *new_data = ufbxwi_alloc_size(ator, size, new_capacity, &alloc_size);
	ufbxwi_check_return_err(ator->error, new_data, NULL);

	list->data = new_data;
	list->capacity = alloc_size / size;
	list->count += n;
	return (char*)list->data + size * count;
}

static ufbxwi_forceinline ufbxwi_nodiscard void *ufbxwi_list_push_size(ufbxwi_allocator *ator, void *p_list, size_t size, size_t n)
{
	ufbxwi_list *list = (ufbxwi_list*)p_list;
	size_t count = list->count;
	if (list->capacity - count >= n) {
		list->count += n;
		return (char*)list->data + size * count;
	} else {
		return ufbxwi_list_push_size_slow(ator, list, size, n);
	}
}

static ufbxwi_forceinline ufbxwi_nodiscard void *ufbxwi_list_push_zero_size(ufbxwi_allocator *ator, void *p_list, size_t size, size_t n)
{
	void *data = ufbxwi_list_push_size(ator, p_list, size, n);
	if (!data) return NULL;
	memset(data, 0, size * n);
	return data;
}

static ufbxwi_forceinline ufbxwi_nodiscard void *ufbxwi_list_push_copy_size(ufbxwi_allocator *ator, void *p_list, size_t size, size_t n, void *src)
{
	void *data = ufbxwi_list_push_size(ator, p_list, size, n);
	if (!data) return NULL;
	memcpy(data, src, size * n);
	return data;
}

static ufbxwi_forceinline void ufbxwi_list_free_size(ufbxwi_allocator *ator, void *p_list, size_t size)
{
	ufbxwi_list *list = (ufbxwi_list*)p_list;
	ufbxwi_free(ator, list->data);
	list->data = NULL;
	list->count = list->capacity = 0;
}

#if UFBXW_DEV
	#define ufbxwi_check_ptr_type(type, expr) ((void)(sizeof((type*)NULL - (expr))))
#else
	#define ufbxwi_check_ptr_type(type, expr) ((void)0)
#endif

#define ufbxwi_list_push_uninit(ator, list, type) ufbxwi_maybe_null((ufbxwi_check_ptr_type(type, (list)->data), (type*)ufbxwi_list_push_size((ator), (list), sizeof(type), 1)))
#define ufbxwi_list_push_uninit_n(ator, list, type, n) ufbxwi_maybe_null((ufbxwi_check_ptr_type(type, (list)->data), (type*)ufbxwi_list_push_size((ator), (list), sizeof(type), (n))))
#define ufbxwi_list_push_zero(ator, list, type) ufbxwi_maybe_null((ufbxwi_check_ptr_type(type, (list)->data), (type*)ufbxwi_list_push_zero_size((ator), (list), sizeof(type), 1)))
#define ufbxwi_list_push_zero_n(ator, list, type, n) ufbxwi_maybe_null((ufbxwi_check_ptr_type(type, (list)->data), (type*)ufbxwi_list_push_zero_size((ator), (list), sizeof(type), (n))))
#define ufbxwi_list_push_copy(ator, list, type, src) ufbxwi_maybe_null((ufbxwi_check_ptr_type(type, (list)->data), ufbxwi_check_ptr_type(type, src), (type*)ufbxwi_list_push_copy_size((ator), (list), sizeof(type), 1, (src))))
#define ufbxwi_list_push_copy_n(ator, list, type, n, src) ufbxwi_maybe_null((ufbxwi_check_ptr_type(type, (list)->data), ufbxwi_check_ptr_type(type, src), (type*)ufbxwi_list_push_copy_size((), (list), sizeof(type), (n), (src))))
#define ufbxwi_list_free(ator, list) ufbxwi_list_free_size((ator),&(list), sizeof(*(list)->data))

// -- Scene

#define UFBXWI_ELEMENT_TYPE_NONE ((ufbxw_element_type)0) 

UFBXWI_LIST_TYPE(ufbxwi_uint32_list, uint32_t);

static ufbxwi_forceinline ufbxw_id ufbxwi_make_id(ufbxw_element_type type, uint32_t generation, size_t index)
{
	ufbxw_assert(index < (1u << 24));
	return (ufbxw_id)(((uint64_t)generation << 40) | ((uint64_t)type << 32) | (index));
}

#define ufbxwi_id_index(id) (uint32_t)(id)
#define ufbxwi_id_type(id) (ufbxw_element_type)((id >> 32) & 0xff)
#define ufbxwi_id_generation(id) (uint32_t)(((id) >> 40))

typedef struct {
	const char *name;

} ufbxwi_prop;

UFBXWI_LIST_TYPE(ufbxwi_prop_list, ufbxwi_prop);

typedef struct {
	ufbxw_id id;
	ufbxwi_prop_list props;
	void *data;
} ufbxwi_element;

UFBXWI_LIST_TYPE(ufbxwi_element_list, ufbxwi_element);

typedef struct {
	ufbxw_vec3 lcl_translation;
	ufbxw_vec3 lcl_rotation;
	ufbxw_vec3 lcl_scaling;
} ufbxwi_node;

typedef struct {
	size_t temp;
} ufbxwi_mesh;

struct ufbxw_scene {
	ufbxwi_allocator ator;
	ufbxwi_error error;
	ufbxw_scene_opts opts;

	ufbxwi_element_list elements;
	ufbxwi_uint32_list free_element_ids;
	size_t num_elements;
};

static ufbxwi_forceinline ufbxwi_element *ufbxwi_get_element(ufbxw_scene *scene, ufbxw_id id)
{
	size_t index = ufbxwi_id_index(id);
	if (index >= scene->elements.count) return NULL;
	ufbxwi_element *element = &scene->elements.data[index];
	if (element->id != id) return NULL;
	return element;
}

static size_t ufbxwi_element_data_size(ufbxw_element_type type)
{
	switch (type) {
	case UFBXW_ELEMENT_NODE: return sizeof(ufbxwi_node);
	case UFBXW_ELEMENT_MESH: return sizeof(ufbxwi_mesh);
	default: return 0;
	}
}

// -- API

ufbxw_abi ufbxw_scene *ufbxw_create_scene(const ufbxw_scene_opts *opts)
{
	ufbxw_scene_opts default_opts;
	if (!opts) {
		memset(&default_opts, 0, sizeof(default_opts));
		opts = &default_opts;
	}

	ufbxwi_allocator ator = { 0 };
	ator.ator = opts->allocator;
	ator.max_allocs = SIZE_MAX;
	ator.max_size = SIZE_MAX / 4;

	ufbxw_scene *scene = ufbxwi_alloc(&ator, ufbxw_scene, 1);
	if (!scene) return NULL;

	memset(scene, 0, sizeof(ufbxw_scene));
	scene->opts = *opts;
	ufbxwi_move_allocator(&scene->ator, &ator);

	return scene;
}

ufbxw_abi void ufbxw_free_scene(ufbxw_scene *scene)
{
	ufbxwi_allocator ator;
	ufbxwi_move_allocator(&ator, &scene->ator);
	ufbxwi_free_allocator(&ator);
}

ufbxw_abi bool ufbxw_get_error(ufbxw_scene *scene, ufbxw_error *error)
{
	if (scene->error.failed) {
		if (error) {
			*error = scene->error.error;
		}
		return true;
	} else {
		if (error) {
			memset(error, 0, sizeof(ufbxw_error));
		}
		return false;
	}
}

ufbxw_abi ufbxw_id ufbxw_create_element(ufbxw_scene *scene, ufbxw_element_type type)
{
	size_t data_size = ufbxwi_element_data_size(type);
	void *data = ufbxwi_alloc_size(&scene->ator, data_size, 1, NULL);
	ufbxwi_check_return(data, 0);
	if (data_size > 0) {
		memset(data, 0, data_size);
	}

	size_t index = 0;
	if (scene->free_element_ids.count > 0) {
		index = scene->free_element_ids.data[--scene->free_element_ids.count];
	} else {
		index = scene->elements.count;
		ufbxwi_check_return(ufbxwi_list_push_zero(&scene->ator, &scene->elements, ufbxwi_element), 0);
	}

	ufbxwi_element *element = &scene->elements.data[index];
	uint32_t generation = ufbxwi_id_generation(element->id) + 1;
	element->id = ufbxwi_make_id(type, generation, index);
	element->data = data;
	
	scene->num_elements++;

	return ufbxwi_make_id(type, generation, index);
}

ufbxw_abi void ufbxw_delete_element(ufbxw_scene *scene, ufbxw_id id)
{
	ufbxwi_element *element = ufbxwi_get_element(scene, id);
	ufbxwi_check(element);

	ufbxw_element_type type = ufbxwi_id_type(id);
	ufbxwi_free(&scene->ator, element->data);

	uint32_t generation = ufbxwi_id_generation(element->id);
	memset(element, 0, sizeof(ufbxwi_element));
	element->id = ufbxwi_make_id(UFBXWI_ELEMENT_TYPE_NONE, generation, 0);
	scene->num_elements--;

	if (generation < 0xffff) {
		// We can leak the index safely here if we fail to allocate
		uint32_t index = ufbxwi_id_index(id);
		ufbxwi_ignore(ufbxwi_list_push_copy(&scene->ator, &scene->free_element_ids, uint32_t, &index));
	}
}

ufbxw_abi size_t ufbxw_get_num_elements(ufbxw_scene *scene)
{
	return scene->num_elements;
}

ufbxw_abi size_t ufbxw_get_elements(ufbxw_scene *scene, ufbxw_id *elements, size_t num_elements)
{
	size_t count = 0;
	size_t element_count = scene->elements.count;
	for (size_t index = 0; index < element_count; index++) {
		ufbxwi_element *element = &scene->elements.data[index];
		if (ufbxwi_id_type(element->id) != (ufbxw_element_type)0) {
			if (count >= num_elements) break;
			elements[count++] = element->id;
		}
	}
	return count;
}

ufbxw_abi ufbxw_node ufbxw_create_node(ufbxw_scene *scene)
{
	ufbxw_node node = { ufbxw_create_element(scene, UFBXW_ELEMENT_NODE) };
	return node;
}

ufbxw_abi ufbxw_node ufbxw_as_node(ufbxw_id id) {
	ufbxw_node node = { ufbxwi_id_type(id) == UFBXW_ELEMENT_NODE ? id : 0 };
	return node;
}

ufbxw_abi ufbxw_mesh ufbxw_create_mesh(ufbxw_scene *scene)
{
	ufbxw_mesh mesh = { ufbxw_create_element(scene, UFBXW_ELEMENT_MESH) };
	return mesh;
}

ufbxw_abi ufbxw_mesh ufbxw_as_mesh(ufbxw_id id) {
	ufbxw_mesh mesh = { ufbxwi_id_type(id) == UFBXW_ELEMENT_MESH ? id : 0 };
	return mesh;
}

// -- IO

ufbxw_abi bool ufbxw_save_file(ufbxw_scene *scene, const char *filename, const ufbxw_save_opts *opts, ufbxw_error *error)
{
	return ufbxw_save_file_len(scene, filename, strlen(filename), opts, error);
}

ufbxw_abi bool ufbxw_save_file_len(ufbxw_scene *scene, const char *filenamee, size_t filename_len, const ufbxw_save_opts *opts, ufbxw_error *error)
{

	return true;
}

#endif
