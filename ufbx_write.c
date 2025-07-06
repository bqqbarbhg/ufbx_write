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

#define ufbxwi_unreachable(reason) do { ufbxw_assert(0 && reason); } while (0)

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

#if !defined(ufbxw_static_assert)
	#if defined(__cplusplus) && __cplusplus >= 201103
		#define ufbxw_static_assert(desc, cond) static_assert(cond, #desc ": " #cond)
	#else
		#define ufbxw_static_assert(desc, cond) typedef char ufbxwi_static_assert_##desc[(cond)?1:-1]
	#endif
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

static ufbxwi_forceinline uint32_t ufbxwi_min_u32(uint32_t a, uint32_t b) { return a < b ? a : b; }
static ufbxwi_forceinline uint32_t ufbxwi_max_u32(uint32_t a, uint32_t b) { return a < b ? b : a; }
static ufbxwi_forceinline size_t ufbxwi_min_sz(size_t a, size_t b) { return a < b ? a : b; }
static ufbxwi_forceinline size_t ufbxwi_max_sz(size_t a, size_t b) { return a < b ? b : a; }

#define ufbxwi_arraycount(arr) (sizeof(arr) / sizeof(*(arr)))
#define ufbxwi_for(m_type, m_name, m_begin, m_num) for (m_type *m_name = m_begin, *m_name##_end = ufbxwi_add_ptr(m_name, m_num); m_name != m_name##_end; m_name++)
#define ufbxwi_for_ptr(m_type, m_name, m_begin, m_num) for (m_type **m_name = m_begin, **m_name##_end = ufbxwi_add_ptr(m_name, m_num); m_name != m_name##_end; m_name++)

// WARNING: Evaluates `m_list` twice!
#define ufbxwi_for_list(m_type, m_name, m_list) for (m_type *m_name = (m_list).data, *m_name##_end = ufbxwi_add_ptr(m_name, (m_list).count); m_name != m_name##_end; m_name++)
#define ufbxwi_for_ptr_list(m_type, m_name, m_list) for (m_type **m_name = (m_list).data, **m_name##_end = ufbxwi_add_ptr(m_name, (m_list).count); m_name != m_name##_end; m_name++)

#define ufbxwi_array_list(arr) { arr, ufbxwi_arraycount(arr) }

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

static ufbxwi_forceinline size_t ufbxwi_align(size_t value, size_t align)
{
	return value + (((size_t)0 - value) & (align - 1));
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
	if (n == 0) {
		if (p_alloc_size) *p_alloc_size = 0;
		return (void*)ufbxwi_zero_size_buffer;
	}

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

static ufbxwi_noinline bool ufbxwi_list_resize_size_slow(ufbxwi_allocator *ator, ufbxwi_list *list, size_t size, size_t n)
{
	size_t new_capacity = ufbxwi_max_sz(n, list->capacity * 2);

	size_t alloc_size = 0;
	char *new_data = ufbxwi_alloc_size(ator, size, new_capacity, &alloc_size);
	ufbxwi_check_return_err(ator->error, new_data, false);

	memcpy(new_data, list->data, list->count * size);

	list->data = new_data;
	list->capacity = alloc_size / size;
	list->count = n;
	return true;
}

static ufbxwi_forceinline bool ufbxwi_list_resize_size(ufbxwi_allocator *ator, void *p_list, size_t size, size_t n)
{
	ufbxwi_list *list = (ufbxwi_list*)p_list;
	if (list->capacity >= n) {
		list->count = n;
		return true;
	} else {
		return ufbxwi_list_resize_size_slow(ator, list, size, n);
	}
}

static ufbxwi_forceinline void *ufbxwi_list_push_zero_size(ufbxwi_allocator *ator, void *p_list, size_t size, size_t n)
{
	void *data = ufbxwi_list_push_size(ator, p_list, size, n);
	if (!data) return NULL;
	memset(data, 0, size * n);
	return data;
}

static ufbxwi_forceinline void *ufbxwi_list_push_copy_size(ufbxwi_allocator *ator, void *p_list, size_t size, size_t n, const void *src)
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
#define ufbxwi_list_push_copy_n(ator, list, type, n, src) ufbxwi_maybe_null((ufbxwi_check_ptr_type(type, (list)->data), ufbxwi_check_ptr_type(type, src), (type*)ufbxwi_list_push_copy_size((ator), (list), sizeof(type), (n), (src))))
#define ufbxwi_list_resize_uninit(ator, list, type, n) (ufbxwi_check_ptr_type(type, (list)->data), (type*)ufbxwi_list_resize_size((ator), (list), sizeof(type), (n)))
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

	// Reserve 0 and 1 for empty/tombstone
	return hash >= 2 ? hash : 2;
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
	uint32_t token;
	const char *data;
} ufbxwi_string_entry;

UFBXWI_LIST_TYPE(ufbxw_string_list, ufbxw_string);

typedef struct ufbxwi_string_pool {
	ufbxwi_allocator *ator;
	ufbxwi_error *error;

	ufbxwi_string_entry *entries;
	uint32_t entry_count;
	uint32_t entry_capacity;

	ufbxw_string_list tokens;

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

static char *ufbxwi_copy_string(ufbxwi_string_pool *pool, const char *str, size_t length)
{
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

	ufbxw_assert(copy);
	memcpy(copy, str, length);
	copy[length] = '\0';

	return copy;
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

	char *copy = ufbxwi_copy_string(pool, str, length);
	ufbxwi_check_return_err(pool->error, copy, false);

	pool->entry_count++;

	ufbxwi_string_entry *entry = &entries[index & (capacity - 1)];
	entry->data = copy;
	entry->hash = hash;
	entry->token = 0;
	entry->length = (uint32_t)length;

	dst->data = copy;
	dst->length = length;
	return true;
}

static uint32_t ufbxwi_intern_token(ufbxwi_string_pool *pool, const char *str, size_t length)
{
	if (length == 0) return 1; // UFBXWI_TOKEN_EMPTY

	ufbxwi_check_return_err(pool->error, length <= UINT32_MAX / 2, 0);

	uint32_t hash = ufbxwi_hash_string(str, length);

	if (pool->entry_count * 2 >= pool->entry_capacity) {
		ufbxwi_check_return_err(pool->error, ufbxwi_string_pool_rehash(pool), 0);
	}

	uint32_t capacity = pool->entry_capacity;
	ufbxwi_string_entry *entries = pool->entries;

	uint32_t index = hash;
	for (;;) {
		uint32_t slot = index & (capacity - 1);
		if (entries[slot].hash == hash && entries[slot].length == length && !memcmp(entries[slot].data, str, length)) {
			if (entries[slot].token > 0) {
				return entries[slot].token;
			} else {
				uint32_t token = (uint32_t)pool->tokens.count;
				ufbxw_string *dst = ufbxwi_list_push_uninit(pool->ator, &pool->tokens, ufbxw_string);
				ufbxwi_check_return_err(pool->error, dst, 0);
				dst->data = entries[slot].data;
				dst->length = length;
				entries[slot].token = token;
				return token;
			}
		} else if (entries[slot].hash == 0) {
			break;
		}
		index++;
	}

	char *copy = ufbxwi_copy_string(pool, str, length);
	ufbxwi_check_return_err(pool->error, copy, 0);

	pool->entry_count++;

	uint32_t token = (uint32_t)pool->tokens.count;
	ufbxw_string *dst = ufbxwi_list_push_uninit(pool->ator, &pool->tokens, ufbxw_string);
	ufbxwi_check_return_err(pool->error, dst, 0);
	dst->data = copy;
	dst->length = length;

	ufbxwi_string_entry *entry = &entries[index & (capacity - 1)];
	entry->data = copy;
	entry->hash = hash;
	entry->token = token;
	entry->length = (uint32_t)length;
	return token;
}

static uint32_t ufbxwi_get_token(const ufbxwi_string_pool *pool, const char *str, size_t length)
{
	if (length == 0) return 1; // UFBXWI_TOKEN_EMPTY

	ufbxwi_check_return_err(pool->error, length <= UINT32_MAX / 2, 0);

	uint32_t hash = ufbxwi_hash_string(str, length);
	uint32_t capacity = pool->entry_capacity;
	const ufbxwi_string_entry *entries = pool->entries;

	uint32_t index = hash;
	for (;;) {
		uint32_t slot = index & (capacity - 1);
		if (entries[slot].hash == hash && entries[slot].length == length && !memcmp(entries[slot].data, str, length)) {
			return entries[slot].token;
		} else if (entries[slot].hash == 0) {
			return 0;
		}
		index++;
	}
}

static bool ufbxwi_intern_string_str(ufbxw_string *dst, ufbxwi_string_pool *pool, ufbxw_string str)
{
	return ufbxwi_intern_string(dst, pool, str.data, str.length);
}

typedef enum ufbxwi_token {
	UFBXWI_TOKEN_NONE,
	UFBXWI_TOKEN_EMPTY,
	UFBXWI_ActiveAnimStackName,
	UFBXWI_AmbientColor,
	UFBXWI_AmbientFactor,
	UFBXWI_AnimationCurve,
	UFBXWI_AnimationCurveNode,
	UFBXWI_AnimationLayer,
	UFBXWI_AnimationStack,
	UFBXWI_AxisLen,
	UFBXWI_BBoxMax,
	UFBXWI_BBoxMin,
	UFBXWI_Bump,
	UFBXWI_BumpFactor,
	UFBXWI_Casts_Shadows,
	UFBXWI_Color,
	UFBXWI_DefaultAttributeIndex,
	UFBXWI_DiffuseColor,
	UFBXWI_DiffuseFactor,
	UFBXWI_DisplacementColor,
	UFBXWI_DisplacementFactor,
	UFBXWI_Document,
	UFBXWI_EmissiveColor,
	UFBXWI_EmissiveFactor,
	UFBXWI_FbxMesh,
	UFBXWI_FbxNode,
	UFBXWI_FbxSurfaceLambert,
	UFBXWI_Freeze,
	UFBXWI_GeometricRotation,
	UFBXWI_GeometricScaling,
	UFBXWI_GeometricTranslation,
	UFBXWI_Geometry,
	UFBXWI_GlobalSettings,
	UFBXWI_InheritType,
	UFBXWI_LODBox,
	UFBXWI_Lcl_Rotation,
	UFBXWI_Lcl_Scaling,
	UFBXWI_Lcl_Translation,
	UFBXWI_LookAtProperty,
	UFBXWI_Material,
	UFBXWI_MaxDampRangeX,
	UFBXWI_MaxDampRangeY,
	UFBXWI_MaxDampRangeZ,
	UFBXWI_MaxDampStrengthX,
	UFBXWI_MaxDampStrengthY,
	UFBXWI_MaxDampStrengthZ,
	UFBXWI_Mesh,
	UFBXWI_MinDampRangeX,
	UFBXWI_MinDampRangeY,
	UFBXWI_MinDampRangeZ,
	UFBXWI_MinDampStrengthX,
	UFBXWI_MinDampStrengthY,
	UFBXWI_MinDampStrengthZ,
	UFBXWI_Model,
	UFBXWI_MultiLayer,
	UFBXWI_NegativePercentShapeSupport,
	UFBXWI_NodeAttribute,
	UFBXWI_NormalMap,
	UFBXWI_PostRotation,
	UFBXWI_PreRotation,
	UFBXWI_PreferedAngleX,
	UFBXWI_PreferedAngleY,
	UFBXWI_PreferedAngleZ,
	UFBXWI_Primary_Visibility,
	UFBXWI_QuaternionInterpolate,
	UFBXWI_Receive_Shadows,
	UFBXWI_RotationActive,
	UFBXWI_RotationMax,
	UFBXWI_RotationMaxX,
	UFBXWI_RotationMaxY,
	UFBXWI_RotationMaxZ,
	UFBXWI_RotationMin,
	UFBXWI_RotationMinX,
	UFBXWI_RotationMinY,
	UFBXWI_RotationMinZ,
	UFBXWI_RotationOffset,
	UFBXWI_RotationOrder,
	UFBXWI_RotationPivot,
	UFBXWI_RotationSpaceForLimitOnly,
	UFBXWI_RotationStiffnessX,
	UFBXWI_RotationStiffnessY,
	UFBXWI_RotationStiffnessZ,
	UFBXWI_ScalingActive,
	UFBXWI_ScalingMax,
	UFBXWI_ScalingMaxX,
	UFBXWI_ScalingMaxY,
	UFBXWI_ScalingMaxZ,
	UFBXWI_ScalingMin,
	UFBXWI_ScalingMinX,
	UFBXWI_ScalingMinY,
	UFBXWI_ScalingMinZ,
	UFBXWI_ScalingOffset,
	UFBXWI_ScalingPivot,
	UFBXWI_SceneInfo,
	UFBXWI_ShadingModel,
	UFBXWI_Show,
	UFBXWI_SourceObject,
	UFBXWI_TranslationActive,
	UFBXWI_TranslationMax,
	UFBXWI_TranslationMaxX,
	UFBXWI_TranslationMaxY,
	UFBXWI_TranslationMaxZ,
	UFBXWI_TranslationMin,
	UFBXWI_TranslationMinX,
	UFBXWI_TranslationMinY,
	UFBXWI_TranslationMinZ,
	UFBXWI_TransparencyFactor,
	UFBXWI_TransparentColor,
	UFBXWI_UpVectorProperty,
	UFBXWI_UserData,
	UFBXWI_VectorDisplacementColor,
	UFBXWI_VectorDisplacementFactor,
	UFBXWI_Visibility,
	UFBXWI_Visibility_Inheritance,
	UFBXWI_TOKEN_COUNT,
	UFBXWI_TOKEN_FORCE_32BIT = 0x7fffffff,
} ufbxwi_token;

static const ufbxw_string ufbxwi_tokens[] = {
	{ "", 0 },
	{ "ActiveAnimStackName", 19 },
	{ "AmbientColor", 12 },
	{ "AmbientFactor", 13 },
	{ "AnimationCurve", 14 },
	{ "AnimationCurveNode", 18 },
	{ "AnimationLayer", 14 },
	{ "AnimationStack", 14 },
	{ "AxisLen", 7 },
	{ "BBoxMax", 7 },
	{ "BBoxMin", 7 },
	{ "Bump", 4 },
	{ "BumpFactor", 10 },
	{ "Casts Shadows", 13 },
	{ "Color", 5 },
	{ "DefaultAttributeIndex", 21 },
	{ "DiffuseColor", 12 },
	{ "DiffuseFactor", 13 },
	{ "DisplacementColor", 17 },
	{ "DisplacementFactor", 18 },
	{ "Document", 8 },
	{ "EmissiveColor", 13 },
	{ "EmissiveFactor", 14 },
	{ "FbxMesh", 7 },
	{ "FbxNode", 7 },
	{ "FbxSurfaceLambert", 17 },
	{ "Freeze", 6 },
	{ "GeometricRotation", 17 },
	{ "GeometricScaling", 16 },
	{ "GeometricTranslation", 20 },
	{ "Geometry", 8 },
	{ "GlobalSettings", 14 },
	{ "InheritType", 11 },
	{ "LODBox", 6 },
	{ "Lcl Rotation", 12 },
	{ "Lcl Scaling", 11 },
	{ "Lcl Translation", 15 },
	{ "LookAtProperty", 14 },
	{ "Material", 8 },
	{ "MaxDampRangeX", 13 },
	{ "MaxDampRangeY", 13 },
	{ "MaxDampRangeZ", 13 },
	{ "MaxDampStrengthX", 16 },
	{ "MaxDampStrengthY", 16 },
	{ "MaxDampStrengthZ", 16 },
	{ "Mesh", 4 },
	{ "MinDampRangeX", 13 },
	{ "MinDampRangeY", 13 },
	{ "MinDampRangeZ", 13 },
	{ "MinDampStrengthX", 16 },
	{ "MinDampStrengthY", 16 },
	{ "MinDampStrengthZ", 16 },
	{ "Model", 5 },
	{ "MultiLayer", 10 },
	{ "NegativePercentShapeSupport", 27 },
	{ "NodeAttribute", 13 },
	{ "NormalMap", 9 },
	{ "PostRotation", 12 },
	{ "PreRotation", 11 },
	{ "PreferedAngleX", 14 },
	{ "PreferedAngleY", 14 },
	{ "PreferedAngleZ", 14 },
	{ "Primary Visibility", 18 },
	{ "QuaternionInterpolate", 21 },
	{ "Receive Shadows", 15 },
	{ "RotationActive", 14 },
	{ "RotationMax", 11 },
	{ "RotationMaxX", 12 },
	{ "RotationMaxY", 12 },
	{ "RotationMaxZ", 12 },
	{ "RotationMin", 11 },
	{ "RotationMinX", 12 },
	{ "RotationMinY", 12 },
	{ "RotationMinZ", 12 },
	{ "RotationOffset", 14 },
	{ "RotationOrder", 13 },
	{ "RotationPivot", 13 },
	{ "RotationSpaceForLimitOnly", 25 },
	{ "RotationStiffnessX", 18 },
	{ "RotationStiffnessY", 18 },
	{ "RotationStiffnessZ", 18 },
	{ "ScalingActive", 13 },
	{ "ScalingMax", 10 },
	{ "ScalingMaxX", 11 },
	{ "ScalingMaxY", 11 },
	{ "ScalingMaxZ", 11 },
	{ "ScalingMin", 10 },
	{ "ScalingMinX", 11 },
	{ "ScalingMinY", 11 },
	{ "ScalingMinZ", 11 },
	{ "ScalingOffset", 13 },
	{ "ScalingPivot", 12 },
	{ "SceneInfo", 9 },
	{ "ShadingModel", 12 },
	{ "Show", 4 },
	{ "SourceObject", 12 },
	{ "TranslationActive", 17 },
	{ "TranslationMax", 14 },
	{ "TranslationMaxX", 15 },
	{ "TranslationMaxY", 15 },
	{ "TranslationMaxZ", 15 },
	{ "TranslationMin", 14 },
	{ "TranslationMinX", 15 },
	{ "TranslationMinY", 15 },
	{ "TranslationMinZ", 15 },
	{ "TransparencyFactor", 18 },
	{ "TransparentColor", 16 },
	{ "UpVectorProperty", 16 },
	{ "UserData", 8 },
	{ "VectorDisplacementColor", 23 },
	{ "VectorDisplacementFactor", 24 },
	{ "Visibility", 10 },
	{ "Visibility Inheritance", 22 },
};

// Not including none
ufbxw_static_assert(ufbxwi_tokens_count, ufbxwi_arraycount(ufbxwi_tokens) == UFBXWI_TOKEN_COUNT - 1);

uint32_t ufbxwi_hash_token(ufbxwi_token token)
{
	uint32_t x = (uint32_t)token;
	x ^= x >> 16;
	x *= 0x7feb352d;
	x ^= x >> 16;
	return x;
}

// -- Prop types

static const ufbxw_vec3 ufbxwi_one_vec3 = { 1.0f, 1.0f, 1.0f };

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
	const char *name;
	ufbxw_prop_type type;
	uint32_t flags;
} ufbxwi_prop_info;

