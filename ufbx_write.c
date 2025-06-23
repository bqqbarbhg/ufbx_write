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

#if defined(UFBXW_UBSAN)
	static void ufbxwi_assert_zero(size_t offset) { ufbxw_assert(offset == 0); }
	#define ufbxwi_add_ptr(ptr, offset) ((ptr) ? (ptr) + (offset) : (ufbxwi_assert_zero((size_t)(offset)), (ptr)))
	#define ufbxwi_sub_ptr(ptr, offset) ((ptr) ? (ptr) - (offset) : (ufbxwi_assert_zero((size_t)(offset)), (ptr)))
#else
	#define ufbxwi_add_ptr(ptr, offset) ((ptr) + (offset))
	#define ufbxwi_sub_ptr(ptr, offset) ((ptr) - (offset))
#endif

#if defined(UFBXW_REGRESSION)
	static size_t ufbxwi_to_size(ptrdiff_t delta) {
		ufbxw_assert(delta >= 0);
		return (size_t)delta;
	}
#else
	#define ufbxwi_to_size(delta) ((size_t)(delta))
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

#if defined(UFBXWI_HAS_ATTRIBUTE_ALIGNED)
	#define UFBXWI_HAS_UNALIGNED 1
	#define UFBXWI_HAS_ALIASING 1
	#define ufbxwi_unaligned
	typedef uint16_t __attribute__((aligned(1))) ufbxwi_unaligned_u16;
	typedef uint32_t __attribute__((aligned(1))) ufbxwi_unaligned_u32;
	typedef uint64_t __attribute__((aligned(1))) ufbxwi_unaligned_u64;
	typedef float __attribute__((aligned(1))) ufbxwi_unaligned_f32;
	typedef double __attribute__((aligned(1))) ufbxwi_unaligned_f64;
	typedef uint32_t __attribute__((may_alias)) ufbxwi_aliasing_u32;
#elif !defined(UFBXW_STANDARD_C) && defined(_MSC_VER)
	#define UFBXWI_HAS_UNALIGNED 1
	#if defined(_M_IX86)
		// MSVC seems to assume all pointers are unaligned for x86
		#define ufbxwi_unaligned
	#else
		#define ufbxwi_unaligned __unaligned
	#endif
	typedef uint16_t ufbxwi_unaligned_u16;
	typedef uint32_t ufbxwi_unaligned_u32;
	typedef uint64_t ufbxwi_unaligned_u64;
	typedef float ufbxwi_unaligned_f32;
	typedef double ufbxwi_unaligned_f64;
	// MSVC doesn't have aliasing types in theory, but it works in practice..
	#define UFBXWI_HAS_ALIASING 1
	typedef uint32_t ufbxwi_aliasing_u32;
#endif

#if (defined(UFBXWI_HAS_UNALIGNED) && UFBXW_LITTLE_ENDIAN && !defined(UFBXW_NO_UNALIGNED_LOADS)) || defined(UFBXW_USE_UNALIGNED_LOADS)
	#define ufbxwi_read_u16(ptr) (*(const ufbxwi_unaligned ufbxwi_unaligned_u16*)(ptr))
	#define ufbxwi_read_u32(ptr) (*(const ufbxwi_unaligned ufbxwi_unaligned_u32*)(ptr))
	#define ufbxwi_read_u64(ptr) (*(const ufbxwi_unaligned ufbxwi_unaligned_u64*)(ptr))
	#define ufbxwi_read_f32(ptr) (*(const ufbxwi_unaligned ufbxwi_unaligned_f32*)(ptr))
	#define ufbxwi_read_f64(ptr) (*(const ufbxwi_unaligned ufbxwi_unaligned_f64*)(ptr))
#else
	static ufbxwi_forceinline uint16_t ufbxwi_read_u16(const void *ptr) {
		const char *p = (const char*)ptr;
		return (uint16_t)(
			(unsigned)(uint8_t)p[0] << 0u |
			(unsigned)(uint8_t)p[1] << 8u );
	}
	static ufbxwi_forceinline uint32_t ufbxwi_read_u32(const void *ptr) {
		const char *p = (const char*)ptr;
		return (uint32_t)(
			(unsigned)(uint8_t)p[0] <<  0u |
			(unsigned)(uint8_t)p[1] <<  8u |
			(unsigned)(uint8_t)p[2] << 16u |
			(unsigned)(uint8_t)p[3] << 24u );
	}
	static ufbxwi_forceinline uint64_t ufbxwi_read_u64(const void *ptr) {
		const char *p = (const char*)ptr;
		return (uint64_t)(
			(uint64_t)(uint8_t)p[0] <<  0u |
			(uint64_t)(uint8_t)p[1] <<  8u |
			(uint64_t)(uint8_t)p[2] << 16u |
			(uint64_t)(uint8_t)p[3] << 24u |
			(uint64_t)(uint8_t)p[4] << 32u |
			(uint64_t)(uint8_t)p[5] << 40u |
			(uint64_t)(uint8_t)p[6] << 48u |
			(uint64_t)(uint8_t)p[7] << 56u );
	}
	static ufbxwi_forceinline float ufbxwi_read_f32(const void *ptr) {
		uint32_t u = ufbxwi_read_u32(ptr);
		float f;
		memcpy(&f, &u, 4);
		return f;
	}
	static ufbxwi_forceinline double ufbxwi_read_f64(const void *ptr) {
		uint64_t u = ufbxwi_read_u64(ptr);
		double f;
		memcpy(&f, &u, 8);
		return f;
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
	UFBXWI_SIZE_CLASS_COUNT = UFBXWI_MAX_SIZE_CLASS_LOG2 - UFBXWI_MIN_SIZE_CLASS_LOG2 + 1,
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

	memcpy(new_data, list->data, count * size);

	list->data = new_data;
	list->capacity = alloc_size / size;
	list->count += n;
	return (char*)list->data + size * count;
}