typedef struct {
	ufbxw_string type;
	ufbxw_string sub_type;
	ufbxw_prop_data_type data_type;
} ufbxwi_prop_type;

UFBXWI_LIST_TYPE(ufbxwi_prop_type_list, ufbxwi_prop_type);

enum {
	UFBXWI_PROP_FLAG_TEMP_VALUE = 0x0 << 24,
	UFBXWI_PROP_FLAG_TEMP_DEFAULT = 0x1 << 24,
	UFBXWI_PROP_FLAG_TEMP_TEMPLATE = 0x2 << 24,
	UFBXWI_PROP_FLAG_TEMP_MASK = 0x3 << 24,
};

typedef struct {
	ufbxwi_token token;
	uint16_t type;
	uint16_t order;
	int32_t value_offset;
	uint32_t flags;
} ufbxwi_prop;

UFBXWI_LIST_TYPE(ufbxwi_prop_list, ufbxwi_prop);

typedef struct {
	ufbxwi_prop *props;
	uint32_t capacity;
	uint32_t count;
	uint32_t order_counter;
} ufbxwi_props;

typedef struct {
	ufbxwi_token type;
} ufbxwi_object_type;

UFBXWI_LIST_TYPE(ufbxwi_object_type_list, ufbxwi_object_type);

typedef void ufbxwi_element_init_data_fn(void *data);

typedef struct {
	ufbxw_element_type element_type;

	ufbxwi_token class_type;
	ufbxwi_token sub_type;
	ufbxwi_token object_type;
	ufbxwi_token fbx_type;

	uint32_t object_type_id;
	ufbxwi_props props;
	ufbxw_id template_id;
	uint32_t flags;
	ufbxwi_element_init_data_fn *init_fn;
} ufbxwi_element_type;

UFBXWI_LIST_TYPE(ufbxwi_element_type_list, ufbxwi_element_type);

enum {
	UFBXWI_ELEMENT_FLAG_HAS_DEFAULT_PROPS = 0x1,
	UFBXWI_ELEMENT_FLAG_ALLOW_NO_OBJECT_ID = 0x2,
};

typedef struct {
	ufbxw_id id;
	ufbxw_string name;

	ufbxwi_props props;
	void *data;
	size_t data_capacity;
	size_t data_size;

	uint32_t type_id;
	uint32_t flags;

	// TODO: Replace these with more performant containers
	ufbxwi_id_list connections_src;
	ufbxwi_id_list connections_dst;
} ufbxwi_element;

UFBXWI_LIST_TYPE(ufbxwi_element_list, ufbxwi_element);
UFBXWI_LIST_TYPE(ufbxw_node_list, ufbxw_node);

typedef struct {
	ufbxw_id id;
} ufbxwi_element_data;

typedef struct {
	ufbxwi_element_data element;

	ufbxw_node parent;
	ufbxw_node_list children;

	ufbxw_vec3 lcl_translation;
	ufbxw_vec3 lcl_rotation;
	ufbxw_vec3 lcl_scaling;
	ufbxw_vec3 pre_rotation;
	ufbxw_vec3 post_rotation;
	ufbxw_vec3 rotation_offset;
	ufbxw_vec3 rotation_pivot;
	ufbxw_vec3 scaling_offset;
	ufbxw_vec3 scaling_pivot;

	ufbxw_vec3 geometric_translation;
	ufbxw_vec3 geometric_rotation;
	ufbxw_vec3 geometric_scaling;

	int32_t rotation_order;
	int32_t inherit_type;
	bool visibility;
	bool visibility_inheritance;
} ufbxwi_node;

typedef struct {
	ufbxwi_element_data element;
} ufbxwi_mesh;

typedef struct {
	ufbxwi_element_data element;
} ufbxwi_material;

typedef struct {
	ufbxwi_element_data element;
	ufbxwi_token type;
} ufbxwi_template;

typedef struct {
	ufbxwi_element_data element;
	ufbxw_id root_node;
} ufbxwi_document;

struct ufbxw_scene {
	ufbxwi_allocator ator;
	ufbxwi_error error;
	ufbxw_scene_opts opts;
	ufbxwi_string_pool string_pool;

	ufbxwi_element_list elements;
	ufbxwi_uint32_list free_element_ids;
	size_t num_elements;

	ufbxwi_object_type_list object_types;
	ufbxwi_element_type_list element_types;
	ufbxwi_prop_type_list prop_types;
};

#define ufbxwi_max_size(a, b) (sizeof(a) > sizeof(b) ? sizeof(a) : sizeof(b))

typedef struct {
	uint8_t size;
	uint8_t alignment;
} ufbxwi_prop_data_info;

typedef struct {
	const char *type;
	const char *sub_type;
	ufbxw_prop_data_type data_type;
} ufbxwi_prop_type_desc;

static const ufbxwi_prop_data_info ufbxwi_prop_data_infos[] = {
	{ 0, 0 },
	{ sizeof(bool), sizeof(bool) },
	{ sizeof(int32_t), sizeof(int32_t) },
	{ sizeof(int64_t), sizeof(int64_t) },
	{ sizeof(ufbxw_real), sizeof(ufbxw_real) },
	{ sizeof(ufbxw_vec2), sizeof(ufbxw_real) },
	{ sizeof(ufbxw_vec3), sizeof(ufbxw_real) },
	{ sizeof(ufbxw_vec4), sizeof(ufbxw_real) },
	{ sizeof(ufbxw_string), ufbxwi_max_size(char*, size_t) },
	{ sizeof(ufbxw_id), sizeof(ufbxw_id) },
	{ sizeof(ufbxw_real_string), ufbxwi_max_size(ufbxw_real, char*) },
	{ sizeof(ufbxw_blob), ufbxwi_max_size(void*, size_t) },
	{ sizeof(ufbxw_user_int), sizeof(int32_t) },
	{ sizeof(ufbxw_user_real), sizeof(ufbxw_real) },
	{ sizeof(ufbxw_user_enum), sizeof(int32_t) },
};

ufbxw_static_assert(ufbxwi_prop_data_type_count, ufbxwi_arraycount(ufbxwi_prop_data_infos) == UFBXW_PROP_DATA_TYPE_COUNT);

static const ufbxwi_prop_type_desc ufbxwi_prop_types[] = {
	{ "", "", UFBXW_PROP_DATA_NONE },
	{ "Compound", "", UFBXW_PROP_DATA_NONE },
	{ "bool", "", UFBXW_PROP_DATA_BOOL },
	{ "enum", "", UFBXW_PROP_DATA_INT32 },
	{ "int", "Integer", UFBXW_PROP_DATA_INT32 },
	{ "KTime", "Time", UFBXW_PROP_DATA_INT64 },
	{ "UByte", "", UFBXW_PROP_DATA_INT64 },
	{ "ULongLong", "", UFBXW_PROP_DATA_INT64 },
	{ "float", "", UFBXW_PROP_DATA_REAL },
	{ "double", "Number", UFBXW_PROP_DATA_REAL },
	{ "Number", "", UFBXW_PROP_DATA_REAL },
	{ "Vector2D", "Vector2", UFBXW_PROP_DATA_VEC2 },
	{ "Vector", "", UFBXW_PROP_DATA_VEC3 },
	{ "Vector3D", "Vector", UFBXW_PROP_DATA_VEC3 },
	{ "Color", "", UFBXW_PROP_DATA_VEC3 },
	{ "ColorRGB", "Color", UFBXW_PROP_DATA_VEC3 },
	{ "ColorAndAlpha", "", UFBXW_PROP_DATA_VEC4 },
	{ "KString", "", UFBXW_PROP_DATA_STRING },
	{ "KString", "Url", UFBXW_PROP_DATA_STRING },
	{ "KString", "XRefUrl", UFBXW_PROP_DATA_STRING },
	{ "DateTime", "", UFBXW_PROP_DATA_STRING },
	{ "object", "", UFBXW_PROP_DATA_ID },
	{ "Distance", "", UFBXW_PROP_DATA_REAL_STRING },
	{ "Blob", "", UFBXW_PROP_DATA_BLOB },
	{ "Bool", "", UFBXW_PROP_DATA_BOOL },
	{ "Vector", "", UFBXW_PROP_DATA_VEC3 },
	{ "Integer", "", UFBXW_PROP_DATA_USER_INT },
	{ "Number", "", UFBXW_PROP_DATA_USER_REAL },
	{ "KString", "", UFBXW_PROP_DATA_STRING },
	{ "Enum", "", UFBXW_PROP_DATA_USER_ENUM },
	{ "Visibility", "", UFBXW_PROP_DATA_BOOL },
	{ "Visibility Inheritance", "", UFBXW_PROP_DATA_BOOL },
	{ "Roll", "", UFBXW_PROP_DATA_REAL },
	{ "OpticalCenterX", "", UFBXW_PROP_DATA_REAL },
	{ "OpticalCenterY", "", UFBXW_PROP_DATA_REAL },
	{ "FieldOfViewX", "", UFBXW_PROP_DATA_REAL },
	{ "FieldOfViewX", "", UFBXW_PROP_DATA_REAL },
	{ "FieldOfViewY", "", UFBXW_PROP_DATA_REAL },
	{ "Lcl Translation", "", UFBXW_PROP_DATA_VEC3 },
	{ "Lcl Rotation", "", UFBXW_PROP_DATA_VEC3 },
	{ "Lcl Scaling", "", UFBXW_PROP_DATA_VEC3 },
};

ufbxw_static_assert(ufbxwi_prop_type_count, ufbxwi_arraycount(ufbxwi_prop_types) == UFBXW_PROP_TYPE_FIRST_CUSTOM);

typedef struct {
	ufbxwi_token type;
} ufbxwi_object_desc;

static const ufbxwi_object_desc ufbxwi_object_types[] = {
	{ UFBXWI_GlobalSettings },
	{ UFBXWI_AnimationStack },
	{ UFBXWI_AnimationLayer },
	{ UFBXWI_NodeAttribute },
	{ UFBXWI_Geometry },
	{ UFBXWI_Material },
	{ UFBXWI_Model },
	{ UFBXWI_AnimationCurveNode },
	{ UFBXWI_AnimationCurve },
};

typedef struct {
	union {
		bool bool_;
		int32_t int32_t;
		int64_t int64_t;
		ufbxw_real real;
		ufbxw_vec2 vec2;
		ufbxw_vec3 vec3;
		ufbxw_vec4 vec4;
		ufbxw_string string;
		ufbxw_id id;
		ufbxw_real_string real_string;
		ufbxw_blob blob;
		ufbxw_user_int user_int;
		ufbxw_user_real user_real;
		ufbxw_user_enum user_enum;
	} zero;
	bool bool_true;
	int32_t int_neg1;
	ufbxw_real double_1;
	ufbxw_real double_10;
	ufbxw_vec3 vec3_1;
	ufbxw_vec3 vec3_color;
} ufbxwi_prop_defaults;

static const ufbxwi_prop_defaults ufbxwi_prop_default_data = {
	{ 0 }, // zero
	true,
	-1,
	(ufbxw_real)1.0,
	(ufbxw_real)10.0,
	{ 1.0f, 1.0f, 1.0f },
	{ (ufbxw_real)0.8, (ufbxw_real)0.8, (ufbxw_real)0.8 },
};

typedef struct {
	ufbxwi_token name;
	uint8_t type;
	int32_t value_offset;
	int32_t default_offset;
	uint32_t flags;
} ufbxwi_prop_desc;

#define ufbxwi_field(m_type, m_field) (int32_t)offsetof(m_type, m_field)
#define ufbxwi_default(m_field) -(int32_t)offsetof(ufbxwi_prop_defaults, m_field)

static const ufbxwi_prop_desc ufbxwi_node_props[] = {
	{ UFBXWI_QuaternionInterpolate, UFBXW_PROP_TYPE_ENUM, },
	{ UFBXWI_RotationOffset, UFBXW_PROP_TYPE_VECTOR3D, ufbxwi_field(ufbxwi_node, rotation_offset) },
	{ UFBXWI_RotationPivot, UFBXW_PROP_TYPE_VECTOR3D, ufbxwi_field(ufbxwi_node, rotation_pivot) },
	{ UFBXWI_ScalingOffset, UFBXW_PROP_TYPE_VECTOR3D, ufbxwi_field(ufbxwi_node, scaling_offset) },
	{ UFBXWI_ScalingPivot, UFBXW_PROP_TYPE_VECTOR3D, ufbxwi_field(ufbxwi_node, scaling_pivot) },
	{ UFBXWI_TranslationActive, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_TranslationMin, UFBXW_PROP_TYPE_VECTOR3D, },
	{ UFBXWI_TranslationMax, UFBXW_PROP_TYPE_VECTOR3D, },
	{ UFBXWI_TranslationMinX, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_TranslationMinY, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_TranslationMinZ, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_TranslationMaxX, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_TranslationMaxY, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_TranslationMaxZ, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_RotationOrder, UFBXW_PROP_TYPE_ENUM, ufbxwi_field(ufbxwi_node, rotation_order) },
	{ UFBXWI_RotationSpaceForLimitOnly, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_RotationStiffnessX, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_RotationStiffnessY, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_RotationStiffnessZ, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_AxisLen, UFBXW_PROP_TYPE_DOUBLE, ufbxwi_default(double_10) },
	{ UFBXWI_PreRotation, UFBXW_PROP_TYPE_VECTOR3D, ufbxwi_field(ufbxwi_node, pre_rotation) },
	{ UFBXWI_PostRotation, UFBXW_PROP_TYPE_VECTOR3D, ufbxwi_field(ufbxwi_node, post_rotation) },
	{ UFBXWI_RotationActive, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_RotationMin, UFBXW_PROP_TYPE_VECTOR3D, },
	{ UFBXWI_RotationMax, UFBXW_PROP_TYPE_VECTOR3D, },
	{ UFBXWI_RotationMinX, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_RotationMinY, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_RotationMinZ, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_RotationMaxX, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_RotationMaxY, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_RotationMaxZ, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_InheritType, UFBXW_PROP_TYPE_ENUM, ufbxwi_field(ufbxwi_node, inherit_type) },
	{ UFBXWI_ScalingActive, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_ScalingMin, UFBXW_PROP_TYPE_VECTOR3D, },
	{ UFBXWI_ScalingMax, UFBXW_PROP_TYPE_VECTOR3D, ufbxwi_default(vec3_1) },
	{ UFBXWI_ScalingMinX, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_ScalingMinY, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_ScalingMinZ, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_ScalingMaxX, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_ScalingMaxY, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_ScalingMaxZ, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_GeometricTranslation, UFBXW_PROP_TYPE_VECTOR3D, ufbxwi_field(ufbxwi_node, geometric_translation) },
	{ UFBXWI_GeometricRotation, UFBXW_PROP_TYPE_VECTOR3D, ufbxwi_field(ufbxwi_node, geometric_rotation) },
	{ UFBXWI_GeometricScaling, UFBXW_PROP_TYPE_VECTOR3D, ufbxwi_field(ufbxwi_node, geometric_scaling), ufbxwi_default(vec3_1) },
	{ UFBXWI_MinDampRangeX, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_MinDampRangeY, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_MinDampRangeZ, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_MaxDampRangeX, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_MaxDampRangeY, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_MaxDampRangeZ, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_MinDampStrengthX, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_MinDampStrengthY, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_MinDampStrengthZ, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_MaxDampStrengthX, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_MaxDampStrengthY, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_MaxDampStrengthZ, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_PreferedAngleX, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_PreferedAngleY, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_PreferedAngleZ, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_LookAtProperty, UFBXW_PROP_TYPE_OBJECT, },
	{ UFBXWI_UpVectorProperty, UFBXW_PROP_TYPE_OBJECT, },
	{ UFBXWI_Show, UFBXW_PROP_TYPE_BOOL, ufbxwi_default(bool_true) },
	{ UFBXWI_NegativePercentShapeSupport, UFBXW_PROP_TYPE_BOOL, ufbxwi_default(bool_true) },
	{ UFBXWI_DefaultAttributeIndex, UFBXW_PROP_TYPE_INT, ufbxwi_default(int_neg1) },
	{ UFBXWI_Freeze, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_LODBox, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_Lcl_Translation, UFBXW_PROP_TYPE_LCL_TRANSLATION, ufbxwi_field(ufbxwi_node, lcl_translation), 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_Lcl_Rotation, UFBXW_PROP_TYPE_LCL_ROTATION, ufbxwi_field(ufbxwi_node, lcl_rotation), 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_Lcl_Scaling, UFBXW_PROP_TYPE_LCL_SCALING, ufbxwi_field(ufbxwi_node, lcl_scaling), ufbxwi_default(vec3_1), UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_Visibility, UFBXW_PROP_TYPE_VISIBILITY, ufbxwi_field(ufbxwi_node, visibility), ufbxwi_default(bool_true), UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_Visibility_Inheritance, UFBXW_PROP_TYPE_VISIBILITY_INHERITANCE, ufbxwi_field(ufbxwi_node, visibility_inheritance), ufbxwi_default(bool_true), UFBXW_PROP_FLAG_ANIMATABLE },
};

static const ufbxwi_prop_desc ufbxwi_mesh_props[] = {
	{ UFBXWI_Color, UFBXW_PROP_TYPE_COLOR_RGB, ufbxwi_default(vec3_color) },
	{ UFBXWI_BBoxMin, UFBXW_PROP_TYPE_VECTOR3D },
	{ UFBXWI_BBoxMax, UFBXW_PROP_TYPE_VECTOR3D },
	{ UFBXWI_Primary_Visibility, UFBXW_PROP_TYPE_BOOL, ufbxwi_default(bool_true) },
	{ UFBXWI_Casts_Shadows, UFBXW_PROP_TYPE_BOOL, ufbxwi_default(bool_true) },
	{ UFBXWI_Receive_Shadows, UFBXW_PROP_TYPE_BOOL, ufbxwi_default(bool_true) },
};