static ufbxwi_forceinline void *ufbxwi_list_push_size(ufbxwi_allocator *ator, void *p_list, size_t size, size_t n)
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

static ufbxwi_forceinline void *ufbxwi_list_push_zero_size(ufbxwi_allocator *ator, void *p_list, size_t size, size_t n)
{
	void *data = ufbxwi_list_push_size(ator, p_list, size, n);
	if (!data) return NULL;
	memset(data, 0, size * n);
	return data;
}

static ufbxwi_forceinline void *ufbxwi_list_push_copy_size(ufbxwi_allocator *ator, void *p_list, size_t size, size_t n, void *src)
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

// -- Special list

UFBXWI_LIST_TYPE(ufbxwi_id_list, ufbxw_id);

static bool ufbxwi_id_list_add(ufbxwi_allocator *ator, void *p_list, ufbxw_id id)
{
	ufbxwi_id_list *list = (ufbxwi_id_list*)p_list;
	ufbxw_id *dst = ufbxwi_list_push_uninit(ator, list, ufbxw_id);
	if (dst) {
		*dst = id;
		return true;
	} else {
		return false;
	}
}

static bool ufbxwi_id_list_remove(ufbxwi_allocator *ator, void *p_list, ufbxw_id id)
{
	ufbxwi_id_list *list = (ufbxwi_id_list*)p_list;
	ufbxw_id *begin = list->data, *dst = begin, *end = begin + list->count;
	for (; dst != end; dst++) {
		if (*dst == id) break;
	}
	if (dst != end) return false;

	--list->count;
	--end;
	for (; dst != end; dst++) {
		dst[0] = dst[1];
	}
	return true;
}

// -- Hash functions

static ufbxwi_noinline uint32_t ufbxwi_hash_string(const char *str, size_t length)
{
	uint32_t hash = (uint32_t)length;
	uint32_t seed = UINT32_C(0x9e3779b9);
	if (length >= 4) {
		do {
			uint32_t word = ufbxwi_read_u32(str);
			hash = ((hash << 5u | hash >> 27u) ^ word) * seed;
			str += 4;
			length -= 4;
		} while (length >= 4);

		uint32_t word = ufbxwi_read_u32(str + length - 4);
		hash = ((hash << 5u | hash >> 27u) ^ word) * seed;
	} else {
		uint32_t word = 0;
		if (length >= 1) word |= (uint32_t)(uint8_t)str[0] << 0;
		if (length >= 2) word |= (uint32_t)(uint8_t)str[1] << 8;
		if (length >= 3) word |= (uint32_t)(uint8_t)str[2] << 16;
		hash = ((hash << 5u | hash >> 27u) ^ word) * seed;
	}
	hash ^= hash >> 16;
	hash *= UINT32_C(0x7feb352d);
	hash ^= hash >> 15;
	return hash;
}

// -- String

static const char ufbxwi_empty_char[] = "";
static const ufbxw_string ufbxwi_empty_string = { ufbxwi_empty_char, 0 };

static ufbxw_string ufbxwi_c_str(const char *str)
{
	if (str == NULL) str = ufbxwi_empty_char;
	ufbxw_string s = { str, strlen(str) };
	return s;
}

typedef struct ufbxwi_string_entry {
	uint32_t hash;
	uint32_t length;
	const char *data;
} ufbxwi_string_entry;

UFBXWI_LIST_TYPE(ufbxwi_string_entry_list, ufbxwi_string_entry);

typedef struct ufbxwi_string_pool {
	ufbxwi_allocator *ator;
	ufbxwi_error *error;

	ufbxwi_string_entry *entries;
	uint32_t entry_count;
	uint32_t entry_capacity;

	char *block_pos;
	char *block_end;
} ufbxwi_string_pool;

bool ufbxwi_string_pool_rehash(ufbxwi_string_pool *pool)
{
	size_t capacity = ufbxwi_max_sz(pool->entry_capacity * 2, 256);
	ufbxwi_string_entry *new_entries = ufbxwi_alloc(pool->ator, ufbxwi_string_entry, capacity);
	ufbxwi_check_return_err(pool->error, new_entries, false);

	memset(new_entries, 0, capacity * sizeof(ufbxwi_string_entry));
	ufbxwi_for(ufbxwi_string_entry, entry, pool->entries, pool->entry_capacity) {
		// TODO: Better hashing
		uint32_t index = entry->hash;
		for (;;) {
			uint32_t slot = index & (capacity - 1);
			if (new_entries[slot].hash == 0) {
				new_entries[slot] = *entry;
				break;
			}
			index++;
		}
	}

	pool->entries = new_entries;
	pool->entry_capacity = (uint32_t)capacity;
	return true;
}