static const ufbxwi_prop_desc ufbxwi_material_lambert_props[] = {
	{ UFBXWI_ShadingModel, UFBXW_PROP_TYPE_STRING, },
	{ UFBXWI_MultiLayer, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_EmissiveColor, UFBXW_PROP_TYPE_COLOR, },
	{ UFBXWI_EmissiveFactor, UFBXW_PROP_TYPE_NUMBER, },
	{ UFBXWI_AmbientColor, UFBXW_PROP_TYPE_COLOR, },
	{ UFBXWI_AmbientFactor, UFBXW_PROP_TYPE_NUMBER, },
	{ UFBXWI_DiffuseColor, UFBXW_PROP_TYPE_COLOR, ufbxwi_default(vec3_color) },
	{ UFBXWI_DiffuseFactor, UFBXW_PROP_TYPE_NUMBER, ufbxwi_default(double_1) },
	{ UFBXWI_Bump, UFBXW_PROP_TYPE_VECTOR3D, },
	{ UFBXWI_NormalMap, UFBXW_PROP_TYPE_VECTOR3D, },
	{ UFBXWI_BumpFactor, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_TransparentColor, UFBXW_PROP_TYPE_COLOR, },
	{ UFBXWI_TransparencyFactor, UFBXW_PROP_TYPE_NUMBER, },
	{ UFBXWI_DisplacementColor, UFBXW_PROP_TYPE_COLOR_RGB, },
	{ UFBXWI_DisplacementFactor, UFBXW_PROP_TYPE_DOUBLE, },
	{ UFBXWI_VectorDisplacementColor, UFBXW_PROP_TYPE_COLOR_RGB, },
	{ UFBXWI_VectorDisplacementFactor, UFBXW_PROP_TYPE_DOUBLE, },
};

static const ufbxwi_prop_desc ufbxwi_document_props[] = {
	{ UFBXWI_SourceObject, UFBXW_PROP_TYPE_OBJECT },
	{ UFBXWI_ActiveAnimStackName, UFBXW_PROP_TYPE_STRING },
};

typedef struct {
	uint16_t data_size;
} ufbxwi_element_type_info;

static const ufbxwi_element_type_info ufbxwi_element_type_infos[] = {
	{ 0 }, // 0
	{ 0 }, // CUSTOM
	{ sizeof(ufbxwi_node) }, // NODE
	{ 0 }, // CUSTOM_NODE_ATTRIBUTE
	{ sizeof(ufbxwi_mesh) }, // MESH
	{ sizeof(ufbxwi_material) }, // MATERIAL
	{ sizeof(ufbxwi_template) }, // TEMPLATE
	{ 0 }, // SCENE_INFO
	{ 0 }, // GLOBAL_SETTINGS
	{ sizeof(ufbxwi_document) }, // DOCUMENT
};

ufbxw_static_assert(ufbxwi_element_type_infos_count, ufbxwi_arraycount(ufbxwi_element_type_infos) == UFBXW_ELEMENT_TYPE_COUNT);

static void ufbxwi_init_node_data(void *data)
{
	ufbxwi_node *node = (ufbxwi_node*)data;
	node->lcl_scaling = ufbxwi_one_vec3;
	node->geometric_scaling = ufbxwi_one_vec3;
	node->visibility = true;
	node->visibility_inheritance = true;
}

enum {
	UFBXWI_ELEMENT_TYPE_FLAG_EAGER_PROPS = 0x1,
};

typedef struct {
	ufbxw_element_type element_type;
	ufbxwi_token class_type;
	ufbxwi_token sub_type;
	ufbxwi_token object_type;
	ufbxwi_token fbx_type;
	ufbxwi_token tmpl_type;
	const ufbxwi_prop_desc *props;
	size_t num_props;
	ufbxwi_element_init_data_fn *init_fn;
	uint32_t flags;
} ufbxwi_element_type_desc;

static const ufbxwi_element_type_desc ufbxwi_element_types[] = {
	{
		UFBXW_ELEMENT_TEMPLATE, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE,
		NULL, 0, NULL,
		UFBXWI_ELEMENT_FLAG_ALLOW_NO_OBJECT_ID,
	},
	{
		UFBXW_ELEMENT_NODE, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE, UFBXWI_Model, UFBXWI_Model, UFBXWI_FbxNode,
		ufbxwi_node_props, ufbxwi_arraycount(ufbxwi_node_props), &ufbxwi_init_node_data,
		0,
	},
	{
		UFBXW_ELEMENT_MESH, UFBXWI_TOKEN_NONE, UFBXWI_Mesh, UFBXWI_Geometry, UFBXWI_Geometry, UFBXWI_FbxMesh,
		ufbxwi_mesh_props, ufbxwi_arraycount(ufbxwi_mesh_props), NULL,
		0,
	},
	{
		UFBXW_ELEMENT_SCENE_INFO, UFBXWI_TOKEN_NONE, UFBXWI_SceneInfo, UFBXWI_UserData, UFBXWI_SceneInfo, UFBXWI_TOKEN_NONE,
		NULL, 0, NULL,
		UFBXWI_ELEMENT_FLAG_ALLOW_NO_OBJECT_ID,
	},
	{
		UFBXW_ELEMENT_GLOBAL_SETTINGS, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE, UFBXWI_GlobalSettings, UFBXWI_TOKEN_NONE, UFBXWI_GlobalSettings,
		NULL, 0, NULL,
		0,
	},
	{
		UFBXW_ELEMENT_DOCUMENT, UFBXWI_TOKEN_NONE, UFBXWI_Document, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE,
		ufbxwi_document_props, ufbxwi_arraycount(ufbxwi_document_props), NULL,
		UFBXWI_ELEMENT_FLAG_ALLOW_NO_OBJECT_ID,
	},
	{
		UFBXW_ELEMENT_MATERIAL, UFBXWI_FbxSurfaceLambert, UFBXWI_TOKEN_EMPTY, UFBXWI_Material, UFBXWI_Material, UFBXWI_FbxSurfaceLambert,
		ufbxwi_material_lambert_props, ufbxwi_arraycount(ufbxwi_material_lambert_props), NULL,
		UFBXWI_ELEMENT_TYPE_FLAG_EAGER_PROPS,
	},
};

static bool ufbxwi_props_rehash(ufbxw_scene *scene, ufbxwi_props *props, size_t min_capacity)
{
	uint32_t old_capacity = props->capacity;
	uint32_t new_capacity = ufbxwi_max_u32(old_capacity * 2, 1);
	while (new_capacity < min_capacity * 2) {
		new_capacity *= 2;
	}

	ufbxwi_prop *new_slots = ufbxwi_alloc(&scene->ator, ufbxwi_prop, new_capacity);
	ufbxwi_check_return(new_slots, false);

	memset(new_slots, 0, new_capacity * sizeof(ufbxwi_prop));

	ufbxwi_prop *old_slots = props->props;

	uint32_t mask = new_capacity - 1;
	for (size_t i = 0; i < old_capacity; i++) {
		uint32_t token = old_slots[i].token;
		if (token == 0 || token == ~0u) continue;

		uint32_t hash = ufbxwi_hash_token(token);
		for (uint32_t scan = 0; ; scan++) {
			uint32_t slot = (hash + scan*scan) & mask;
			if (new_slots[slot].token == 0) {
				new_slots[slot] = old_slots[i];
				break;
			}
		}
	}

	ufbxwi_free(&scene->ator, old_slots);

	props->props = new_slots;
	props->capacity = new_capacity;
	return true;
}

static ufbxwi_prop *ufbxwi_props_find_prop(ufbxwi_props *props, ufbxwi_token token)
{
	if (props->count == 0) return NULL;

	ufbxwi_prop *prop = NULL;

	uint32_t mask = props->capacity - 1;
	uint32_t hash = ufbxwi_hash_token(token);
	ufbxwi_prop *slots = props->props;
	for (uint32_t scan = 0; ; scan++) {
		uint32_t slot = (hash + scan*scan) & mask;
		if (slots[slot].token == token) {
			return &slots[slot];
		} else if (slots[slot].token == 0) {
			return NULL;
		}
	}
}

static ufbxwi_prop *ufbxwi_props_add_prop(ufbxw_scene *scene, ufbxwi_props *props, ufbxwi_token token)
{
	ufbxw_assert(token != UFBXWI_TOKEN_NONE && token != UFBXWI_TOKEN_EMPTY);
	if (props->count * 2 >= props->capacity) {
		ufbxwi_check_return(ufbxwi_props_rehash(scene, props, 4), NULL);
	}

	ufbxwi_prop *prop = NULL;

	uint32_t mask = props->capacity - 1;
	uint32_t hash = ufbxwi_hash_token(token);
	ufbxwi_prop *slots = props->props;
	for (uint32_t scan = 0; ; scan++) {
		uint32_t slot = (hash + scan*scan) & mask;
		if (slots[slot].token == token) {
			prop = &slots[slot];
			break;
		} else if (slots[slot].token == 0) {
			prop = &slots[slot];
			break;
		}
	}

	if (prop->token == 0) {
		prop->token = (uint32_t)token;
		prop->order = ++props->order_counter;
	}
	props->count++;

	return prop;
}

static bool ufbxwi_props_copy(ufbxw_scene *scene, ufbxwi_props *dst, const ufbxwi_props *src)
{
	ufbxwi_prop *copy = ufbxwi_alloc(&scene->ator, ufbxwi_prop, src->capacity);
	ufbxwi_check_return(copy, false);
	memcpy(copy, src->props, src->capacity * sizeof(ufbxwi_prop));
	*dst = *src;
	dst->props = copy;
	return true;
}

static uint32_t ufbxwi_find_element_type_id(ufbxw_scene *scene, ufbxw_element_type type, ufbxwi_token class_type)
{
	ufbxwi_element_type_list types = scene->element_types;
	for (uint32_t i = 0; i < types.count; i++) {
		ufbxwi_element_type *et = &types.data[i];
		if (et->element_type != type) continue;
		if (et->class_type == class_type) return i;
	}
	return ~0u;
}

static int32_t ufbxwi_element_add_field(ufbxw_scene *scene, ufbxwi_element *element, ufbxw_prop_data_type data_type)
{
	ufbxwi_prop_data_info data_info = ufbxwi_prop_data_infos[data_type];
	size_t data_offset = ufbxwi_align(element->data_size, data_info.alignment);
	size_t data_end = data_offset + data_info.size;

	if (data_end > element->data_capacity) {
		size_t new_capacity = ufbxwi_max_sz(data_end, element->data_capacity * 2);
		void *new_data = ufbxwi_alloc_size(&scene->ator, 1, new_capacity, &new_capacity);
		ufbxwi_check_return(new_data, 0);

		memcpy(new_data, element->data, element->data_size);
		memset((char*)new_data + element->data_size, 0, new_capacity - element->data_size);

		ufbxwi_free(&scene->ator, element->data);
		element->data = new_data;
		element->data_capacity = new_capacity;
	}

	element->data_size = data_end;
	return (int32_t)data_offset;
}

static ufbxwi_prop *ufbxwi_element_find_prop(ufbxw_scene *scene, ufbxwi_element *element, ufbxwi_token token)
{
	ufbxwi_prop *prop = ufbxwi_props_find_prop(&element->props, token);
	if (!prop && element->flags & UFBXWI_ELEMENT_FLAG_HAS_DEFAULT_PROPS) {
		ufbxwi_props *default_props = &scene->element_types.data[element->type_id].props;
		prop = ufbxwi_props_find_prop(default_props, token);
	}
	return prop;
}

static ufbxwi_prop *ufbxwi_element_edit_prop(ufbxw_scene *scene, ufbxwi_element *element, ufbxwi_token token)
{
	if (element->flags & UFBXWI_ELEMENT_FLAG_HAS_DEFAULT_PROPS) {
		ufbxwi_props *default_props = &scene->element_types.data[element->type_id].props;

		ufbxwi_prop *prop = ufbxwi_props_find_prop(&element->props, token);
		if (!prop) {
			prop = ufbxwi_props_find_prop(default_props, token);
			if (!prop) return NULL;

			ufbxwi_prop *new_prop = ufbxwi_props_add_prop(scene, &element->props, token);
			ufbxwi_check_return(new_prop, NULL);
			*new_prop = *prop;
			prop = new_prop;
		}
		if (prop->value_offset <= 0) {
			ufbxw_prop_data_type data_type = scene->prop_types.data[prop->type].data_type;
			int32_t offset = ufbxwi_element_add_field(scene, element, data_type);
			ufbxwi_check_return(offset != 0, NULL);

			const char *src = (const char*)&ufbxwi_prop_default_data + -prop->value_offset;
			char *dst = (char*)element->data + offset;
			memcpy(dst, src, ufbxwi_prop_data_infos[data_type].size);

			prop->value_offset = offset;
		}
		return prop;
	} else {
		ufbxwi_prop *prop = ufbxwi_props_find_prop(&element->props, token);
		if (prop && prop->value_offset <= 0) {
			ufbxw_prop_data_type data_type = scene->prop_types.data[prop->type].data_type;
			int32_t offset = ufbxwi_element_add_field(scene, element, data_type);
			ufbxwi_check_return(offset != 0, NULL);
			prop->value_offset = offset;
		}

		return prop;
	}
}

static ufbxwi_prop *ufbxwi_element_add_prop(ufbxw_scene *scene, ufbxwi_element *element, ufbxwi_token token, ufbxw_prop_type type)
{
	ufbxw_prop_data_type data_type = scene->prop_types.data[type].data_type;
	if (element->flags & UFBXWI_ELEMENT_FLAG_HAS_DEFAULT_PROPS) {
		ufbxwi_props *default_props = &scene->element_types.data[element->type_id].props;

		ufbxwi_prop *prop = ufbxwi_props_find_prop(&element->props, token);
		if (!prop) {
			prop = ufbxwi_props_find_prop(default_props, token);
		}
		if (prop) {
			ufbxwi_check_return(scene->prop_types.data[prop->type].data_type == data_type, NULL);
		}
		if (!prop || prop->value_offset <= 0) {
			ufbxwi_prop *new_prop = ufbxwi_props_add_prop(scene, &element->props, token);
			ufbxwi_check_return(new_prop, NULL);

			int32_t offset = ufbxwi_element_add_field(scene, element, data_type);
			ufbxwi_check_return(offset != 0, NULL);
			if (prop) {
				const char *src = (const char*)&ufbxwi_prop_default_data + -prop->value_offset;
				char *dst = (char*)element->data + offset;
				memcpy(dst, src, ufbxwi_prop_data_infos[data_type].size);
			}

			if (prop) {
				*new_prop = *prop;
			} else {
				new_prop->type = type;
			}
			new_prop->value_offset = offset;
			prop = new_prop;
		}
		return prop;
	} else {
		ufbxwi_prop *prop = ufbxwi_props_add_prop(scene, &element->props, token);
		ufbxwi_check_return(prop, NULL);

		if (prop->value_offset <= 0) {
			int32_t offset = ufbxwi_element_add_field(scene, element, data_type);
			ufbxwi_check_return(offset != 0, NULL);
			prop->value_offset = offset;
			prop->type = type;
		} else {
			ufbxwi_check_return(scene->prop_types.data[prop->type].data_type == data_type, NULL);
		}

		return prop;
	}
}

static ufbxwi_noinline bool ufbxwi_cast_value(void *dst, const void *src, ufbxw_prop_data_type dst_type, ufbxw_prop_data_type src_type)
{
	if (dst_type == src_type) {
		memcpy(dst, src, ufbxwi_prop_data_infos[dst_type].size);
		return true;
	}

	if (dst_type == UFBXW_PROP_DATA_BOOL) {
		bool *d = (bool*)dst;
		switch (src_type) {
		case UFBXW_PROP_DATA_INT32:
			*d = *(const int32_t*)src != 0;
			return true;
		case UFBXW_PROP_DATA_INT64:
			*d = *(const int64_t*)src != 0;
			return true;
		}
	} else if (dst_type == UFBXW_PROP_DATA_INT32) {
		int32_t *d = (int32_t*)dst;
		switch (src_type) {
		case UFBXW_PROP_DATA_BOOL:
			*d = *(const bool*)src ? 1 : 0;
			return true;
		case UFBXW_PROP_DATA_INT64:
			// TODO: Handle out of bounds
			*d = (int32_t)*(const int64_t*)src;
			return true;
		case UFBXW_PROP_DATA_USER_INT:
			*d = ((const ufbxw_user_int*)src)->value;
			return true;
		case UFBXW_PROP_DATA_USER_ENUM:
			*d = ((const ufbxw_user_enum*)src)->value;
			return true;
		}
	} else if (dst_type == UFBXW_PROP_DATA_INT64) {
		uint64_t *d = (uint64_t*)dst;
		switch (src_type) {
		case UFBXW_PROP_DATA_BOOL:
			*d = *(const bool*)src ? 1 : 0;
			return true;
		case UFBXW_PROP_DATA_INT32:
			*d = *(const int32_t*)src;
			return true;
		case UFBXW_PROP_DATA_USER_INT:
			*d = ((const ufbxw_user_int*)src)->value;
			return true;
		case UFBXW_PROP_DATA_USER_ENUM:
			*d = ((const ufbxw_user_enum*)src)->value;
			return true;
		}
	} else if (dst_type == UFBXW_PROP_DATA_REAL) {
		ufbxw_real *d = (ufbxw_real*)dst;
		switch (src_type) {
		case UFBXW_PROP_DATA_USER_REAL:
			*d = ((const ufbxw_user_real*)src)->value;
			return true;
		}
	} else if (dst_type == UFBXW_PROP_DATA_USER_INT) {
		ufbxw_user_int *d = (ufbxw_user_int*)dst;
		switch (src_type) {
		case UFBXW_PROP_DATA_BOOL:
			d->value = *(const bool*)src ? 1 : 0;
			return true;
		case UFBXW_PROP_DATA_INT32:
			d->value = *(const int32_t*)src;
			return true;
		case UFBXW_PROP_DATA_INT64:
			// TODO: Handle out of bounds
			d->value = (int32_t)*(const int64_t*)src;
			return true;
		}
	} else if (dst_type == UFBXW_PROP_DATA_USER_REAL) {
		ufbxw_user_real *d = (ufbxw_user_real*)dst;
		switch (src_type) {
		case UFBXW_PROP_DATA_REAL:
			d->value = *(const int32_t*)src;
			return true;
		}
	} else if (dst_type == UFBXW_PROP_DATA_USER_ENUM) {
		ufbxw_user_enum *d = (ufbxw_user_enum*)dst;
		switch (src_type) {
		case UFBXW_PROP_DATA_BOOL:
			d->value = *(const bool*)src ? 1 : 0;
			return true;
		case UFBXW_PROP_DATA_INT32:
			d->value = *(const int32_t*)src;
			return true;
		case UFBXW_PROP_DATA_INT64:
			// TODO: Handle out of bounds
			d->value = (int32_t)*(const int64_t*)src;
			return true;
		}
	}

	return false;
}

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

static ufbxwi_forceinline const void *ufbxwi_resolve_value_offset(const void *base, int32_t offset)
{
	if (offset > 0) {
		return (const char*)base + offset;
	} else {
		ufbxw_assert(-offset < sizeof(ufbxwi_prop_default_data));
		return (const char*)&ufbxwi_prop_default_data + -offset;
	}
}

static ufbxw_id ufbxwi_create_element(ufbxw_scene *scene, ufbxw_element_type type, ufbxwi_token class_type)
{
	uint32_t type_id = ufbxwi_find_element_type_id(scene, type, class_type);
	ufbxwi_check_return(type_id != ~0u, ufbxw_null_id);

	const ufbxwi_element_type_info *type_info = &ufbxwi_element_type_infos[type];

	size_t data_size = type_info->data_size;
	if (data_size < sizeof(ufbxwi_element_data)) {
		data_size = sizeof(ufbxwi_element_data);
	}

	void *data = NULL;

	size_t data_capacity = 0;
	data = ufbxwi_alloc_size(&scene->ator, 1, data_size, &data_capacity);
	ufbxwi_check_return(data, 0);
	memset(data, 0, data_capacity);

	size_t index = 0;
	if (scene->free_element_ids.count > 0) {
		index = scene->free_element_ids.data[--scene->free_element_ids.count];
	} else {
		index = scene->elements.count;
		ufbxwi_check_return(ufbxwi_list_push_zero(&scene->ator, &scene->elements, ufbxwi_element), 0);
	}

	ufbxwi_element *element = &scene->elements.data[index];
	uint32_t generation = ufbxwi_id_generation(element->id) + 1;

	ufbxw_id id = ufbxwi_make_id(type, generation, index);
	element->id = id;
	element->name = ufbxwi_empty_string;
	element->data = data;
	element->data_size = data_size;
	element->data_capacity = data_capacity;

	ufbxwi_element_type *element_type = &scene->element_types.data[type_id];
	element->type_id = type_id;

	if (element_type->props.count == 0) {
		// No properties
	} else if ((element_type->flags & UFBXWI_ELEMENT_TYPE_FLAG_EAGER_PROPS) != 0) {
		ufbxwi_check_return(ufbxwi_props_copy(scene, &element->props, &element_type->props), 0);
	} else {
		element->flags |= UFBXWI_ELEMENT_FLAG_HAS_DEFAULT_PROPS;
		element->props.order_counter = element_type->props.order_counter;
	}

	ufbxwi_element_data *element_data = (ufbxwi_element_data*)data;
	element_data->id = id;

	if (element_type->init_fn) {
		element_type->init_fn(data);
	}

	scene->num_elements++;

	return ufbxwi_make_id(type, generation, index);
}

static ufbxwi_noinline bool ufbxwi_set_prop(ufbxw_scene *scene, ufbxw_id id, const char *prop, size_t prop_len, const void *src, ufbxw_prop_data_type src_type)
{
	ufbxwi_token token = ufbxwi_get_token(&scene->string_pool, prop, prop_len);
	ufbxwi_element *element = ufbxwi_get_element(scene, id);
	if (!token || !element) return false;

	ufbxwi_prop *p = ufbxwi_element_edit_prop(scene, element, token);
	if (!p) return false;
	ufbxw_assert(p->value_offset > 0);

	ufbxw_prop_data_type data_type = scene->prop_types.data[p->type].data_type;

	void *dst = (char*)element->data + p->value_offset;
	if (data_type == src_type) {
		memcpy(dst, src, ufbxwi_prop_data_infos[src_type].size);
		return true;
	} else {
		return ufbxwi_cast_value(dst, src, data_type, src_type);
	}
}

static ufbxwi_noinline bool ufbxwi_add_prop(ufbxw_scene *scene, ufbxw_id id, const char *prop, size_t prop_len, ufbxw_prop_type type, const void *src, ufbxw_prop_data_type src_type)
{
	ufbxwi_token token = ufbxwi_intern_token(&scene->string_pool, prop, prop_len);
	ufbxwi_element *element = ufbxwi_get_element(scene, id);
	if (!token || !element) return false;

	ufbxwi_prop *p = ufbxwi_element_add_prop(scene, element, token, type);
	if (!p) return false;
	ufbxw_assert(p->value_offset > 0);

	ufbxw_prop_data_type data_type = scene->prop_types.data[type].data_type;
	void *dst = (char*)element->data + p->value_offset;
	if (data_type == src_type) {
		memcpy(dst, src, ufbxwi_prop_data_infos[src_type].size);
		return true;
	} else {
		return ufbxwi_cast_value(dst, src, data_type, src_type);
	}
}

static ufbxwi_noinline bool ufbxwi_get_prop(ufbxw_scene *scene, ufbxw_id id, const char *prop, size_t prop_len, void *dst, ufbxw_prop_data_type dst_type)
{
	ufbxwi_token token = ufbxwi_get_token(&scene->string_pool, prop, prop_len);
	ufbxwi_element *element = ufbxwi_get_element(scene, id);
	if (!token || !element) return false;

	ufbxwi_prop *p = ufbxwi_element_find_prop(scene, element, token);
	if (!p) return false;

	ufbxw_prop_data_type data_type = scene->prop_types.data[p->type].data_type;
	const void *src = ufbxwi_resolve_value_offset(element->data, p->value_offset);
	if (data_type == dst_type) {
		memcpy(dst, src, ufbxwi_prop_data_infos[dst_type].size);
		return true;
	} else {
		return ufbxwi_cast_value(dst, src, dst_type, data_type);
	}
}

static ufbxwi_forceinline ufbxwi_node *ufbxwi_get_node_data(ufbxw_scene *scene, ufbxw_node node) { return (ufbxwi_node*)ufbxwi_get_element_data(scene, node.id); }
static ufbxwi_forceinline ufbxwi_mesh *ufbxwi_get_mesh_data(ufbxw_scene *scene, ufbxw_mesh mesh) { return (ufbxwi_mesh*)ufbxwi_get_element_data(scene, mesh.id); }