static bool ufbxwi_intern_string(ufbxw_string *dst, ufbxwi_string_pool *pool, const char *str, size_t length)
{
	if (length == 0) {
		dst->data = ufbxwi_empty_char;
		dst->length = 0;
		return true;
	}

	ufbxwi_check_return_err(pool->error, length <= UINT32_MAX / 2, false);

	uint32_t hash = ufbxwi_hash_string(str, length);
	if (hash == 0) hash = 1;

	if (pool->entry_count * 2 >= pool->entry_capacity) {
		ufbxwi_check_return_err(pool->error, ufbxwi_string_pool_rehash(pool), false);
	}

	uint32_t capacity = pool->entry_capacity;
	ufbxwi_string_entry *entries = pool->entries;

	uint32_t index = hash;
	for (;;) {
		uint32_t slot = index & (capacity - 1);
		if (entries[slot].hash == hash && entries[slot].length == length && !memcmp(entries[slot].data, str, length)) {
			dst->data = entries[slot].data;
			dst->length = length;
			return true;
		} else if (entries[slot].hash == 0) {
			break;
		}
		index++;
	}

	char *copy = NULL;
	if (length >= 256) {
		copy = ufbxwi_alloc(pool->ator, char, length + 1);
		ufbxwi_check_return_err(pool->error, copy, false);
	} else {
		if (ufbxwi_to_size(pool->block_end - pool->block_pos) < length + 1) {
			const size_t block_size = 4096;
			char *block = ufbxwi_alloc(pool->ator, char, block_size);
			ufbxwi_check_return_err(pool->error, block, false);

			pool->block_pos = block;
			pool->block_end = block + block_size;
		}

		copy = pool->block_pos;
		pool->block_pos = copy + (length + 1);
	}

	pool->entry_count++;

	ufbxw_assert(copy);
	memcpy(copy, str, length);
	copy[length] = '\0';

	ufbxwi_string_entry *entry = &entries[index & (capacity - 1)];
	entry->data = copy;
	entry->hash = hash;
	entry->length = (uint32_t)length;

	dst->data = copy;
	dst->length = length;
	return true;
}

static bool ufbxwi_intern_string_str(ufbxw_string *dst, ufbxwi_string_pool *pool, ufbxw_string str)
{
	return ufbxwi_intern_string(dst, pool, str.data, str.length);
}

// -- Scene

#define UFBXWI_ELEMENT_TYPE_NONE ((ufbxw_element_type)0) 

UFBXWI_LIST_TYPE(ufbxwi_uint32_list, uint32_t);

static ufbxwi_forceinline ufbxw_id ufbxwi_make_id(ufbxw_element_type type, uint32_t generation, size_t index)
{
	ufbxw_assert(index < (1u << 24));
	return (ufbxw_id)(((uint64_t)type << 48) | ((uint64_t)generation << 32) | (index));
}

#define ufbxwi_id_index(id) (uint32_t)(id)
#define ufbxwi_id_type(id) (ufbxw_element_type)((id >> 48))
#define ufbxwi_id_generation(id) (uint32_t)(((id) >> 32) & 0xffff)

typedef struct {
	ufbxw_string name;
	ufbxw_string type;
	ufbxw_string sub_type;
} ufbxwi_prop_type;

UFBXWI_LIST_TYPE(ufbxwi_prop_type_list, ufbxwi_prop_type);

typedef struct {
	ufbxw_prop prop;

	uint32_t str_length;
	const char *str_value;
} ufbxwi_prop;

UFBXWI_LIST_TYPE(ufbxwi_prop_list, ufbxwi_prop);

typedef struct {
	ufbxwi_prop_list props;
} ufbxwi_props;

typedef struct {
	ufbxw_id id;

	ufbxw_string name;

	ufbxwi_props props;
	void *data;

	// TODO: Replace these with more performant containers
	ufbxwi_id_list connections_src;
	ufbxwi_id_list connections_dst;
} ufbxwi_element;

UFBXWI_LIST_TYPE(ufbxwi_element_list, ufbxwi_element);
UFBXWI_LIST_TYPE(ufbxw_node_list, ufbxw_node);

typedef struct {
	ufbxw_node parent;
	ufbxw_node_list children;

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
	ufbxwi_string_pool string_pool;

	ufbxwi_element_list elements;
	ufbxwi_uint32_list free_element_ids;
	size_t num_elements;

	ufbxwi_prop_type_list prop_types;
};

static ufbxwi_forceinline ufbxwi_element *ufbxwi_get_element(ufbxw_scene *scene, ufbxw_id id)
{
	size_t index = ufbxwi_id_index(id);
	if (index >= scene->elements.count) return NULL;
	ufbxwi_element *element = &scene->elements.data[index];
	if (element->id != id) return NULL;
	return element;
}

static ufbxwi_forceinline void *ufbxwi_get_element_data(ufbxw_scene *scene, ufbxw_id id)
{
	ufbxwi_element *element = ufbxwi_get_element(scene, id);
	return element ? element->data : NULL;
}

static ufbxwi_forceinline ufbxwi_node *ufbxwi_get_node_data(ufbxw_scene *scene, ufbxw_node node) { return (ufbxwi_node*)ufbxwi_get_element_data(scene, node.id); }
static ufbxwi_forceinline ufbxwi_mesh *ufbxwi_get_mesh_data(ufbxw_scene *scene, ufbxw_mesh mesh) { return (ufbxwi_mesh*)ufbxwi_get_element_data(scene, mesh.id); }

static ufbxwi_forceinline ufbxw_node ufbxwi_assert_node(ufbxw_id id) { ufbxw_assert(ufbxwi_id_type(id) == UFBXW_ELEMENT_NODE); ufbxw_node node = { id }; return node; }

static size_t ufbxwi_element_data_size(ufbxw_element_type type)
{
	switch (type) {
	case UFBXW_ELEMENT_NODE: return sizeof(ufbxwi_node);
	case UFBXW_ELEMENT_MESH: return sizeof(ufbxwi_mesh);
	default: return 0;
	}
}

static void ufbxwi_create_defaults(ufbxw_scene *scene)
{
	if (!scene->opts.no_default_scene_info) {
		ufbxw_id id = ufbxw_create_element(scene, UFBXW_ELEMENT_SCENE_INFO);
		ufbxw_set_name(scene, id, "GlobalInfo");

		ufbxw_set_string(scene, id, ufbxw_get_custom_prop_c(scene, "DocumentUrl", "KString", "Url"), "test.fbx");
		ufbxw_set_string(scene, id, ufbxw_get_custom_prop_c(scene, "SrcDocumentUrl", "KString", "Url"), "test.fbx");
	}

	if (!scene->opts.no_default_document) {
		ufbxw_id id = ufbxw_create_element(scene, UFBXW_ELEMENT_DOCUMENT);
	}
}

static void ufbxwi_init_scene(ufbxw_scene *scene)
{
	scene->string_pool.ator = &scene->ator;
	scene->string_pool.error = &scene->error;

	if (scene->opts.no_default_elements) {
		scene->opts.no_default_scene_info = true;
		scene->opts.no_default_document = true;
	}

	size_t num_prop_types = (size_t)UFBXW_PROP_FIRST_USER;
	ufbxwi_check(ufbxwi_list_push_zero_n(&scene->ator, &scene->prop_types, ufbxwi_prop_type, num_prop_types));

	ufbxwi_create_defaults(scene);
}

static void ufbxwi_connect_imp(ufbxw_scene *scene, ufbxwi_element *src, ufbxwi_element *dst)
{
	ufbxw_id src_id = src->id, dst_id = dst->id;
	ufbxw_element_type src_type = ufbxwi_id_type(src_id);
	ufbxw_element_type dst_type = ufbxwi_id_type(dst_id);

	if (dst_type == UFBXW_ELEMENT_NODE) {
		ufbxwi_node *dst_node = (ufbxwi_node*)dst->data;

		if (src_type == UFBXW_ELEMENT_NODE) {
			ufbxwi_node *src_node = (ufbxwi_node*)src->data;

			ufbxwi_id_list_add(&scene->ator, &dst_node->children, src_id);
			src_node->parent = ufbxwi_assert_node(dst_id);
		}
	}
}

static void ufbxwi_disconnect_imp(ufbxw_scene *scene, ufbxwi_element *src, ufbxwi_element *dst)
{
	ufbxw_id src_id = src->id, dst_id = dst->id;
	ufbxw_element_type src_type = ufbxwi_id_type(src_id);
	ufbxw_element_type dst_type = ufbxwi_id_type(dst_id);

	if (dst_type == UFBXW_ELEMENT_NODE) {
		ufbxwi_node *dst_node = (ufbxwi_node*)dst->data;

		if (src_type == UFBXW_ELEMENT_NODE) {
			ufbxwi_node *src_node = (ufbxwi_node*)src->data;

			ufbxwi_id_list_remove(&scene->ator, &dst_node->children, dst_id);
			src_node->parent = ufbxw_null_node;
		}
	}
}

// -- Saving

typedef struct {
	ufbxwi_allocator ator;

	ufbxwi_error error;
	ufbxw_save_opts opts;

	ufbxw_scene *scene;
	ufbxw_write_stream stream;

	uint32_t depth;

	char *buffer;
	char *buffer_pos;
	char *buffer_end;
	size_t direct_write_size;

	uint64_t file_pos;
} ufbxw_save_context;

// -- Writing IO

static ufbxwi_noinline void ufbxwi_write_flush(ufbxw_save_context *sc)
{
	size_t size = ufbxwi_to_size(sc->buffer_pos - sc->buffer);
	if (size == 0) return;

	bool write_ok = sc->stream.write_fn(sc->stream.user, sc->file_pos, sc->buffer, size);
	ufbxwi_check_err(&sc->error, write_ok);

	sc->file_pos += (uint64_t)size;
	sc->buffer_pos = sc->buffer;
}

static ufbxwi_noinline void ufbxwi_write_slow(ufbxw_save_context *sc, const void *data, size_t length)
{
	char *dst = sc->buffer_pos;
	size_t left = ufbxwi_to_size(sc->buffer_end - dst);
	if (left >= length) {
		memcpy(dst, data, length);
		sc->buffer_pos = dst + length;
	} else {
		memcpy(dst, data, left);

		data = (const char*)data + left;
		length -= left;

		ufbxwi_write_flush(sc);

		if (length >= sc->direct_write_size) {
			bool write_ok = sc->stream.write_fn(sc->stream.user, sc->file_pos, data, length);
			ufbxwi_check_err(&sc->error, write_ok);
			sc->file_pos += (uint64_t)length;
			sc->buffer_pos = sc->buffer;
		} else {
			memcpy(sc->buffer_pos, data, length);
			sc->buffer_pos += length;
		}
	}
}

static ufbxwi_forceinline void ufbxwi_write(ufbxw_save_context *sc, const void *data, size_t length)
{
	char *dst = sc->buffer_pos;
	size_t left = ufbxwi_to_size(sc->buffer_end - dst);
	if (left >= length) {
		memcpy(dst, data, length);
		sc->buffer_pos = dst + length;
	} else {
		ufbxwi_write_slow(sc, data, length);
	}
}

static ufbxwi_noinline char *ufbxwi_write_reserve_slow(ufbxw_save_context *sc, size_t length)
{
	ufbxwi_write_flush(sc);
	ufbxw_assert(length < ufbxwi_to_size(sc->buffer_end - sc->buffer_pos));
	return sc->buffer_pos;
}

static ufbxwi_forceinline char *ufbxwi_write_reserve(ufbxw_save_context *sc, size_t length)
{
	char *dst = sc->buffer_pos;
	size_t left = ufbxwi_to_size(sc->buffer_end - dst);
	if (left >= length) {
		return sc->buffer_pos;
	} else {
		return ufbxwi_write_reserve_slow(sc, length);
	}
}