static ufbxwi_forceinline ufbxw_node ufbxwi_assert_node(ufbxw_id id) { ufbxw_assert(ufbxwi_id_type(id) == UFBXW_ELEMENT_NODE); ufbxw_node node = { id }; return node; }

static bool ufbwi_init_tokens(ufbxw_scene *scene)
{
	// Reserve the none and empty tokens
	{
		ufbxw_string *null_tokens = ufbxwi_list_push_zero_n(&scene->ator, &scene->string_pool.tokens, ufbxw_string, 2);
		ufbxwi_check_return(null_tokens, false);
		null_tokens[0].data = ufbxwi_empty_char;
		null_tokens[1].data = ufbxwi_empty_char;
	}

	ufbxwi_for(const ufbxw_string, str, ufbxwi_tokens, ufbxwi_arraycount(ufbxwi_tokens)) {
		ufbxwi_check_return(ufbxwi_intern_token(&scene->string_pool, str->data, str->length), false);
	}

	// Reserve the count token
	{
		ufbxw_string *null_tokens = ufbxwi_list_push_zero_n(&scene->ator, &scene->string_pool.tokens, ufbxw_string, 1);
		ufbxwi_check_return(null_tokens, false);
		null_tokens[0].data = ufbxwi_empty_char;
	}

	return true;
}

static bool ufbxwi_init_prop_types(ufbxw_scene *scene)
{
	ufbxwi_prop_type *dst_type = ufbxwi_list_push_zero_n(&scene->ator, &scene->prop_types, ufbxwi_prop_type, ufbxwi_arraycount(ufbxwi_prop_types));
	ufbxwi_check_return(dst_type, false);

	ufbxwi_for(const ufbxwi_prop_type_desc, desc, ufbxwi_prop_types, ufbxwi_arraycount(ufbxwi_prop_types)) {
		ufbxwi_check_return(ufbxwi_intern_string(&dst_type->type, &scene->string_pool, desc->type, strlen(desc->type)), false);
		ufbxwi_check_return(ufbxwi_intern_string(&dst_type->sub_type, &scene->string_pool, desc->sub_type, strlen(desc->sub_type)), false);
		dst_type->data_type = desc->data_type;

		dst_type++;
	}

	return true;
}

static bool ufbxwi_create_object_types(ufbxw_scene *scene)
{
	ufbxwi_object_type *object_types = ufbxwi_list_push_zero_n(&scene->ator, &scene->object_types, ufbxwi_object_type, ufbxwi_arraycount(ufbxwi_object_types));
	ufbxwi_check_return(object_types, false);

	ufbxwi_object_type *dst = object_types;
	ufbxwi_for(const ufbxwi_object_desc, desc, ufbxwi_object_types, ufbxwi_arraycount(ufbxwi_object_types)) {
		dst->type = desc->type;
		dst++;
	}

	return true;
}

static bool ufbxwi_create_element_types(ufbxw_scene *scene)
{
	for (size_t type_ix = 0; type_ix < ufbxwi_arraycount(ufbxwi_element_types); type_ix++) {
		const ufbxwi_element_type_desc *desc = &ufbxwi_element_types[type_ix];
		ufbxwi_element_type *et = ufbxwi_list_push_zero(&scene->ator, &scene->element_types, ufbxwi_element_type);

		et->element_type = desc->element_type;
		et->class_type = desc->class_type;
		et->sub_type = desc->sub_type;
		et->object_type = desc->object_type;
		et->fbx_type = desc->fbx_type;
		et->init_fn = desc->init_fn;
		et->flags = desc->flags;

		et->object_type_id = ~0u;
		for (uint32_t i = 0; i < scene->object_types.count; i++) {
			if (scene->object_types.data[i].type == desc->object_type) {
				et->object_type_id = i;
				break;
			}
		}
		ufbxw_assert(et->object_type_id != ~0u || (desc->flags & UFBXWI_ELEMENT_FLAG_ALLOW_NO_OBJECT_ID) != 0);

		if (desc->num_props > 0) {
			ufbxwi_check_return(ufbxwi_props_rehash(scene, &et->props, desc->num_props), false);

			ufbxwi_for(const ufbxwi_prop_desc, pd, desc->props, desc->num_props) {
				ufbxwi_prop *prop = ufbxwi_props_add_prop(scene, &et->props, pd->name);
				ufbxwi_check_return(prop, false);
				prop->type = pd->type;
				prop->value_offset = pd->value_offset;
			}

			if (desc->tmpl_type != UFBXWI_TOKEN_NONE) {
				ufbxw_id template_id = ufbxw_create_element(scene, UFBXW_ELEMENT_TEMPLATE);
				ufbxwi_check_return(template_id, false);

				ufbxwi_element *tmpl_elem = ufbxwi_get_element(scene, template_id);
				ufbxw_assert(tmpl_elem);

				ufbxwi_template *tmpl_data = (ufbxwi_template*)tmpl_elem->data;
				tmpl_data->type = desc->tmpl_type;

				ufbxwi_check_return(ufbxwi_props_rehash(scene, &tmpl_elem->props, desc->num_props), false);

				ufbxwi_for(const ufbxwi_prop_desc, pd, desc->props, desc->num_props) {
					ufbxwi_prop *prop = ufbxwi_props_add_prop(scene, &tmpl_elem->props, pd->name);
					ufbxwi_check_return(prop, false);
					prop->type = pd->type;
					prop->value_offset = pd->value_offset <= 0 ? pd->value_offset : pd->default_offset;
				}

				et->template_id = template_id;
			}
		}
	}

	return true;
}