static ufbxwi_forceinline void ufbxwi_write_commit(ufbxw_save_context *sc, size_t length)
{
	ufbxw_assert(length < ufbxwi_to_size(sc->buffer_end - sc->buffer_pos));
	sc->buffer_pos += length;
}

// -- ASCII

static void ufbxwi_ascii_indent(ufbxw_save_context *sc)
{
	size_t indent = ufbxwi_min_sz(sc->depth, 64);
	char *dst = ufbxwi_write_reserve(sc, indent);
	for (size_t i = 0; i < indent; i++) {
		dst[i] = '\t';
	}
	ufbxwi_write_commit(sc, indent);
}

static void ufbxwi_ascii_dom_string(ufbxw_save_context *sc, const char *str, size_t length)
{
	ufbxwi_write(sc, "\"", 1);

	const char *end = str + length;
	for (;;) {
		const char *quote = memchr(str, '"', ufbxwi_to_size(end - str));
		if (!quote) {
			ufbxwi_write(sc, str, ufbxwi_to_size(end - str));
			break;
		} else {
			ufbxwi_write(sc, str, ufbxwi_to_size(quote - str));
			ufbxwi_write(sc, "&quot;", 6);
			str = quote + 1;
		}
	}

	ufbxwi_write(sc, "\"", 1);
}

static void ufbxwi_ascii_dom_write(ufbxw_save_context *sc, const char *tag, const char *fmt, va_list args, bool open)
{
	ufbxwi_ascii_indent(sc);

	ufbxwi_write(sc, tag, strlen(tag));
	ufbxwi_write(sc, ": ", 2);

	for (const char *pf = fmt; *pf; ++pf) {
		if (pf != fmt) {
			ufbxwi_write(sc, ", ", 2);
		}

		switch (*pf) {
		case 'I': {
			char *dst = ufbxwi_write_reserve(sc, 32);
			int len = snprintf(dst, 32, "%d", va_arg(args, int32_t));
			ufbxw_assert(len >= 0);
			ufbxwi_write_commit(sc, (size_t)len);
		} break;
		case 'L': {
			char *dst = ufbxwi_write_reserve(sc, 32);
			int len = snprintf(dst, 32, "%lld", (long long)va_arg(args, int64_t));
			ufbxw_assert(len >= 0);
			ufbxwi_write_commit(sc, (size_t)len);
		} break;
		case 'F': {
			char *dst = ufbxwi_write_reserve(sc, 32);
			int len = snprintf(dst, 32, "%.8g", (float)va_arg(args, double));
			ufbxw_assert(len >= 0);
			ufbxwi_write_commit(sc, (size_t)len);
		} break;
		case 'D': {
			char *dst = ufbxwi_write_reserve(sc, 32);
			int len = snprintf(dst, 32, "%.8g", va_arg(args, double));
			ufbxw_assert(len >= 0);
			ufbxwi_write_commit(sc, (size_t)len);
		} break;
		case 'C': {
			const char *str = va_arg(args, const char*);
			ufbxwi_ascii_dom_string(sc, str, strlen(str));
		} break;
		case 'S': {
			ufbxw_string str = va_arg(args, ufbxw_string);
			ufbxwi_ascii_dom_string(sc, str.data, str.length);
		} break;
		}
	}

	if (open) {
		ufbxwi_write(sc, " {", 2);
	}
	ufbxwi_write(sc, "\n", 1);
}

static void ufbxwi_ascii_dom_close(ufbxw_save_context *sc)
{
	ufbxwi_ascii_indent(sc);
	ufbxwi_write(sc, "}\n", 2);
}

// -- DOM

static void ufbxwi_dom_open(ufbxw_save_context *sc, const char *tag, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	if (sc->opts.ascii) {
		ufbxwi_ascii_dom_write(sc, tag, fmt, args, true);
	} else {
		// TODO
	}

	sc->depth++;

	va_end(args);
}

static void ufbxwi_dom_close(ufbxw_save_context *sc)
{
	sc->depth--;

	if (sc->opts.ascii) {
		ufbxwi_ascii_dom_close(sc);
	} else {
		// TODO
	}
}

static void ufbxwi_dom_value(ufbxw_save_context *sc, const char *tag, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	if (sc->opts.ascii) {
		ufbxwi_ascii_dom_write(sc, tag, fmt, args, false);
	} else {
		// TODO
	}

	va_end(args);
}

enum {
	UFBXWI_SAVE_ELEMENT_NO_ID = 0x1,
};

static void ufbxwi_save_properties(ufbxw_save_context *sc, ufbxwi_props *props)
{
	ufbxw_scene *scene = sc->scene;

	// TODO: Version 6
	ufbxwi_dom_open(sc, "Properties70", "");

	ufbxwi_for_list(ufbxwi_prop, prop, props->props) {
		const ufbxwi_prop_type *type = &scene->prop_types.data[prop->prop];

		// TODO: Other types, etc
		ufbxw_string value = { prop->str_value, prop->str_length };
		ufbxwi_dom_value(sc, "P", "SSSCS", type->name, type->type, type->sub_type, "", value);
	}

	ufbxwi_dom_close(sc);
}

static void ufbxwi_save_element(ufbxw_save_context *sc, ufbxwi_element *element, uint32_t flags)
{
	if (!element) return;

	// TODO: Make this data-driven
	const char *fbx_type = "";
	const char *super_type = NULL;
	const char *sub_type = "";

	ufbxw_id id = element->id;
	ufbxw_element_type type = ufbxwi_id_type(id);

	if (type == UFBXW_ELEMENT_NODE) {
		fbx_type = "Model";
		sub_type = "Mesh"; // ???
	} else if (type == UFBXW_ELEMENT_MESH) {
		fbx_type = "Geometry";
		sub_type = "Mesh";
	} else if (type == UFBXW_ELEMENT_SCENE_INFO) {
		fbx_type = "SceneInfo";
		sub_type = "UserData";
	}

	if (super_type == NULL) super_type = fbx_type;

	// TODO: Dynamic buffer
	char name[256];
	if (sc->opts.ascii) {
		snprintf(name, sizeof(name), "%s::%s", fbx_type, element->name.data);
	} else {
	}

	if (flags & UFBXWI_SAVE_ELEMENT_NO_ID) {
		ufbxwi_dom_open(sc, super_type, "CC", name, sub_type);
	} else {
		ufbxwi_dom_open(sc, super_type, "LCC", (int64_t)id, name, sub_type);
	}

	if (type == UFBXW_ELEMENT_NODE) {
	} else if (type == UFBXW_ELEMENT_MESH) {
	} else if (type == UFBXW_ELEMENT_SCENE_INFO) {
		ufbxwi_dom_value(sc, "Type", "C", "UserData");
		ufbxwi_dom_value(sc, "Version", "I", 100);

		ufbxwi_dom_open(sc, "MetaData", "");
		ufbxwi_dom_value(sc,"Version", "I", 100);
		ufbxwi_dom_value(sc, "Title", "C", "");
		ufbxwi_dom_value(sc, "Subject", "C", "");
		ufbxwi_dom_value(sc, "Author", "C", "");
		ufbxwi_dom_value(sc, "Keywords", "C", "");
		ufbxwi_dom_value(sc, "Revision", "C", "");
		ufbxwi_dom_value(sc, "Comment", "C", "");
		ufbxwi_dom_close(sc);
	}

	ufbxwi_save_properties(sc, &element->props);

	ufbxwi_dom_close(sc);
}

static void ufbxwi_save_timestamp(ufbxw_save_context *sc)
{
	ufbxwi_dom_open(sc, "CreationTimeStamp", "");
	ufbxwi_dom_value(sc, "Version", "I", 1000);
	ufbxwi_dom_value(sc, "Year", "I", 2020);
	ufbxwi_dom_value(sc, "Month", "I", 3);
	ufbxwi_dom_value(sc, "Day", "I", 22);
	ufbxwi_dom_value(sc, "Hour", "I", 0);
	ufbxwi_dom_value(sc, "Minute", "I", 27);
	ufbxwi_dom_value(sc, "Second", "I", 54);
	ufbxwi_dom_value(sc, "Millisecond", "I", 501);
	ufbxwi_dom_close(sc);
}

static void ufbxwi_save_root(ufbxw_save_context *sc)
{
	ufbxw_scene *scene = sc->scene;

	ufbxwi_dom_open(sc, "FBXHeaderExtension", "");
	ufbxwi_dom_value(sc, "FBXHeaderVersion", "I", 1003);
	ufbxwi_dom_value(sc, "FBXVersion", "I", sc->opts.version);

	ufbxwi_save_timestamp(sc);

	ufbxwi_dom_value(sc, "Creator", "C", "ufbx_write (version format TBD)");

	ufbxw_id scene_info_id = ufbxw_get_scene_info_id(scene);
	ufbxwi_element *scene_info = ufbxwi_get_element(scene, scene_info_id);
	ufbxwi_save_element(sc, scene_info, UFBXWI_SAVE_ELEMENT_NO_ID);

	ufbxwi_dom_close(sc);
}

// -- Save root