static void ufbxwi_create_defaults(ufbxw_scene *scene)
{
	if (!scene->opts.no_default_scene_info) {
		ufbxw_id id = ufbxw_create_element(scene, UFBXW_ELEMENT_SCENE_INFO);
		ufbxw_set_name(scene, id, "GlobalInfo");

		ufbxw_add_string(scene, id, "DocumentUrl", UFBXW_PROP_TYPE_URL, "test.fbx");
		ufbxw_add_string(scene, id, "SrcDocumentUrl", UFBXW_PROP_TYPE_URL, "test.fbx");
	}

	if (!scene->opts.no_default_global_settings) {
		ufbxw_id id = ufbxw_create_element(scene, UFBXW_ELEMENT_GLOBAL_SETTINGS);

		ufbxw_add_int(scene, id, "UpAxis", UFBXW_PROP_TYPE_INT, 1);
		ufbxw_add_int(scene, id, "UpAxisSign", UFBXW_PROP_TYPE_INT, 1);
		ufbxw_add_int(scene, id, "FrontAxis", UFBXW_PROP_TYPE_INT, 2);
		ufbxw_add_int(scene, id, "FrontAxisSign", UFBXW_PROP_TYPE_INT, 1);
		ufbxw_add_int(scene, id, "CoordAxis", UFBXW_PROP_TYPE_INT, 0);
		ufbxw_add_int(scene, id, "CoordAxisSign", UFBXW_PROP_TYPE_INT, 1);
		ufbxw_add_vec3(scene, id, "AmbientColor", UFBXW_PROP_TYPE_COLOR_RGB, ufbxw_zero_vec3);
		ufbxw_add_string(scene, id, "DefaultCamera", UFBXW_PROP_TYPE_STRING, "Producer Perspective");
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

	// size_t num_prop_types = (size_t)UFBXW_PROP_FIRST_USER;
	// ufbxwi_check(ufbxwi_list_push_zero_n(&scene->ator, &scene->prop_types, ufbxwi_prop_type, num_prop_types));

	ufbwi_init_tokens(scene);
	ufbxwi_init_prop_types(scene);
	ufbxwi_create_object_types(scene);
	ufbxwi_create_element_types(scene);
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

	ufbxwi_prop_list tmp_prop_list;

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

static void ufbxwi_ascii_comment(ufbxw_save_context *sc, const char *fmt, va_list args)
{
	// TODO: More rigorous
	size_t max_length = 512;
	char *dst = ufbxwi_write_reserve(sc, max_length);
	int len = vsnprintf(dst, max_length, fmt, args);
	ufbxw_assert(len >= 0);
	ufbxwi_write_commit(sc, (size_t)len);
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
		char f = *pf;
		if (pf != fmt) {
			if (f == 'C' || f == 'S' || f == 'T') {
				ufbxwi_write(sc, ", ", 2);
			} else {
				ufbxwi_write(sc, ",", 1);
			}
		}

		switch (f) {
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
		case 'T': {
			ufbxwi_token token = va_arg(args, ufbxwi_token);
			ufbxw_string str = sc->scene->string_pool.tokens.data[token];
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

static const char *ufbxwi_dom_section_str = "------------------------------------------------------------------";

static ufbxwi_forceinline void ufbxwi_dom_comment(ufbxw_save_context *sc, const char *fmt, ...)
{
	if (sc->opts.ascii) {
		va_list args;
		va_start(args, fmt);
		ufbxwi_ascii_comment(sc, fmt, args);
		va_end(args);
	}
}

static void ufbxwi_dom_section(ufbxw_save_context *sc, const char *name)
{
	if (sc->opts.ascii) {
		ufbxwi_dom_comment(sc, "\n; %s\n;%.66s\n\n", name, ufbxwi_dom_section_str);
	}
}

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
	UFBXWI_SAVE_ELEMENT_MANUAL_OPEN = 0x1,
	UFBXWI_SAVE_ELEMENT_NO_ID = 0x2,
};

static int ufbxwi_cmp_prop_identity(const void *va, const void *vb)
{
	const ufbxwi_prop *a = (const ufbxwi_prop*)va, *b = (const ufbxwi_prop*)vb;
	if (a->token != b->token) return a->token < b->token ? -1 : +1;
	uint32_t a_temp = a->flags & UFBXWI_PROP_FLAG_TEMP_MASK;
	uint32_t b_temp = b->flags & UFBXWI_PROP_FLAG_TEMP_MASK;
	if (a_temp != b_temp) return a_temp < b_temp ? -1 : +1;
	return 0;
}

static int ufbxwi_cmp_prop_order(const void *va, const void *vb)
{
	const ufbxwi_prop *a = (const ufbxwi_prop*)va, *b = (const ufbxwi_prop*)vb;
	if (a->order != b->order) return a->order < b->order ? -1 : +1;
	return 0;
}

static void ufbxwi_save_props(ufbxw_save_context *sc, const void *data_buffer, const ufbxwi_props *value_props, const ufbxwi_props *default_props, const void *template_buffer, const ufbxwi_props *template_props)
{
	ufbxw_scene *scene = sc->scene;

	// Merge all the properties into one sorted list
	size_t prop_count = value_props->count;
	if (default_props) prop_count += default_props->count;
	if (template_props) prop_count += template_props->count;

	ufbxwi_check_err(&sc->error, ufbxwi_list_resize_uninit(&sc->ator, &sc->tmp_prop_list, ufbxwi_prop, prop_count));

	ufbxwi_prop *props = sc->tmp_prop_list.data;

	{
		ufbxwi_prop *dst_prop = props;

		ufbxwi_for(const ufbxwi_prop, prop, value_props->props, value_props->capacity) {
			if (prop->token == 0 || prop->token == ~0u) continue;
			*dst_prop = *prop;
			dst_prop++;
		}

		if (default_props) {
			ufbxwi_for(const ufbxwi_prop, prop, default_props->props, default_props->capacity) {
				if (prop->token == 0 || prop->token == ~0u) continue;
				*dst_prop = *prop;
				dst_prop->flags |= UFBXWI_PROP_FLAG_TEMP_DEFAULT;
				dst_prop++;
			}
		}

		if (template_props) {
			ufbxwi_for(const ufbxwi_prop, prop, template_props->props, template_props->capacity) {
				if (prop->token == 0 || prop->token == ~0u) continue;
				*dst_prop = *prop;
				dst_prop->flags |= UFBXWI_PROP_FLAG_TEMP_TEMPLATE;
				dst_prop++;
			}
		}

		ufbxw_assert(dst_prop - props == prop_count);
	}

	// TODO: Better sort here
	qsort(sc->tmp_prop_list.data, prop_count, sizeof(ufbxwi_prop), &ufbxwi_cmp_prop_identity);

	{
		// Resolve properties
		size_t dst_ix = 0;
		for (size_t src_ix = 0; src_ix < prop_count; src_ix++) {
			const ufbxwi_prop *base = &props[src_ix], *tmpl = NULL;
			ufbxwi_token token = base->token;

			// Skip properties with the same token, preferring the first one and look for a template
			while (src_ix + 1 < prop_count && props[src_ix + 1].token == token) {
				if ((props[src_ix + 1].flags & UFBXWI_PROP_FLAG_TEMP_MASK) == UFBXWI_PROP_FLAG_TEMP_TEMPLATE) {
					tmpl = &props[src_ix + 1];
				}
				src_ix++;
			}

			// Skip writing if the value matches the template
			if (tmpl != NULL && tmpl->type == base->type) {
				const void *base_data = ufbxwi_resolve_value_offset(data_buffer, base->value_offset);
				const void *tmpl_data = ufbxwi_resolve_value_offset(template_buffer, tmpl->value_offset);
				size_t data_size = ufbxwi_prop_data_infos[scene->prop_types.data[base->type].data_type].size;
				if (!memcmp(base_data, tmpl_data, data_size)) {
					continue;
				}
			}

			props[dst_ix++] = *base;
		}
		prop_count = dst_ix;
	}

	if (prop_count == 0) {
		return;
	}

	qsort(sc->tmp_prop_list.data, prop_count, sizeof(ufbxwi_prop), &ufbxwi_cmp_prop_order);

	// TODO: Version 6
	ufbxwi_dom_open(sc, "Properties70", "");

	ufbxwi_token prev_prop = UFBXWI_TOKEN_NONE;
	for (size_t i = 0; i < prop_count; i++) {
		const ufbxwi_prop *p = &sc->tmp_prop_list.data[i];
		if (p->token == prev_prop) continue;
		prev_prop = p->token;

		ufbxw_string name = scene->string_pool.tokens.data[p->token];

		const ufbxwi_prop_type *type = &scene->prop_types.data[p->type];
		const void *data = ufbxwi_resolve_value_offset(data_buffer, p->value_offset);

		switch (type->data_type) {
			case UFBXW_PROP_DATA_NONE: {
				ufbxwi_dom_value(sc, "P", "SSSC", name, type->type, type->sub_type, "");
			} break;
			case UFBXW_PROP_DATA_BOOL: {
				const bool *d = (const bool*)data;
				ufbxwi_dom_value(sc, "P", "SSSCI", name, type->type, type->sub_type, "", *d ? 1 : 0);
			} break;
			case UFBXW_PROP_DATA_INT32: {
				const int32_t *d = (const int32_t*)data;
				ufbxwi_dom_value(sc, "P", "SSSCI", name, type->type, type->sub_type, "", *d);
			} break;
			case UFBXW_PROP_DATA_INT64: {
				const int64_t *d = (const int64_t*)data;
				ufbxwi_dom_value(sc, "P", "SSSCL", name, type->type, type->sub_type, "", *d);
			} break;
			case UFBXW_PROP_DATA_REAL: {
				const ufbxw_real *d = (const ufbxw_real*)data;
				ufbxwi_dom_value(sc, "P", "SSSCD", name, type->type, type->sub_type, "", *d);
			} break;
			case UFBXW_PROP_DATA_VEC2: {
				const ufbxw_vec2 *d = (const ufbxw_vec2*)data;
				ufbxwi_dom_value(sc, "P", "SSSCDD", name, type->type, type->sub_type, "", d->x, d->y);
			} break;
			case UFBXW_PROP_DATA_VEC3: {
				const ufbxw_vec3 *d = (const ufbxw_vec3*)data;
				ufbxwi_dom_value(sc, "P", "SSSCDDD", name, type->type, type->sub_type, "", d->x, d->y, d->z);
			} break;
			case UFBXW_PROP_DATA_VEC4: {
				const ufbxw_vec4 *d = (const ufbxw_vec4*)data;
				ufbxwi_dom_value(sc, "P", "SSSCDDDD", name, type->type, type->sub_type, "", d->x, d->y, d->z, d->w);
			} break;
			case UFBXW_PROP_DATA_STRING: {
				const ufbxw_string *d = (const ufbxw_string*)data;
				ufbxwi_dom_value(sc, "P", "SSSCS", name, type->type, type->sub_type, "", *d);
			} break;
			case UFBXW_PROP_DATA_ID: {
				ufbxwi_dom_value(sc, "P", "SSSC", name, type->type, type->sub_type, "");
			} break;
			case UFBXW_PROP_DATA_REAL_STRING: {
				const ufbxw_real_string *d = (const ufbxw_real_string*)data;
				ufbxwi_dom_value(sc, "P", "SSSCDS", name, type->type, type->sub_type, "", d->value, d->string);
			} break;
			case UFBXW_PROP_DATA_BLOB: {
				const ufbxw_blob *d = (const ufbxw_blob*)data;
				ufbxw_assert(0 && "TODO");
			} break;
			case UFBXW_PROP_DATA_USER_INT: {
				const ufbxw_user_int *d = (const ufbxw_user_int*)data;
				ufbxwi_dom_value(sc, "P", "SSSCIII", name, type->type, type->sub_type, "", d->value, d->min_value, d->max_value);
			} break;
			case UFBXW_PROP_DATA_USER_REAL: {
				const ufbxw_user_real *d = (const ufbxw_user_real*)data;
				ufbxwi_dom_value(sc, "P", "SSSCDDD", name, type->type, type->sub_type, "", d->value, d->min_value, d->max_value);
			} break;
			case UFBXW_PROP_DATA_USER_ENUM: {
				const ufbxw_user_enum *d = (const ufbxw_user_enum*)data;
				ufbxwi_dom_value(sc, "P", "SSSCIS", name, type->type, type->sub_type, "", d->value, d->options);
			} break;
		}
	}

	ufbxwi_dom_close(sc);
}

static void ufbxwi_save_template(ufbxw_save_context *sc, ufbxwi_element *element, uint32_t flags)
{
	ufbxwi_template *tmpl = (ufbxwi_template*)element->data;

	ufbxwi_dom_open(sc, "PropertyTemplate", "T", tmpl->type);

	ufbxwi_save_props(sc, tmpl, &element->props, NULL, NULL, NULL);

	ufbxwi_dom_close(sc);
}

static void ufbxwi_save_element(ufbxw_save_context *sc, ufbxwi_element *element, uint32_t flags)
{
	ufbxw_scene *scene = sc->scene;

	ufbxw_id id = element->id;
	ufbxw_element_type type = ufbxwi_id_type(id);
	const ufbxwi_element_type *et = &scene->element_types.data[element->type_id];

	ufbxwi_token fbx_type = et->fbx_type;
	ufbxwi_token sub_type = et->sub_type;
	ufbxwi_token obj_type = et->object_type;

	if (type == UFBXW_ELEMENT_NODE) {
		// TODO
		sub_type = UFBXWI_Mesh;
	}

	ufbxw_string obj_type_str = scene->string_pool.tokens.data[obj_type];
	ufbxw_string fbx_type_str = scene->string_pool.tokens.data[fbx_type];

	if ((flags & UFBXWI_SAVE_ELEMENT_MANUAL_OPEN) == 0) {
		// TODO: Dynamic buffer
		char name[256];
		if (sc->opts.ascii) {
			snprintf(name, sizeof(name), "%s::%s", fbx_type_str.data, element->name.data);
		} else {
		}

		if ((flags & UFBXWI_SAVE_ELEMENT_NO_ID) != 0) {
			ufbxwi_dom_open(sc, obj_type_str.data, "CT", name, sub_type);
		} else {
			ufbxwi_dom_open(sc, obj_type_str.data, "LCT", (int64_t)id, name, sub_type);
		}
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
	} else if (type == UFBXW_ELEMENT_GLOBAL_SETTINGS) {
		ufbxwi_dom_value(sc, "Version", "I", 1000);
	}

	const ufbxwi_element_type *elem_type = &scene->element_types.data[element->type_id];

	const ufbxwi_props *default_props = NULL;
	if (element->flags & UFBXWI_ELEMENT_FLAG_HAS_DEFAULT_PROPS) {
		default_props = &elem_type->props;
	}

	ufbxwi_props *tmpl_props = NULL;
	void *tmpl_data = NULL;
	ufbxwi_element *tmpl_element = ufbxwi_get_element(sc->scene, elem_type->template_id);
	if (tmpl_element) {
		tmpl_props = &tmpl_element->props;
		tmpl_data = tmpl_element->data;
	}

	ufbxwi_save_props(sc, element->data, &element->props, default_props, tmpl_data, tmpl_props);

	if (type == UFBXW_ELEMENT_DOCUMENT) {
		ufbxwi_document *document = (ufbxwi_document*)element->data;
		ufbxwi_dom_value(sc, "RootNode", "L", document->root_node);
	}

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

static void ufbxwi_save_documents(ufbxw_save_context *sc)
{
	ufbxw_scene *scene = sc->scene;

	ufbxwi_dom_open(sc, "Documents", "");

	size_t document_count = 0;

	ufbxwi_for_list(ufbxwi_element, element, scene->elements) {
		ufbxw_element_type type = ufbxwi_id_type(element->id);
		if (type != UFBXW_ELEMENT_DOCUMENT) continue;
		document_count++;
	}

	ufbxwi_dom_value(sc, "Count", "I", (int32_t)document_count);

	ufbxwi_for_list(ufbxwi_element, element, scene->elements) {
		ufbxw_element_type type = ufbxwi_id_type(element->id);
		if (type != UFBXW_ELEMENT_DOCUMENT) continue;
		
		ufbxwi_dom_open(sc, "Document", "LCC", element->id, "", "Scene");
		ufbxwi_save_element(sc, element, UFBXWI_SAVE_ELEMENT_MANUAL_OPEN);
	}

	ufbxwi_dom_close(sc);
}

static void ufbxwi_save_root(ufbxw_save_context *sc)
{
	ufbxw_scene *scene = sc->scene;

	if (sc->opts.ascii) {
		uint32_t major = sc->opts.version / 1000 % 10;
		uint32_t minor = sc->opts.version / 100 % 10;
		uint32_t patch = sc->opts.version % 100;
		ufbxwi_dom_comment(sc, "; FBX %u.%u.%u project file\n; %.52s\n\n", major, minor, patch, ufbxwi_dom_section_str);
	}

	ufbxwi_dom_open(sc, "FBXHeaderExtension", "");
	ufbxwi_dom_value(sc, "FBXHeaderVersion", "I", 1003);
	ufbxwi_dom_value(sc, "FBXVersion", "I", sc->opts.version);

	ufbxwi_save_timestamp(sc);

	ufbxwi_dom_value(sc, "Creator", "C", "ufbx_write (version format TBD)");

	{
		ufbxw_id scene_info_id = ufbxw_get_scene_info_id(scene);
		ufbxwi_element *scene_info = ufbxwi_get_element(scene, scene_info_id);
		if (scene_info) {
			ufbxwi_save_element(sc, scene_info, UFBXWI_SAVE_ELEMENT_NO_ID);
		}
	}

	ufbxwi_dom_close(sc);

	{
		ufbxw_id global_settings_id = ufbxw_get_global_settings_id(scene);
		ufbxwi_element *global_settings = ufbxwi_get_element(scene, global_settings_id);
		if (global_settings) {
			ufbxwi_dom_open(sc, "GlobalSettings", "");
			ufbxwi_save_element(sc, global_settings, UFBXWI_SAVE_ELEMENT_MANUAL_OPEN);
		}
	}

	ufbxwi_dom_section(sc, "Documents Description");
	ufbxwi_save_documents(sc);

	ufbxwi_dom_section(sc, "Document References");

	ufbxwi_dom_open(sc, "References", "");
	ufbxwi_dom_close(sc);

	ufbxwi_dom_section(sc, "Object definitions");

	ufbxwi_dom_open(sc, "Definitions", "");

	ufbxwi_for_list(ufbxwi_element, element, scene->elements) {
		ufbxw_element_type type = ufbxwi_id_type(element->id);
		if (type != UFBXW_ELEMENT_TEMPLATE) continue;

		ufbxwi_save_template(sc, element, 0);
	}

	ufbxwi_dom_close(sc);

	ufbxwi_dom_section(sc, "Object properties");

	ufbxwi_dom_open(sc, "Objects", "");

	ufbxwi_for_list(ufbxwi_element, element, scene->elements) {
		ufbxw_element_type type = ufbxwi_id_type(element->id);
		if (type == UFBXWI_ELEMENT_TYPE_NONE) continue;

		bool do_save = true;
		switch (type) {
		case UFBXW_ELEMENT_DOCUMENT:
		case UFBXW_ELEMENT_TEMPLATE:
		case UFBXW_ELEMENT_SCENE_INFO:
		case UFBXW_ELEMENT_GLOBAL_SETTINGS:
			do_save = false;
			break;
		}

		if (do_save) {
			ufbxwi_save_element(sc, element, 0);
		}
	}

	ufbxwi_dom_close(sc);

	ufbxwi_dom_section(sc, "Object connections");

	ufbxwi_dom_open(sc, "Connections", "");
	ufbxwi_dom_close(sc);

	// Differently formatted section...
	ufbxwi_dom_comment(sc, ";Takes section\n;%.52s\n\n", ufbxwi_dom_section_str);

	ufbxwi_dom_open(sc, "Takes", "");
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

ufbxw_abi_data const ufbxw_string ufbxw_empty_string = { ufbxwi_empty_char, 0 };
ufbxw_abi_data const ufbxw_vec2 ufbxw_zero_vec2 = { 0.0f, 0.0f };
ufbxw_abi_data const ufbxw_vec3 ufbxw_zero_vec3 = { 0.0f, 0.0f, 0.0f };
ufbxw_abi_data const ufbxw_vec4 ufbxw_zero_vec4 = { 0.0f, 0.0f, 0.0f, 0.0f };

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
	return ufbxwi_create_element(scene, type, UFBXWI_TOKEN_NONE);
}

ufbxw_abi ufbxw_id ufbxw_create_element_ex(ufbxw_scene *scene, ufbxw_element_type type, const char *class_type)
{
	return ufbxw_create_element_ex_len(scene, type, class_type, strlen(class_type));
}

ufbxw_abi ufbxw_id ufbxw_create_element_ex_len(ufbxw_scene *scene, ufbxw_element_type type, const char *class_type, size_t class_type_len)
{
	ufbxwi_token class_type_token = ufbxwi_get_token(&scene->string_pool, class_type, class_type_len);
	return ufbxwi_create_element(scene, type, class_type_token);
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

	// TODO: Disconnect everything

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
		if (ufbxwi_id_type(element->id) != UFBXWI_ELEMENT_TYPE_NONE) {
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

ufbxw_abi void ufbxw_set_bool(ufbxw_scene *scene, ufbxw_id id, const char *prop, bool value)
{
	ufbxwi_set_prop(scene, id, prop, strlen(prop), &value, UFBXW_PROP_DATA_BOOL);
}

ufbxw_abi void ufbxw_set_int(ufbxw_scene *scene, ufbxw_id id, const char *prop, int32_t value)
{
	ufbxwi_set_prop(scene, id, prop, strlen(prop), &value, UFBXW_PROP_DATA_INT32);
}

ufbxw_abi void ufbxw_set_int64(ufbxw_scene *scene, ufbxw_id id, const char *prop, int64_t value)
{
	ufbxwi_set_prop(scene, id, prop, strlen(prop), &value, UFBXW_PROP_DATA_INT64);
}

ufbxw_abi void ufbxw_set_real(ufbxw_scene *scene, ufbxw_id id, const char *prop, ufbxw_real value)
{
	ufbxwi_set_prop(scene, id, prop, strlen(prop), &value, UFBXW_PROP_DATA_REAL);
}

ufbxw_abi void ufbxw_set_vec2(ufbxw_scene *scene, ufbxw_id id, const char *prop, ufbxw_vec2 value)
{
	ufbxwi_set_prop(scene, id, prop, strlen(prop), &value, UFBXW_PROP_DATA_VEC2);
}

ufbxw_abi void ufbxw_set_vec3(ufbxw_scene *scene, ufbxw_id id, const char *prop, ufbxw_vec3 value)
{
	ufbxwi_set_prop(scene, id, prop, strlen(prop), &value, UFBXW_PROP_DATA_VEC3);
}

ufbxw_abi void ufbxw_set_vec4(ufbxw_scene *scene, ufbxw_id id, const char *prop, ufbxw_vec4 value)
{
	ufbxwi_set_prop(scene, id, prop, strlen(prop), &value, UFBXW_PROP_DATA_VEC4);
}

ufbxw_abi void ufbxw_set_string(ufbxw_scene *scene, ufbxw_id id, const char *prop, const char *value)
{
	ufbxw_string str;
	if (!ufbxwi_intern_string(&str, &scene->string_pool, value, strlen(value))) return;
	ufbxwi_set_prop(scene, id, prop, strlen(prop), &str, UFBXW_PROP_DATA_STRING);
}

ufbxw_abi void ufbxw_add_bool(ufbxw_scene *scene, ufbxw_id id, const char *prop, ufbxw_prop_type type, bool value)
{
	ufbxwi_add_prop(scene, id, prop, strlen(prop), type, &value, UFBXW_PROP_DATA_BOOL);
}

ufbxw_abi void ufbxw_add_int(ufbxw_scene *scene, ufbxw_id id, const char *prop, ufbxw_prop_type type, int32_t value)
{
	ufbxwi_add_prop(scene, id, prop, strlen(prop), type, &value, UFBXW_PROP_DATA_INT32);
}

ufbxw_abi void ufbxw_add_int64(ufbxw_scene *scene, ufbxw_id id, const char *prop, ufbxw_prop_type type, int64_t value)
{
	ufbxwi_add_prop(scene, id, prop, strlen(prop), type, &value, UFBXW_PROP_DATA_INT64);
}

ufbxw_abi void ufbxw_add_real(ufbxw_scene *scene, ufbxw_id id, const char *prop, ufbxw_prop_type type, ufbxw_real value)
{
	ufbxwi_add_prop(scene, id, prop, strlen(prop), type, &value, UFBXW_PROP_DATA_REAL);
}

ufbxw_abi void ufbxw_add_vec2(ufbxw_scene *scene, ufbxw_id id, const char *prop, ufbxw_prop_type type, ufbxw_vec2 value)
{
	ufbxwi_add_prop(scene, id, prop, strlen(prop), type, &value, UFBXW_PROP_DATA_VEC2);
}

ufbxw_abi void ufbxw_add_vec3(ufbxw_scene *scene, ufbxw_id id, const char *prop, ufbxw_prop_type type, ufbxw_vec3 value)
{
	ufbxwi_add_prop(scene, id, prop, strlen(prop), type, &value, UFBXW_PROP_DATA_VEC3);
}

ufbxw_abi void ufbxw_add_vec4(ufbxw_scene *scene, ufbxw_id id, const char *prop, ufbxw_prop_type type, ufbxw_vec4 value)
{
	ufbxwi_add_prop(scene, id, prop, strlen(prop), type, &value, UFBXW_PROP_DATA_VEC4);
}

ufbxw_abi void ufbxw_add_string(ufbxw_scene *scene, ufbxw_id id, const char *prop, ufbxw_prop_type type, const char *value)
{
	ufbxw_string str;
	if (!ufbxwi_intern_string(&str, &scene->string_pool, value, strlen(value))) return;
	ufbxwi_add_prop(scene, id, prop, strlen(prop), type, &str, UFBXW_PROP_DATA_STRING);
}

ufbxw_abi bool ufbxw_get_bool(ufbxw_scene *scene, ufbxw_id id, const char *prop)
{
	bool ret;
	ufbxwi_get_prop(scene, id, prop, strlen(prop), &ret, UFBXW_PROP_DATA_BOOL);
	return ret;
}

ufbxw_abi int32_t ufbxw_get_int(ufbxw_scene *scene, ufbxw_id id, const char *prop)
{
	int32_t ret;
	ufbxwi_get_prop(scene, id, prop, strlen(prop), &ret, UFBXW_PROP_DATA_INT32);
	return ret;
}

ufbxw_abi int64_t ufbxw_get_int64(ufbxw_scene *scene, ufbxw_id id, const char *prop)
{
	int64_t ret;
	ufbxwi_get_prop(scene, id, prop, strlen(prop), &ret, UFBXW_PROP_DATA_INT64);
	return ret;
}

ufbxw_abi ufbxw_real ufbxw_get_real(ufbxw_scene *scene, ufbxw_id id, const char *prop)
{
	ufbxw_real ret;
	ufbxwi_get_prop(scene, id, prop, strlen(prop), &ret, UFBXW_PROP_DATA_REAL);
	return ret;
}

ufbxw_abi ufbxw_vec2 ufbxw_get_vec2(ufbxw_scene *scene, ufbxw_id id, const char *prop)
{
	ufbxw_vec2 ret;
	ufbxwi_get_prop(scene, id, prop, strlen(prop), &ret, UFBXW_PROP_DATA_VEC2);
	return ret;
}

ufbxw_abi ufbxw_vec3 ufbxw_get_vec3(ufbxw_scene *scene, ufbxw_id id, const char *prop)
{
	ufbxw_vec3 ret;
	ufbxwi_get_prop(scene, id, prop, strlen(prop), &ret, UFBXW_PROP_DATA_VEC3);
	return ret;
}

ufbxw_abi ufbxw_vec4 ufbxw_get_vec4(ufbxw_scene *scene, ufbxw_id id, const char *prop)
{
	ufbxw_vec4 ret;
	ufbxwi_get_prop(scene, id, prop, strlen(prop), &ret, UFBXW_PROP_DATA_VEC4);
	return ret;
}

ufbxw_abi ufbxw_string ufbxw_get_string(ufbxw_scene *scene, ufbxw_id id, const char *prop)
{
	ufbxw_string ret;
	ufbxwi_get_prop(scene, id, prop, strlen(prop), &ret, UFBXW_PROP_DATA_STRING);
	return ret;
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
	ufbxwi_node *data = ufbxwi_get_node_data(scene, node);
	ufbxwi_check_return(data, 0);
	return data->children.count;
}

ufbxw_abi ufbxw_node ufbxw_node_get_child(ufbxw_scene *scene, ufbxw_node node, size_t index)
{
	ufbxwi_node *data = ufbxwi_get_node_data(scene, node);
	ufbxwi_check_return(data, ufbxw_as_node(0));
	ufbxwi_check_return(index < data->children.count, ufbxw_as_node(0));
	return data->children.data[index];
}

ufbxw_abi void ufbxw_node_set_translation(ufbxw_scene *scene, ufbxw_node node, ufbxw_vec3 translation)
{
	ufbxwi_node *data = ufbxwi_get_node_data(scene, node);
	ufbxwi_check(data);
	data->lcl_translation = translation;
}

ufbxw_abi void ufbxw_node_set_rotation(ufbxw_scene *scene, ufbxw_node node, ufbxw_vec3 rotation)
{
	ufbxwi_node *data = ufbxwi_get_node_data(scene, node);
	ufbxwi_check(data);
	data->lcl_rotation = rotation;
}

ufbxw_abi void ufbxw_node_set_scaling(ufbxw_scene *scene, ufbxw_node node, ufbxw_vec3 scaling)
{
	ufbxwi_node *data = ufbxwi_get_node_data(scene, node);
	ufbxwi_check(data);
	data->lcl_scaling = scaling;
}

ufbxw_abi ufbxw_vec3 ufbxw_node_get_translation(ufbxw_scene *scene, ufbxw_node node)
{
	ufbxwi_node *data = ufbxwi_get_node_data(scene, node);
	ufbxwi_check_return(data, ufbxw_zero_vec3);
	return data->lcl_translation;
}

ufbxw_abi ufbxw_vec3 ufbxw_node_get_rotation(ufbxw_scene *scene, ufbxw_node node)
{
	ufbxwi_node *data = ufbxwi_get_node_data(scene, node);
	ufbxwi_check_return(data, ufbxw_zero_vec3);
	return data->lcl_rotation;
}

ufbxw_abi ufbxw_vec3 ufbxw_node_get_scaling(ufbxw_scene *scene, ufbxw_node node)
{
	ufbxwi_node *data = ufbxwi_get_node_data(scene, node);
	ufbxwi_check_return(data, ufbxw_zero_vec3);
	return data->lcl_scaling;
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

ufbxw_abi ufbxw_id ufbxw_get_global_settings_id(ufbxw_scene *scene)
{
	ufbxwi_for_list(ufbxwi_element, element, scene->elements) {
		if (ufbxwi_id_type(element->id) == UFBXW_ELEMENT_GLOBAL_SETTINGS) {
			return element->id;
		}
	}
	return ufbxw_null_id;
}

ufbxw_abi ufbxw_id ufbxw_get_template_id(ufbxw_scene *scene, ufbxw_element_type type)
{
	uint32_t type_id = ufbxwi_find_element_type_id(scene, type, UFBXWI_TOKEN_NONE);
	if (type_id == ~0u) return ufbxw_null_id;
	const ufbxwi_element_type *et = &scene->element_types.data[type_id];
	return et->template_id;
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