static bool ufbxwi_save_imp(ufbxw_save_context *sc)
{
	if (sc->opts.version == 0) sc->opts.version = 7500;

	size_t buffer_size = 0x10000;
	sc->buffer = ufbxwi_alloc(&sc->ator, char, buffer_size);
	sc->buffer_pos = sc->buffer;
	sc->buffer_end = sc->buffer + buffer_size;
	sc->direct_write_size = buffer_size / 2;

	if (!sc->buffer) return false;

	ufbxwi_save_root(sc);
	ufbxwi_write_flush(sc);

	return true;
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

	ufbxwi_init_scene(scene);

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
	void *data = NULL;
	if (data_size > 0) {
		data = ufbxwi_alloc_size(&scene->ator, data_size, 1, NULL);
		ufbxwi_check_return(data, 0);
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
	element->name = ufbxwi_empty_string;
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

ufbxw_abi void ufbxw_set_name(ufbxw_scene *scene, ufbxw_id id, const char *name)
{
	ufbxw_set_name_len(scene, id, name, strlen(name));
}

ufbxw_abi void ufbxw_set_name_len(ufbxw_scene *scene, ufbxw_id id, const char *name, size_t name_len)
{
	ufbxwi_element *element = ufbxwi_get_element(scene, id);
	ufbxwi_check(element);

	ufbxwi_intern_string(&element->name, &scene->string_pool, name, name_len);
}

ufbxw_abi ufbxw_string ufbxw_get_name(ufbxw_scene *scene, ufbxw_id id)
{
	ufbxwi_element *element = ufbxwi_get_element(scene, id);
	ufbxwi_check_return(element, ufbxwi_empty_string);
	return element->name;
}

ufbxw_abi void ufbxw_set_string(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop, const char *value)
{
	ufbxw_set_string_len(scene, id, prop, value, strlen(value));
}

ufbxw_abi void ufbxw_set_string_len(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop, const char *value, size_t value_len)
{
	ufbxwi_element *element = ufbxwi_get_element(scene, id);
	ufbxwi_check(element);

	ufbxwi_prop *dst_prop = NULL;
	ufbxwi_for_list(ufbxwi_prop, p, element->props.props) {
		if (p->prop == prop) {
			dst_prop = p;
			break;
		}
	}
	if (dst_prop == NULL) {
		dst_prop = ufbxwi_list_push_zero(&scene->ator, &element->props.props, ufbxwi_prop);
		ufbxwi_check(dst_prop);
		dst_prop->prop = prop;
	}

	ufbxw_string str;
	ufbxwi_check(ufbxwi_intern_string(&str, &scene->string_pool, value, value_len));
	dst_prop->str_length = (uint32_t)str.length;
	dst_prop->str_value = str.data;
}

ufbxw_abi void ufbxw_connect(ufbxw_scene *scene, ufbxw_id src, ufbxw_id dst)
{
	ufbxwi_element *src_elem = ufbxwi_get_element(scene, src);
	ufbxwi_element *dst_elem = ufbxwi_get_element(scene, dst);
	ufbxwi_check(src_elem && dst_elem);

	// Do not allow duplicate connections by default
	ufbxwi_for_list(ufbxw_id, id, src_elem->connections_dst) {
		if (*id == dst) return;
	}

	ufbxwi_id_list_add(&scene->ator, &src_elem->connections_dst, dst);
	ufbxwi_id_list_add(&scene->ator, &dst_elem->connections_src, src);
	ufbxwi_connect_imp(scene, src_elem, dst_elem);
}

ufbxw_abi void ufbxw_connect_multi(ufbxw_scene *scene, ufbxw_id src, ufbxw_id dst)
{
	ufbxwi_element *src_elem = ufbxwi_get_element(scene, src);
	ufbxwi_element *dst_elem = ufbxwi_get_element(scene, dst);
	ufbxwi_check(src_elem && dst_elem);

	ufbxwi_id_list_add(&scene->ator, &src_elem->connections_dst, dst);
	ufbxwi_id_list_add(&scene->ator, &dst_elem->connections_src, src);
	ufbxwi_connect_imp(scene, src_elem, dst_elem);
}

ufbxw_abi void ufbxw_disconnect(ufbxw_scene *scene, ufbxw_id src, ufbxw_id dst)
{
	ufbxwi_element *src_elem = ufbxwi_get_element(scene, src);
	ufbxwi_element *dst_elem = ufbxwi_get_element(scene, dst);
	ufbxwi_check(src_elem && dst_elem);

	if (!ufbxwi_id_list_remove(&scene->ator, &src_elem->connections_dst, dst)) return;
	ufbxwi_id_list_remove(&scene->ator, &src_elem->connections_src, src);
	ufbxwi_disconnect_imp(scene, src_elem, dst_elem);
}

ufbxw_abi void ufbxw_disconnect_dst(ufbxw_scene *scene, ufbxw_id id, ufbxw_element_type type)
{
	ufbxwi_element *src_elem = ufbxwi_get_element(scene, id);
	ufbxwi_check(src_elem);

	ufbxwi_id_list conn_dst = src_elem->connections_dst;
	size_t dst_ix = 0;
	for (size_t src_ix = 0; src_ix < conn_dst.count; src_ix++) {
		ufbxw_id dst_id = conn_dst.data[src_ix];
		if (ufbxwi_id_type(dst_id) == type) {
			if (dst_ix != src_ix) conn_dst.data[dst_ix] = dst_id;
			dst_ix++;

			ufbxwi_element *dst_elem = ufbxwi_get_element(scene, dst_id);
			if (dst_elem) {
				ufbxwi_id_list_remove(&scene->ator, &dst_elem->connections_src, id);
				ufbxwi_disconnect_imp(scene, src_elem, dst_elem);
			}
		}
	}
}

ufbxw_abi void ufbxw_disconnect_src(ufbxw_scene *scene, ufbxw_id id, ufbxw_element_type type)
{
	ufbxwi_element *dst_elem = ufbxwi_get_element(scene, id);
	ufbxwi_check(dst_elem);

	ufbxwi_id_list conn_src = dst_elem->connections_src;
	size_t dst_ix = 0;
	for (size_t src_ix = 0; src_ix < conn_src.count; src_ix++) {
		ufbxw_id src_id = conn_src.data[src_ix];
		if (ufbxwi_id_type(src_id) == type) {
			if (dst_ix != src_ix) conn_src.data[dst_ix] = src_id;
			dst_ix++;

			ufbxwi_element *src_elem = ufbxwi_get_element(scene, src_id);
			if (src_elem) {
				ufbxwi_id_list_remove(&scene->ator, &dst_elem->connections_dst, id);
				ufbxwi_disconnect_imp(scene, src_elem, dst_elem);
			}
		}
	}
}

ufbxw_abi ufbxw_prop ufbxw_get_custom_prop(ufbxw_scene *scene, const ufbxw_custom_prop *prop)
{
	// TODO: Hash the whole thing
	ufbxw_custom_prop p = { 0 };

	bool ok = true;
	ok = ok && ufbxwi_intern_string_str(&p.name, &scene->string_pool, prop->name);
	ok = ok && ufbxwi_intern_string_str(&p.type, &scene->string_pool, prop->type);
	ok = ok && ufbxwi_intern_string_str(&p.sub_type, &scene->string_pool, prop->sub_type);

	if (!ok) return UFBXW_PROP_NONE;

	size_t count = scene->prop_types.count;
	for (size_t i = (size_t)UFBXW_PROP_FIRST_USER; i < count; i++) {
		ufbxwi_prop_type *pt = &scene->prop_types.data[i];
		if (pt->name.data == p.name.data && pt->type.data == p.type.data && pt->sub_type.data == p.sub_type.data) {
			return (ufbxw_prop)i;
		}
	}

	ufbxwi_prop_type *type = ufbxwi_list_push_zero(&scene->ator, &scene->prop_types, ufbxwi_prop_type);
	ufbxwi_check_return(type, UFBXW_PROP_NONE);

	type->name = p.name;
	type->type = p.type;
	type->sub_type = p.sub_type;
	return (ufbxw_prop)count;
}

ufbxw_abi ufbxw_prop ufbxw_get_custom_prop_c(ufbxw_scene *scene, const char *name, const char *type, const char *sub_type)
{
	ufbxw_custom_prop p;
	p.name = ufbxwi_c_str(name);
	p.type = ufbxwi_c_str(type);
	p.sub_type = ufbxwi_c_str(sub_type);
	return ufbxw_get_custom_prop(scene, &p);
}

ufbxw_abi ufbxw_node ufbxw_create_node(ufbxw_scene *scene)
{
	ufbxw_node node = { ufbxw_create_element(scene, UFBXW_ELEMENT_NODE) };
	return node;
}

ufbxw_abi ufbxw_node ufbxw_as_node(ufbxw_id id)
{
	ufbxw_node node = { ufbxwi_id_type(id) == UFBXW_ELEMENT_NODE ? id : 0 };
	return node;
}

ufbxw_abi size_t ufbxw_node_get_num_children(ufbxw_scene *scene, ufbxw_node node)
{
	ufbxwi_node *node_data = ufbxwi_get_node_data(scene, node);
	ufbxwi_check_return(node_data, 0);
	return node_data->children.count;
}

ufbxw_abi ufbxw_node ufbxw_node_get_child(ufbxw_scene *scene, ufbxw_node node, size_t index)
{
	ufbxwi_node *node_data = ufbxwi_get_node_data(scene, node);
	ufbxwi_check_return(node_data, ufbxw_as_node(0));
	ufbxwi_check_return(index < node_data->children.count, ufbxw_as_node(0));
	return node_data->children.data[index];
}

ufbxw_abi void ufbxw_node_set_parent(ufbxw_scene *scene, ufbxw_node node, ufbxw_node parent)
{
	ufbxw_disconnect_dst(scene, node.id, UFBXW_ELEMENT_NODE);
	ufbxw_connect(scene, node.id, parent.id);
}

ufbxw_abi ufbxw_node ufbxw_node_get_parent(ufbxw_scene *scene, ufbxw_node node)
{
	ufbxwi_node *node_data = ufbxwi_get_node_data(scene, node);
	ufbxwi_check_return(node_data, ufbxw_as_node(0));
	return node_data->parent;
}

ufbxw_abi ufbxw_mesh ufbxw_create_mesh(ufbxw_scene *scene)
{
	ufbxw_mesh mesh = { ufbxw_create_element(scene, UFBXW_ELEMENT_MESH) };
	return mesh;
}

ufbxw_abi ufbxw_mesh ufbxw_as_mesh(ufbxw_id id)
{
	ufbxw_mesh mesh = { ufbxwi_id_type(id) == UFBXW_ELEMENT_MESH ? id : 0 };
	return mesh;
}

ufbxw_abi void ufbxw_mesh_add_instance(ufbxw_scene *scene, ufbxw_mesh mesh, ufbxw_node node)
{
	ufbxw_connect(scene, mesh.id, node.id);
}

ufbxw_abi ufbxw_id ufbxw_get_scene_info_id(ufbxw_scene *scene)
{
	ufbxwi_for_list(ufbxwi_element, element, scene->elements) {
		if (ufbxwi_id_type(element->id) == UFBXW_ELEMENT_SCENE_INFO) {
			return element->id;
		}
	}
	return ufbxw_null_id;
}

// -- Streams

static bool ufbxwi_stdio_write(void *user, uint64_t offset, const void *data, size_t size)
{
	// TODO: Do not seek all the time, support >4GB files
	FILE *f = (FILE*)user;
	if (fseek(f, SEEK_SET, (int)offset)) return false;
	if (fwrite(data, 1, size, f) != size) return false;
	return true;
}

static void ufbxwi_stdio_close(void *user)
{
	FILE *f = (FILE*)user;
	fclose(f);
}

ufbxw_abi bool ufbxw_open_file_write(ufbxw_write_stream *stream, const char *path, size_t path_len)
{
	if (path_len >= 1023) return false;

	// TODO: Do this properly
	char copy[1024];
	memcpy(copy, path, path_len);
	copy[path_len] = '\0';

	FILE *f = fopen(copy, "wb");
	if (!f) return false;

	stream->write_fn = ufbxwi_stdio_write;
	stream->close_fn = ufbxwi_stdio_close;
	stream->user = f;

	return true;
}

// -- IO

ufbxw_abi bool ufbxw_save_file(ufbxw_scene *scene, const char *path, const ufbxw_save_opts *opts, ufbxw_error *error)
{
	return ufbxw_save_file_len(scene, path, strlen(path), opts, error);
}

ufbxw_abi bool ufbxw_save_file_len(ufbxw_scene *scene, const char *path, size_t path_len, const ufbxw_save_opts *opts, ufbxw_error *error)
{
	ufbxw_write_stream stream;
	if (!ufbxw_open_file_write(&stream, path, path_len)) {
		return false;
	}

	return ufbxw_save_stream(scene, &stream, opts, error);
}

ufbxw_abi bool ufbxw_save_stream(ufbxw_scene *scene, ufbxw_write_stream *stream, const ufbxw_save_opts *opts, ufbxw_error *error)
{
	ufbxw_save_context sc = { 0 };
	sc.scene = scene;
	sc.stream = *stream;
	if (opts) {
		sc.opts = *opts;
	}

	bool ok = ufbxwi_save_imp(&sc);

	ufbxwi_free_allocator(&sc.ator);

	if (!ok) {
		return false;
	}

	return true;
}

#endif
