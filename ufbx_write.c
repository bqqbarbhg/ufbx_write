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

#define ufbxwi_fail_err(err, msg) ufbxwi_fail_imp_err((err))

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
	if (list->capacity - count >= n) {
		if (list->data == NULL) list->data = (void*)ufbxwi_zero_size_buffer;
		list->count = count + n;
		return (char*)list->data + size * count;
	}

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
	// TODO: Something better here.. always taking two slow paths in a row to patch non-NULL data pointer
	if (list->capacity - count > n) {
		list->count = count + n;
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

static bool ufbxwi_id_list_remove_one(void *p_list, ufbxw_id id)
{
	ufbxwi_id_list *list = (ufbxwi_id_list*)p_list;
	ufbxw_id *begin = list->data, *dst = begin, *end = begin + list->count;
	for (; dst != end; dst++) {
		if (*dst == id) break;
	}
	if (dst == end) return false;

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
	UFBXWI_AnimCurve,
	UFBXWI_AnimCurveNode,
	UFBXWI_AnimLayer,
	UFBXWI_AnimStack,
	UFBXWI_AnimationCurve,
	UFBXWI_AnimationCurveNode,
	UFBXWI_AnimationLayer,
	UFBXWI_AnimationStack,
	UFBXWI_AreaLightShape,
	UFBXWI_AxisLen,
	UFBXWI_BBoxMax,
	UFBXWI_BBoxMin,
	UFBXWI_BlendMode,
	UFBXWI_BlendModeBypass,
	UFBXWI_BottomBarnDoor,
	UFBXWI_Bump,
	UFBXWI_BumpFactor,
	UFBXWI_CastLightOnObject,
	UFBXWI_CastShadows,
	UFBXWI_Casts_Shadows,
	UFBXWI_Color,
	UFBXWI_CurrentMappingType,
	UFBXWI_CurrentTextureBlendMode,
	UFBXWI_DecayStart,
	UFBXWI_DecayType,
	UFBXWI_DefaultAttributeIndex,
	UFBXWI_Description,
	UFBXWI_DiffuseColor,
	UFBXWI_DiffuseFactor,
	UFBXWI_DisplacementColor,
	UFBXWI_DisplacementFactor,
	UFBXWI_Document,
	UFBXWI_DrawFrontFacingVolumetricLight,
	UFBXWI_DrawGroundProjection,
	UFBXWI_DrawVolumetricLight,
	UFBXWI_EmissiveColor,
	UFBXWI_EmissiveFactor,
	UFBXWI_EnableBarnDoor,
	UFBXWI_EnableFarAttenuation,
	UFBXWI_EnableNearAttenuation,
	UFBXWI_FarAttenuationEnd,
	UFBXWI_FarAttenuationStart,
	UFBXWI_FbxAnimCurveNode,
	UFBXWI_FbxAnimLayer,
	UFBXWI_FbxAnimStack,
	UFBXWI_FbxFileTexture,
	UFBXWI_FbxLight,
	UFBXWI_FbxMesh,
	UFBXWI_FbxNode,
	UFBXWI_FbxSurfaceLambert,
	UFBXWI_FileName,
	UFBXWI_Fog,
	UFBXWI_Freeze,
	UFBXWI_GeometricRotation,
	UFBXWI_GeometricScaling,
	UFBXWI_GeometricTranslation,
	UFBXWI_Geometry,
	UFBXWI_GlobalSettings,
	UFBXWI_InheritType,
	UFBXWI_InnerAngle,
	UFBXWI_Intensity,
	UFBXWI_LODBox,
	UFBXWI_Lcl_Rotation,
	UFBXWI_Lcl_Scaling,
	UFBXWI_Lcl_Translation,
	UFBXWI_LeftBarnDoor,
	UFBXWI_Light,
	UFBXWI_LightType,
	UFBXWI_LocalStart,
	UFBXWI_LocalStop,
	UFBXWI_Lock,
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
	UFBXWI_Mute,
	UFBXWI_NearAttenuationEnd,
	UFBXWI_NearAttenuationStart,
	UFBXWI_NegativePercentShapeSupport,
	UFBXWI_NodeAttribute,
	UFBXWI_NormalMap,
	UFBXWI_OuterAngle,
	UFBXWI_Path,
	UFBXWI_PostRotation,
	UFBXWI_PreRotation,
	UFBXWI_PreferedAngleX,
	UFBXWI_PreferedAngleY,
	UFBXWI_PreferedAngleZ,
	UFBXWI_PremultiplyAlpha,
	UFBXWI_Primary_Visibility,
	UFBXWI_QuaternionInterpolate,
	UFBXWI_Receive_Shadows,
	UFBXWI_ReferenceStart,
	UFBXWI_ReferenceStop,
	UFBXWI_RelPath,
	UFBXWI_RightBarnDoor,
	UFBXWI_Rotation,
	UFBXWI_RotationAccumulationMode,
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
	UFBXWI_ScaleAccumulationMode,
	UFBXWI_Scaling,
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
	UFBXWI_ShadowColor,
	UFBXWI_Show,
	UFBXWI_Solo,
	UFBXWI_SourceObject,
	UFBXWI_Texture,
	UFBXWI_Texture_alpha,
	UFBXWI_TextureRotationPivot,
	UFBXWI_TextureScalingPivot,
	UFBXWI_TextureTypeUse,
	UFBXWI_TopBarnDoor,
	UFBXWI_Translation,
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
	UFBXWI_UVSet,
	UFBXWI_UVSwap,
	UFBXWI_UpVectorProperty,
	UFBXWI_UseMaterial,
	UFBXWI_UseMipMap,
	UFBXWI_UserData,
	UFBXWI_VectorDisplacementColor,
	UFBXWI_VectorDisplacementFactor,
	UFBXWI_Visibility,
	UFBXWI_Visibility_Inheritance,
	UFBXWI_Weight,
	UFBXWI_WrapModeU,
	UFBXWI_WrapModeV,
	UFBXWI_d,
	UFBXWI_d_X,
	UFBXWI_d_Y,
	UFBXWI_d_Z,
	UFBXWI_TOKEN_COUNT,
	UFBXWI_TOKEN_FORCE_32BIT = 0x7fffffff,
} ufbxwi_token;

static const ufbxw_string ufbxwi_tokens[] = {
	{ "", 0 },
	{ "ActiveAnimStackName", 19 },
	{ "AmbientColor", 12 },
	{ "AmbientFactor", 13 },
	{ "AnimCurve", 9 },
	{ "AnimCurveNode", 13 },
	{ "AnimLayer", 9 },
	{ "AnimStack", 9 },
	{ "AnimationCurve", 14 },
	{ "AnimationCurveNode", 18 },
	{ "AnimationLayer", 14 },
	{ "AnimationStack", 14 },
	{ "AreaLightShape", 14 },
	{ "AxisLen", 7 },
	{ "BBoxMax", 7 },
	{ "BBoxMin", 7 },
	{ "BlendMode", 9 },
	{ "BlendModeBypass", 15 },
	{ "BottomBarnDoor", 14 },
	{ "Bump", 4 },
	{ "BumpFactor", 10 },
	{ "CastLightOnObject", 17 },
	{ "CastShadows", 11 },
	{ "Casts Shadows", 13 },
	{ "Color", 5 },
	{ "CurrentMappingType", 18 },
	{ "CurrentTextureBlendMode", 23 },
	{ "DecayStart", 10 },
	{ "DecayType", 9 },
	{ "DefaultAttributeIndex", 21 },
	{ "Description", 11 },
	{ "DiffuseColor", 12 },
	{ "DiffuseFactor", 13 },
	{ "DisplacementColor", 17 },
	{ "DisplacementFactor", 18 },
	{ "Document", 8 },
	{ "DrawFrontFacingVolumetricLight", 30 },
	{ "DrawGroundProjection", 20 },
	{ "DrawVolumetricLight", 19 },
	{ "EmissiveColor", 13 },
	{ "EmissiveFactor", 14 },
	{ "EnableBarnDoor", 14 },
	{ "EnableFarAttenuation", 20 },
	{ "EnableNearAttenuation", 21 },
	{ "FarAttenuationEnd", 17 },
	{ "FarAttenuationStart", 19 },
	{ "FbxAnimCurveNode", 16 },
	{ "FbxAnimLayer", 12 },
	{ "FbxAnimStack", 12 },
	{ "FbxFileTexture", 14 },
	{ "FbxLight", 8 },
	{ "FbxMesh", 7 },
	{ "FbxNode", 7 },
	{ "FbxSurfaceLambert", 17 },
	{ "FileName", 8 },
	{ "Fog", 3 },
	{ "Freeze", 6 },
	{ "GeometricRotation", 17 },
	{ "GeometricScaling", 16 },
	{ "GeometricTranslation", 20 },
	{ "Geometry", 8 },
	{ "GlobalSettings", 14 },
	{ "InheritType", 11 },
	{ "InnerAngle", 10 },
	{ "Intensity", 9 },
	{ "LODBox", 6 },
	{ "Lcl Rotation", 12 },
	{ "Lcl Scaling", 11 },
	{ "Lcl Translation", 15 },
	{ "LeftBarnDoor", 12 },
	{ "Light", 5 },
	{ "LightType", 9 },
	{ "LocalStart", 10 },
	{ "LocalStop", 9 },
	{ "Lock", 4 },
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
	{ "Mute", 4 },
	{ "NearAttenuationEnd", 18 },
	{ "NearAttenuationStart", 20 },
	{ "NegativePercentShapeSupport", 27 },
	{ "NodeAttribute", 13 },
	{ "NormalMap", 9 },
	{ "OuterAngle", 10 },
	{ "Path", 4 },
	{ "PostRotation", 12 },
	{ "PreRotation", 11 },
	{ "PreferedAngleX", 14 },
	{ "PreferedAngleY", 14 },
	{ "PreferedAngleZ", 14 },
	{ "PremultiplyAlpha", 16 },
	{ "Primary Visibility", 18 },
	{ "QuaternionInterpolate", 21 },
	{ "Receive Shadows", 15 },
	{ "ReferenceStart", 14 },
	{ "ReferenceStop", 13 },
	{ "RelPath", 7 },
	{ "RightBarnDoor", 13 },
	{ "Rotation", 8 },
	{ "RotationAccumulationMode", 24 },
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
	{ "ScaleAccumulationMode", 21 },
	{ "Scaling", 7 },
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
	{ "ShadowColor", 11 },
	{ "Show", 4 },
	{ "Solo", 4 },
	{ "SourceObject", 12 },
	{ "Texture", 7 },
	{ "Texture alpha", 13 },
	{ "TextureRotationPivot", 20 },
	{ "TextureScalingPivot", 19 },
	{ "TextureTypeUse", 14 },
	{ "TopBarnDoor", 11 },
	{ "Translation", 11 },
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
	{ "UVSet", 5 },
	{ "UVSwap", 6 },
	{ "UpVectorProperty", 16 },
	{ "UseMaterial", 11 },
	{ "UseMipMap", 9 },
	{ "UserData", 8 },
	{ "VectorDisplacementColor", 23 },
	{ "VectorDisplacementFactor", 24 },
	{ "Visibility", 10 },
	{ "Visibility Inheritance", 22 },
	{ "Weight", 6 },
	{ "WrapModeU", 9 },
	{ "WrapModeV", 9 },
	{ "d", 1 },
	{ "d|X", 3 },
	{ "d|Y", 3 },
	{ "d|Z", 3 },
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

// -- Buffers

UFBXWI_LIST_TYPE(ufbxwi_uint32_list, uint32_t);

#define ufbxwi_empty_int_buffer ((ufbxw_int_buffer){NULL,0})
#define ufbxwi_empty_vec3_buffer ((ufbxw_vec3_buffer){NULL,0})

typedef enum {
	UFBXWI_BUFFER_TYPE_NONE,
	UFBXWI_BUFFER_TYPE_INT,
	UFBXWI_BUFFER_TYPE_LONG,
	UFBXWI_BUFFER_TYPE_REAL,
	UFBXWI_BUFFER_TYPE_VEC2,
	UFBXWI_BUFFER_TYPE_VEC3,
	UFBXWI_BUFFER_TYPE_VEC4,
} ufbxwi_buffer_type;

static const uint8_t ufbxwi_buffer_type_size[] = {
	0,
	sizeof(int32_t),
	sizeof(int64_t),
	sizeof(ufbxw_real),
	sizeof(ufbxw_vec2),
	sizeof(ufbxw_vec3),
	sizeof(ufbxw_vec4),
};

static ufbxwi_forceinline ufbxw_id ufbxwi_make_buffer_id(ufbxwi_buffer_type type, uint32_t generation, size_t index)
{
	ufbxw_assert((uint64_t)index < ((uint64_t)1u << 32u));
	return (ufbxw_id)(((uint64_t)type << 48) | ((uint64_t)generation << 32) | (index));
}

#define ufbxwi_buffer_id_index(id) (uint32_t)(id)
#define ufbxwi_buffer_id_type(id) (ufbxw_element_type)(((id) >> 48))
#define ufbxwi_buffer_id_generation(id) (uint32_t)(((id) >> 32) & 0xffff)

typedef enum {
	UFBXWI_BUFFER_STATE_NONE,
	UFBXWI_BUFFER_STATE_OWNED,
	UFBXWI_BUFFER_STATE_EXTERNAL,
	UFBXWI_BUFFER_STATE_STREAM,
} ufbxwi_buffer_state;

typedef union {
	ufbxw_int_stream_fn *int_fn;
	ufbxw_vec3_stream_fn *vec3_fn;
} ufbxwi_stream_fn;

typedef struct {
	ufbxw_buffer_id id;
	ufbxwi_buffer_state state;
	size_t count;

	uint32_t refcount;
	uint32_t user_refcount;

	ufbxw_buffer_deleter_fn *deleter_fn;
	void *deleter_user;

	union {
		struct {
			void *data;
			size_t alloc_size;
		} owned;
		struct {
			const void *data;
			size_t data_size;
		} external;
		struct {
			ufbxwi_stream_fn fn;
			void *user;
		} stream;
	} data;
} ufbxwi_buffer;

UFBXW_LIST_TYPE(ufbxwi_buffer_list, ufbxwi_buffer);

typedef struct {
	ufbxwi_error *error;
	ufbxwi_allocator *ator;

	ufbxwi_buffer_list buffers;
	ufbxwi_uint32_list free_buffer_ids;

} ufbxwi_buffer_pool;

static void ufbxwi_buffer_pool_init(ufbxwi_buffer_pool *pool, ufbxwi_allocator *ator, ufbxwi_error *error)
{
	pool->ator = ator;
	pool->error = error;
}

static ufbxwi_forceinline ufbxwi_buffer *ufbxwi_get_buffer(ufbxwi_buffer_pool *pool, ufbxw_buffer_id id)
{
	size_t index = ufbxwi_buffer_id_index(id);
	if (index >= pool->buffers.count) return NULL;
	ufbxwi_buffer *buffer = &pool->buffers.data[index];
	if (buffer->id != id) return NULL;
	return buffer;
}

static ufbxwi_forceinline size_t ufbxwi_get_buffer_size(ufbxwi_buffer_pool *pool, ufbxw_buffer_id id)
{
	ufbxwi_buffer *buffer = ufbxwi_get_buffer(pool, id);
	return buffer ? buffer->count : 0;
}

static void ufbxwi_reset_buffer(ufbxwi_buffer_pool *pool, ufbxwi_buffer *buffer)
{
	if (buffer->deleter_fn != NULL) {
		void *deleter_data = NULL;
		if (buffer->state == UFBXWI_BUFFER_STATE_EXTERNAL) {
			deleter_data = (void*)buffer->data.external.data;
		}
		buffer->deleter_fn(buffer->deleter_user, deleter_data);
	}

	if (buffer->state == UFBXWI_BUFFER_STATE_OWNED) {
		ufbxwi_free(pool->ator, buffer->data.owned.data);
	}

	buffer->deleter_fn = NULL;
	buffer->deleter_user = NULL;
	buffer->state = UFBXWI_BUFFER_STATE_NONE;
	memset(&buffer->data, 0, sizeof(buffer->data));
}

static size_t ufbxwi_buffer_stream_read(void *dst, size_t dst_count, size_t offset, ufbxwi_buffer_type type, ufbxwi_stream_fn fn, void *user)
{
	switch (type) {
	case UFBXWI_BUFFER_TYPE_INT:
		return fn.int_fn(user, dst, dst_count, offset);
		break;
	case UFBXWI_BUFFER_TYPE_VEC3:
		return fn.vec3_fn(user, dst, dst_count, offset);
		break;
	}
	return SIZE_MAX;
}

static bool ufbxwi_buffer_read_to(ufbxwi_buffer_pool *pool, ufbxwi_buffer *buffer, void *dst, size_t dst_size)
{
	ufbxwi_buffer_type type = ufbxwi_buffer_id_type(buffer->id);
	size_t type_size = ufbxwi_buffer_type_size[type];

	size_t read_size = buffer->count * type_size;
	ufbxw_assert(dst_size >= read_size);

	switch (buffer->state) {
	case UFBXWI_BUFFER_STATE_NONE:
		return false;
	case UFBXWI_BUFFER_STATE_OWNED:
		memcpy(dst, buffer->data.owned.data, read_size);
		return true;
	case UFBXWI_BUFFER_STATE_EXTERNAL:
		memcpy(dst, buffer->data.external.data, buffer->count * type_size);
		return true;
	case UFBXWI_BUFFER_STATE_STREAM:
		for (size_t offset = 0; offset < buffer->count; ) {
			void *dst_off = (char*)dst + offset * type_size;
			size_t dst_count = buffer->count - offset;
			size_t num_read = ufbxwi_buffer_stream_read(dst_off, dst_count, offset, type, buffer->data.stream.fn, buffer->data.stream.user);
			if (num_read == 0 || num_read == SIZE_MAX) return false;
			ufbxw_assert(num_read < dst_count);
			offset += num_read;
		}
		return true;
	}

	return false;
}

static ufbxw_buffer_id ufbxwi_create_buffer(ufbxwi_buffer_pool *pool, ufbxwi_buffer_type type)
{
	size_t index = 0;
	if (pool->free_buffer_ids.count > 0) {
		index = pool->free_buffer_ids.data[--pool->free_buffer_ids.count];
	} else {
		index = pool->buffers.count;
		ufbxwi_check_return_err(pool->error, ufbxwi_list_push_zero(pool->ator, &pool->buffers, ufbxwi_buffer), 0);
	}

	ufbxwi_buffer *buffer = &pool->buffers.data[index];
	uint32_t generation = ufbxwi_buffer_id_generation(buffer->id) + 1;
	ufbxw_buffer_id id = ufbxwi_make_buffer_id(type, generation, index);

	buffer->id = id;
	buffer->refcount = 1;

	return id;
}

static ufbxw_buffer_id ufbxwi_create_ownded_buffer(ufbxwi_buffer_pool *pool, ufbxwi_buffer_type type, size_t count)
{
	ufbxw_buffer_id id = ufbxwi_create_buffer(pool, type);
	ufbxwi_buffer *buffer = ufbxwi_get_buffer(pool, id);
	if (!buffer) return id;

	size_t alloc_size = 0;
	size_t type_size = ufbxwi_buffer_type_size[type];
	void *data = ufbxwi_alloc_size(pool->ator, type_size, count, &alloc_size);
	ufbxwi_check_return_err(pool->error, data, 0);

	buffer->state = UFBXWI_BUFFER_STATE_OWNED;
	buffer->data.owned.data = data;
	buffer->data.owned.alloc_size = alloc_size;

	return id;
}

static ufbxw_buffer_id ufbxwi_create_copy_buffer(ufbxwi_buffer_pool *pool, ufbxwi_buffer_type type, const void *data, size_t count)
{
	ufbxw_buffer_id id = ufbxwi_create_ownded_buffer(pool, type, count);
	ufbxwi_buffer *buffer = ufbxwi_get_buffer(pool, id);
	if (!buffer) return id;

	ufbxw_assert(buffer->state == UFBXWI_BUFFER_STATE_OWNED);
	size_t type_size = ufbxwi_buffer_type_size[type];
	memcpy(buffer->data.owned.data, data, count * type_size);

	return id;
}

static ufbxw_buffer_id ufbxwi_create_external_buffer(ufbxwi_buffer_pool *pool, ufbxwi_buffer_type type, const void *data, size_t count)
{
	ufbxw_buffer_id id = ufbxwi_create_buffer(pool, type);
	ufbxwi_buffer *buffer = ufbxwi_get_buffer(pool, id);
	if (!buffer) return id;

	buffer->state = UFBXWI_BUFFER_STATE_EXTERNAL;
	buffer->data.external.data = data;
	buffer->data.external.data_size = count * ufbxwi_buffer_type_size[type];

	return id;
}

static ufbxw_buffer_id ufbxwi_create_stream_buffer(ufbxwi_buffer_pool *pool, ufbxwi_buffer_type type, ufbxwi_stream_fn fn, void *user)
{
	ufbxw_buffer_id id = ufbxwi_create_buffer(pool, type);
	ufbxwi_buffer *buffer = ufbxwi_get_buffer(pool, id);
	if (!buffer) return id;

	buffer->state = UFBXWI_BUFFER_STATE_STREAM;
	buffer->data.stream.fn = fn;
	buffer->data.stream.user = user;

	return id;
}

static void ufbxwi_delete_buffer_imp(ufbxwi_buffer_pool *pool, ufbxw_buffer_id id)
{
	ufbxwi_buffer *buffer = ufbxwi_get_buffer(pool, id);
	if (!buffer) return;

	uint32_t generation = ufbxwi_buffer_id_generation(buffer->id);
	memset(buffer, 0, sizeof(ufbxwi_buffer));
	buffer->id = ufbxwi_make_buffer_id(UFBXWI_BUFFER_TYPE_NONE, generation, 0);

	if (generation < 0xffff) {
		// We can leak the index safely here if we fail to allocate
		uint32_t index = ufbxwi_buffer_id_index(id);
		ufbxwi_ignore(ufbxwi_list_push_copy(pool->ator, &pool->free_buffer_ids, uint32_t, &index));
	}
}

static void ufbxwi_free_buffer(ufbxwi_buffer_pool *pool, ufbxw_buffer_id id)
{
	if (!id) return;
	ufbxwi_buffer *buffer = ufbxwi_get_buffer(pool, id);
	if (!buffer) return;

	ufbxw_assert(buffer->refcount > 0);
	if (--buffer->refcount == 0) {
		ufbxwi_delete_buffer_imp(pool, id);
	}
}

static ufbxwi_forceinline ufbxw_buffer_id ufbxwi_make_user_buffer(ufbxwi_buffer_pool *pool, ufbxw_buffer_id src)
{
	ufbxwi_buffer *buffer = ufbxwi_get_buffer(pool, src);
	if (buffer) {
		buffer->user_refcount++;
		buffer->refcount++;
	}
	return src;
}

static void ufbxwi_set_buffer(ufbxwi_buffer_pool *pool, ufbxw_buffer_id *p_dst, ufbxw_buffer_id src)
{
	if (src != 0) {
		ufbxwi_buffer *buffer = ufbxwi_get_buffer(pool, src);
		ufbxw_assert(buffer && buffer->refcount > 0);
	}

	if (*p_dst == src) return;
	ufbxwi_free_buffer(pool, *p_dst);
	*p_dst = src;
}

static void ufbxwi_set_buffer_from_user(ufbxwi_buffer_pool *pool, ufbxw_buffer_id *p_dst, ufbxw_buffer_id src)
{
	ufbxwi_buffer *buffer = ufbxwi_get_buffer(pool, src);
	ufbxw_assert(buffer);
	ufbxw_assert(buffer->user_refcount > 0);
	ufbxw_assert(buffer->refcount > 0);
	buffer->user_refcount--;

	if (*p_dst == src) return;
	ufbxwi_free_buffer(pool, *p_dst);
	*p_dst = src;
}

static bool ufbxwi_make_buffer_owned(ufbxwi_buffer_pool *pool, ufbxw_buffer_id id)
{
	ufbxwi_buffer *buffer = ufbxwi_get_buffer(pool, id);
	if (buffer->state == UFBXWI_BUFFER_STATE_OWNED) return true;

	ufbxwi_buffer_type type = ufbxwi_buffer_id_type(id);
	size_t type_size = ufbxwi_buffer_type_size[type];
	size_t alloc_size = 0;
	void *data = ufbxwi_alloc_size(pool->ator, type_size, buffer->count, &alloc_size);
	ufbxwi_check_return_err(pool->error, data, false);

	size_t read_size = buffer->count * type_size;
	if (!ufbxwi_buffer_read_to(pool, buffer, data, read_size)) {
		ufbxwi_free(pool->ator, data);
		ufbxwi_fail_err(pool->error, "failed to make buffer owned");
		return false;
	}

	ufbxwi_reset_buffer(pool, buffer);
	buffer->state = UFBXWI_BUFFER_STATE_OWNED;
	buffer->data.owned.data = data;
	buffer->data.owned.alloc_size = alloc_size;

	return true;
}

static ufbxwi_forceinline ufbxw_buffer_id ufbxwi_to_user_buffer(ufbxwi_buffer_pool *pool, ufbxw_buffer_id id)
{
	ufbxwi_buffer *buffer = ufbxwi_get_buffer(pool, id);
	if (buffer) {
		buffer->user_refcount++;
	}
	return id;
}

static ufbxwi_forceinline ufbxw_int_buffer ufbxwi_to_user_int_buffer(ufbxwi_buffer_pool *pool, ufbxw_buffer_id id) { ufbxw_int_buffer b = { ufbxwi_to_user_buffer(pool, id) }; return b; }
static ufbxwi_forceinline ufbxw_vec3_buffer ufbxwi_to_user_vec3_buffer(ufbxwi_buffer_pool *pool, ufbxw_buffer_id id) { ufbxw_vec3_buffer b = { ufbxwi_to_user_buffer(pool, id) }; return b; }

#if 0

typedef enum {
	UFBXWI_BUFFER_SOURCE_NONE,
	UFBXWI_BUFFER_SOURCE_ARRAY,
	UFBXWI_BUFFER_SOURCE_STREAM,
	UFBXWI_BUFFER_SOURCE_CONST,
} ufbxwi_buffer_type;

enum {
	UFBXWI_BUFFER_FLAG_DEFERRED = 0x1,
	UFBXWI_BUFFER_FLAG_ALLOCATED = 0x2,
	UFBXWI_BUFFER_FLAG_TRANSIENT = 0x4,
	UFBXWI_BUFFER_FLAG_RETAINED = 0x8,
};

typedef struct ufbxwi_buffer_pool ufbxwi_buffer_pool;

typedef uint32_t ufbxwi_buffer_id;
typedef uint32_t ufbxwi_buffer_transform_id;

#define UFBXWI_BUFFER_TRANSFORM_MAX_SRC 2
#define UFBXWI_BUFFER_TRANSFORM_MAX_DST 2

typedef struct {
	void *begin;
	void *end;
	size_t offset;
	ufbxwi_buffer_id id;
} ufbxwi_buffer_range;

typedef void ufbxwi_buffer_cast_fn(void *dst, const void *src, size_t count);

typedef union {
	struct {
		void *data;
		ufbxw_buffer_data_imp imp;
	} user;
	struct {
		ufbxwi_buffer_cast_fn *fn;
		uint8_t src_size, dst_size, components;
	} cast;
} ufbxwi_buffer_transform_params;

typedef struct {
	ufbxwi_buffer_range src[UFBXWI_BUFFER_TRANSFORM_MAX_SRC];
	ufbxwi_buffer_range dst[UFBXWI_BUFFER_TRANSFORM_MAX_DST];
	ufbxwi_buffer_transform_params params;
} ufbxwi_buffer_transform;

typedef bool ufbxwi_buffer_transform_fn(ufbxwi_buffer_pool *pool, ufbxwi_buffer_transform *transform);

typedef struct {
	ufbxwi_buffer_id src[UFBXWI_BUFFER_TRANSFORM_MAX_SRC];
	ufbxwi_buffer_id dst[UFBXWI_BUFFER_TRANSFORM_MAX_DST];
	ufbxwi_buffer_transform_fn *fn;
	ufbxwi_buffer_transform_params params;
} ufbxwi_buffer_transform_data;

typedef struct {
	ufbxwi_buffer_transform_fn *fn;
	ufbxwi_buffer_id src[2];
	ufbxwi_buffer_id dst[2];
	ufbxwi_buffer_transform_params params;
} ufbxwi_buffer_transform_desc;

typedef struct {
	void *allocation;
	void *data;

	uint32_t count;
	uint32_t buffer_capacity;
	uint32_t buffer_offset;
	uint32_t buffer_count;

	ufbxwi_buffer_type type;
	uint32_t flags;
	ufbxwi_buffer_transform_id src_transform;
	ufbxwi_buffer_transform_id dst_transform;
	int32_t refcount;
} ufbxwi_buffer;

typedef struct {
	ufbxwi_buffer_id id;
	size_t count;
	void *data;
} ufbxwi_buffer_ref;

UFBXW_LIST_TYPE(ufbxwi_buffer_list, ufbxwi_buffer);
UFBXW_LIST_TYPE(ufbxwi_buffer_transform_data_list, ufbxwi_buffer_transform_data);

struct ufbxwi_buffer_pool {
	ufbxwi_error *error;
	ufbxwi_allocator *ator;
	ufbxwi_buffer_list buffers;
	ufbxwi_buffer_transform_data_list transforms;
	ufbxwi_uint32_list free_buffer_ids;
	ufbxwi_uint32_list free_transform_ids;
};

static void ufbxwi_cast_float_real(void *v_dst, const void *v_src, size_t count)
{
	const float *src = (const float*)v_src;
	ufbxw_real *dst = (ufbxw_real*)v_dst;
	for (size_t i = 0; i < count; i++) {
		dst[i] = (ufbxw_real)src[i];
	}
}

static bool ufbxwi_transform_cast(ufbxwi_buffer_pool *pool, ufbxwi_buffer_transform *transform)
{
	const char *src = (const char*)transform->src[0].begin;
	char *dst = (char*)transform->dst[0].begin;

	size_t src_items = ufbxwi_to_size((const char*)transform->src[0].end - src) / transform->params.cast.src_size;
	size_t dst_items = ufbxwi_to_size((char*)transform->dst[0].end - dst) / transform->params.cast.dst_size;
	size_t num_items = ufbxwi_min_sz(src_items, dst_items);

	transform->params.cast.fn(dst, src, num_items * transform->params.cast.components);

	transform->src[0].begin = (void*)(src + num_items * transform->params.cast.src_size);
	transform->dst[0].begin = (void*)(dst + num_items * transform->params.cast.dst_size);
	return true;
}

static bool ufbxwi_transform_stream(ufbxwi_buffer_pool *pool, ufbxwi_buffer_transform *transform)
{
	ufbxwi_buffer_type type = pool->buffers.data[transform->dst[0].id].type;
	char *dst = (char*)transform->dst[0].begin;
	size_t count = ufbxwi_to_size((char*)transform->dst[0].end - dst) / ufbxwi_buffer_type_size[type];
	size_t offset = transform->dst[0].offset;
	size_t num_read = 0;

	switch (type) {
	case UFBXWI_BUFFER_TYPE_INT32:
		num_read = transform->params.user.imp.stream_int32(transform->params.user.data, (int32_t*)dst, offset, count);
		break;
	}

	transform->dst[0].begin = (void*)(dst + num_read);
	return true;
}

static bool ufbxwi_transform_const(ufbxwi_buffer_pool *pool, ufbxwi_buffer_transform *transform)
{
	ufbxwi_buffer_type type = pool->buffers.data[transform->dst[0].id].type;
	char *dst = (char*)transform->dst[0].begin;
	size_t count = ufbxwi_to_size((char*)transform->dst[0].end - dst) / ufbxwi_buffer_type_size[type];

	switch (type) {
	case UFBXWI_BUFFER_TYPE_INT32: {
		int32_t value = transform->params.user.imp.const_int32;
		for (int32_t *d = (int32_t*)dst, *end = d + count; d != end; d++) {
			*d = value;
		}
	} break;
	}

	transform->dst[0].begin = (void*)(dst + count * ufbxwi_buffer_type_size[type]);
	return true;
}

static void ufbxwi_buffer_pool_init(ufbxwi_buffer_pool *pool, ufbxwi_allocator *ator, ufbxwi_error *error)
{
	pool->ator = ator;
	pool->error = error;

	// Push NULL entries
	ufbxwi_list_push_zero(pool->ator, &pool->buffers, ufbxwi_buffer);
	ufbxwi_list_push_zero(pool->ator, &pool->transforms, ufbxwi_buffer_transform_data);
}

static ufbxwi_buffer_transform_id ufbxwi_buffer_add_transform(ufbxwi_buffer_pool *pool, const ufbxwi_buffer_transform_desc *desc)
{
	ufbxwi_buffer_transform_id id = 0;
	if (pool->free_transform_ids.count > 0) {
		id = pool->free_transform_ids.data[--pool->free_transform_ids.count];
	} else {
		id = (ufbxwi_buffer_transform_id)pool->transforms.count;
		ufbxwi_check_return_err(pool->error, ufbxwi_list_push_zero(pool->ator, &pool->transforms, ufbxwi_buffer_transform_data), 0);
	}

	ufbxwi_buffer_transform_data *transform = &pool->transforms.data[id];
	transform->fn = desc->fn;
	transform->params = desc->params;

	for (size_t i = 0; i < UFBXWI_BUFFER_TRANSFORM_MAX_SRC; i++) {
		ufbxwi_buffer_id buf_id = desc->src[i];
		if (buf_id == 0) break;

		ufbxwi_buffer *buf = &pool->buffers.data[buf_id];
		ufbxw_assert(!buf->dst_transform);
		buf->dst_transform = id;
		buf->refcount++;

		transform->src[i] = buf_id;
	}

	for (size_t i = 0; i < UFBXWI_BUFFER_TRANSFORM_MAX_DST; i++) {
		ufbxwi_buffer_id buf_id = desc->dst[i];
		if (buf_id == 0) break;

		ufbxwi_buffer *buf = &pool->buffers.data[buf_id];
		ufbxw_assert(!buf->src_transform);
		buf->src_transform = id;

		transform->dst[i] = buf_id;
	}

	return id;
}

static void ufbxwi_buffer_free(ufbxwi_buffer_pool *pool, ufbxwi_buffer_id id)
{
	if (id == 0) return;

	ufbxwi_buffer *buffer = &pool->buffers.data[id];
	if (--buffer->refcount > 0) return;

	memset(buffer, 0, sizeof(ufbxwi_buffer));

	uint32_t *p_free_id = ufbxwi_list_push_uninit(pool->ator, &pool->free_buffer_ids, uint32_t);
	ufbxwi_check_err(pool->error, p_free_id);
	*p_free_id = id;
}

static void ufbxwi_buffer_lose(ufbxwi_buffer_pool *pool, ufbxwi_buffer_id id)
{
	if (id == 0) return;

	ufbxwi_buffer *buffer = &pool->buffers.data[id];
	if ((buffer->flags & UFBXWI_BUFFER_FLAG_DEFERRED) == 0) {
	}
}

static ufbxwi_buffer_id ufbxwi_buffer_add(ufbxwi_buffer_pool *pool, ufbxwi_buffer_type type, const ufbxw_buffer *source)
{
	ufbxwi_buffer_type source_type = source->data.type;
	if (source && type != source_type) {
		ufbxw_buffer dst_buf = { source->count, { NULL, type, UFBXWI_BUFFER_SOURCE_NONE }};

		ufbxwi_buffer_id src = ufbxwi_buffer_add(pool, source_type, source);
		ufbxwi_buffer_id dst = ufbxwi_buffer_add(pool, type, &dst_buf);
		ufbxwi_check_return_err(pool->error, src && dst, 0);

		ufbxwi_buffer_transform_desc transform = { &ufbxwi_transform_cast };
		transform.params.cast.src_size = ufbxwi_buffer_type_size[source_type];
		transform.params.cast.dst_size = ufbxwi_buffer_type_size[type];

		if (type == UFBXWI_BUFFER_TYPE_VEC3 && source_type == UFBXWI_BUFFER_TYPE_FLOAT3) {
			transform.params.cast.fn = &ufbxwi_cast_float_real;
			transform.params.cast.components = 3;
		}

		transform.src[0] = src;
		transform.dst[0] = dst;
		ufbxwi_buffer_add_transform(pool, &transform);
		return dst;
	}

	ufbxwi_buffer_id id = 0;
	if (pool->free_buffer_ids.count > 0) {
		id = pool->free_buffer_ids.data[--pool->free_buffer_ids.count];
	} else {
		id = (ufbxwi_buffer_id)pool->buffers.count;
		ufbxwi_check_return_err(pool->error, ufbxwi_list_push_zero(pool->ator, &pool->buffers, ufbxwi_buffer), 0);
	}

	ufbxwi_buffer *buffer = &pool->buffers.data[id];

	buffer->count = (uint32_t)source->count;
	buffer->type = source_type;
	buffer->flags = source->data.flags;
	buffer->refcount = 0;

	switch (source->data.source) {
	case UFBXWI_BUFFER_SOURCE_NONE:
		break;
	case UFBXWI_BUFFER_SOURCE_ARRAY:
		buffer->data = (void*)source->data.data;
		buffer->buffer_count = buffer->count;
		if ((buffer->flags & UFBXWI_BUFFER_FLAG_DEFERRED) == 0) {
			buffer->flags |= UFBXWI_BUFFER_FLAG_TRANSIENT;
		}

		break;
	case UFBXWI_BUFFER_SOURCE_STREAM: {
		ufbxwi_buffer_transform_desc desc = { &ufbxwi_transform_stream };
		desc.dst[0] = id;
		desc.params.user.data = (void*)source->data.data;
		desc.params.user.imp = source->data.imp;
		ufbxwi_buffer_add_transform(pool, &desc);
	} break;
	case UFBXWI_BUFFER_SOURCE_CONST: {
		ufbxwi_buffer_transform_desc desc = { &ufbxwi_transform_const };
		desc.dst[0] = id;
		desc.params.user.data = (void*)source->data.data;
		desc.params.user.imp = source->data.imp;
		ufbxwi_buffer_add_transform(pool, &desc);
	} break;
	}

	return id;
}

static ufbxwi_buffer_id ufbxwi_buffer_add_empty(ufbxwi_buffer_pool *pool, ufbxwi_buffer_type type, size_t count)
{
	ufbxw_buffer b = { count, { NULL, type, UFBXWI_BUFFER_SOURCE_NONE }};
	return ufbxwi_buffer_add(pool, type, &b);
}

static void ufbxwi_buffer_set(ufbxwi_buffer_pool *pool, ufbxwi_buffer_ref *ref, ufbxwi_buffer_type type, const ufbxw_buffer *source)
{
	ufbxwi_buffer_free(pool, ref->id);
	ref->id = ufbxwi_buffer_add(pool, type, source);
	ref->count = source->count;
	ref->data = NULL;
}

static void ufbxwi_buffer_init(ufbxwi_buffer_pool *pool, ufbxwi_buffer_ref *ref, ufbxwi_buffer_type type, size_t count)
{
	ufbxwi_buffer_free(pool, ref->id);
	ref->id = ufbxwi_buffer_add_empty(pool, type, count);
	ref->count = count;
	ref->data = NULL;
}

static bool ufbxwi_has_transient_buffers(ufbxwi_buffer_pool *pool, ufbxwi_buffer_id id)
{
	// TODO: Soft recursion
	ufbxwi_buffer *buf = &pool->buffers.data[id];
	if (buf->flags & UFBXWI_BUFFER_FLAG_TRANSIENT) {
		return true;
	}

	if (buf->src_transform != 0) {
		ufbxwi_buffer_transform_data *transform = &pool->transforms.data[buf->src_transform];
		for (size_t i = 0; i < UFBXWI_BUFFER_TRANSFORM_MAX_SRC; i++) {
			if (!transform->src[i]) break;
			if (ufbxwi_has_transient_buffers(pool, transform->src[i])) return true;
		}
	}

	return false;
}

static bool ufbxwi_update_transform(ufbxwi_buffer_pool *pool, ufbxwi_buffer_transform_id id)
{
	ufbxwi_buffer_transform_data *transform = &pool->transforms.data[id];

	ufbxwi_buffer_transform info;
	info.params = transform->params;

	for (size_t i = 0; i < UFBXWI_BUFFER_TRANSFORM_MAX_SRC; i++) {
		ufbxwi_buffer_id buf_id = transform->src[i];
		if (buf_id == 0) break;

		ufbxwi_buffer *buf = &pool->buffers.data[buf_id];
		size_t type_size = ufbxwi_buffer_type_size[buf->type];
		info.src[i].id = buf_id;
		info.src[i].begin = buf->data;
		info.src[i].end = (char*)buf->data + buf->buffer_count * type_size;
		info.src[i].offset = buf->buffer_offset;
	}

	for (size_t i = 0; i < UFBXWI_BUFFER_TRANSFORM_MAX_DST; i++) {
		ufbxwi_buffer_id buf_id = transform->dst[i];
		if (buf_id == 0) break;

		ufbxwi_buffer *buf = &pool->buffers.data[buf_id];
		size_t type_size = ufbxwi_buffer_type_size[buf->type];
		info.dst[i].id = buf_id;
		info.dst[i].begin = (char*)buf->data + buf->buffer_count * type_size;
		info.dst[i].end = (char*)buf->data + buf->buffer_capacity * type_size;
		info.dst[i].offset = buf->buffer_offset + buf->buffer_count;
	}

	if (!transform->fn(pool, &info)) {
		return false;
	}

	for (size_t i = 0; i < UFBXWI_BUFFER_TRANSFORM_MAX_SRC; i++) {
		ufbxwi_buffer_id buf_id = transform->src[i];
		if (buf_id == 0) break;

		ufbxwi_buffer *buf = &pool->buffers.data[buf_id];
		size_t type_size = ufbxwi_buffer_type_size[buf->type];
		char *begin = (char*)buf->data;
		// TODO: We need transform local offsets for materialized buffers
		buf->buffer_offset += (uint32_t)(((char*)info.src[i].begin - begin) / type_size);
		buf->data = info.src[i].begin;
	}

	for (size_t i = 0; i < UFBXWI_BUFFER_TRANSFORM_MAX_DST; i++) {
		ufbxwi_buffer_id buf_id = transform->dst[i];
		if (buf_id == 0) break;

		ufbxwi_buffer *buf = &pool->buffers.data[buf_id];
		size_t type_size = ufbxwi_buffer_type_size[buf->type];
		char *begin = (char*)buf->data + buf->buffer_count * type_size;
		buf->buffer_count += (uint32_t)(((char*)info.dst[i].begin - begin) / type_size);
	}

	return true;
}

static bool ufbxwi_materialize_buffer(ufbxwi_buffer_pool *pool, ufbxwi_buffer_id id)
{
	ufbxwi_buffer *buf = &pool->buffers.data[id];

	size_t type_size = ufbxwi_buffer_type_size[buf->type];
	if (buf->flags & UFBXWI_BUFFER_FLAG_ALLOCATED) {
		ufbxwi_free(pool->ator, buf->data);
		buf->data = NULL;
	}

	buf->data = ufbxwi_alloc_size(pool->ator, type_size, buf->count, NULL);
	buf->flags |= UFBXWI_BUFFER_FLAG_ALLOCATED;
	buf->buffer_capacity = buf->count;

	if (buf->src_transform != 0) {
		while (buf->buffer_count < buf->buffer_capacity) {
			// TODO: Make sure to flush the transform fully here
			if (!ufbxwi_update_transform(pool, buf->src_transform)) {
				return false;
			}
		}
		buf->src_transform = 0;
	}

	return true;
}

static void ufbxwi_buffer_retain(ufbxwi_buffer_pool *pool, ufbxwi_buffer_id id)
{
	ufbxwi_buffer *buf = &pool->buffers.data[id];
	buf->refcount++;
	if (buf->flags & UFBXWI_BUFFER_FLAG_RETAINED) return;
	buf->flags |= UFBXWI_BUFFER_FLAG_RETAINED;

	if (ufbxwi_has_transient_buffers(pool, id)) {
		ufbxwi_materialize_buffer(pool, id);
	}
}

static void ufbxwi_buffer_set_and_retain(ufbxwi_buffer_pool *pool, ufbxwi_buffer_ref *ref, ufbxwi_buffer_type type, const ufbxw_buffer *source)
{
	ufbxwi_buffer_set(pool, ref, type, source);
	ufbxwi_buffer_retain(pool, ref->id);
}

#endif

// -- Prop types

static const ufbxw_vec3 ufbxwi_one_vec3 = { 1.0f, 1.0f, 1.0f };

// -- Scene

#define UFBXWI_ELEMENT_TYPE_NONE ((ufbxw_element_type)0)

static ufbxwi_forceinline ufbxw_id ufbxwi_make_id(ufbxw_element_type type, uint32_t generation, size_t index)
{
	ufbxw_assert((uint64_t)index < ((uint64_t)1u << 32u));
	return (ufbxw_id)(((uint64_t)type << 48) | ((uint64_t)generation << 32) | (index));
}

#define ufbxwi_id_index(id) (uint32_t)(id)
#define ufbxwi_id_type(id) (ufbxw_element_type)(((id) >> 48))
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

typedef bool ufbxwi_element_init_data_fn(ufbxw_scene *scene, void *data);
typedef struct ufbxwi_element_type_desc ufbxwi_element_type_desc;

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

	bool initialized;
	const ufbxwi_element_type_desc *desc;
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

	uint64_t conn_bits;
	uint32_t type_id;
	uint32_t flags;
} ufbxwi_element;

typedef struct {
	ufbxw_id id;
	ufbxwi_token src_prop;
	ufbxwi_token dst_prop;
} ufbxwi_conn;

UFBXWI_LIST_TYPE(ufbxwi_conn_list, ufbxwi_conn);

UFBXWI_LIST_TYPE(ufbxwi_element_list, ufbxwi_element);
UFBXWI_LIST_TYPE(ufbxw_node_list, ufbxw_node);
UFBXWI_LIST_TYPE(ufbxw_material_list, ufbxw_material);

UFBXWI_LIST_TYPE(ufbxw_keyframe_real_list, ufbxw_keyframe_real);

typedef struct {
	ufbxw_id id;
	ufbxwi_conn_list anim_props;
	ufbxwi_conn_list user_conns_src;
	ufbxwi_conn_list user_conns_dst;
} ufbxwi_element_data;

typedef struct {
	ufbxwi_element_data element;

	ufbxw_node parent;
	ufbxw_node_list children;
	ufbxw_id attribute;
	ufbxw_material_list materials;

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

	int32_t default_attribute_index;
} ufbxwi_node;

typedef struct {
	ufbxwi_element_data element;
	ufbxw_node_list instances;
} ufbxwi_node_attribute;

typedef struct {
	union {
		ufbxwi_element_data element;
		ufbxwi_node_attribute attrib;
	};

	ufbxw_vec3_buffer vertices;

	ufbxw_int_buffer vertex_indices;
	ufbxw_int_buffer face_offsets;
	ufbxw_int_buffer polygon_vertex_order;

} ufbxwi_mesh;

typedef struct {
	union {
		ufbxwi_element_data element;
		ufbxwi_node_attribute attrib;
	};

	// TODO: Light data
} ufbxwi_light;

typedef struct {
	ufbxwi_element_data element;
	ufbxwi_conn_list textures;
	ufbxwi_id_list conn_nodes;
	ufbxw_string shading_model;
	bool multi_layer;
} ufbxwi_material;

typedef struct {
	ufbxwi_element_data element;
	ufbxwi_conn_list conn_materials;
	ufbxw_string type;
	ufbxw_string filename;
	ufbxw_string relative_filename;
} ufbxwi_texture;

typedef struct {
	ufbxwi_element_data element;
	ufbxwi_conn prop;

	// TODO: Less wasteful implementation
	ufbxw_keyframe_real_list keys;

} ufbxwi_anim_curve;

typedef struct {
	ufbxwi_element_data element;
	ufbxwi_conn_list curves;
	ufbxw_anim_layer layer;
	ufbxwi_conn prop;
	ufbxw_real defaults[4];
} ufbxwi_anim_prop;

typedef struct {
	ufbxwi_element_data element;
	ufbxwi_conn_list anim_props;
	ufbxw_anim_stack stack;
	ufbxw_real weight;
} ufbxwi_anim_layer;

typedef struct {
	ufbxwi_element_data element;
	ufbxwi_id_list layers;
} ufbxwi_anim_stack;

typedef struct {
	ufbxwi_element_data element;
	ufbxwi_token type;
} ufbxwi_template;

typedef struct {
	ufbxwi_element_data element;
	int a;
} ufbxwi_scene_info;

typedef struct {
	ufbxwi_element_data element;
	int b;
} ufbxwi_global_settings;

typedef struct {
	ufbxwi_element_data element;
	ufbxw_id root_node;
} ufbxwi_document;

struct ufbxw_scene {
	ufbxwi_allocator ator;
	ufbxwi_error error;
	ufbxw_scene_opts opts;
	ufbxwi_string_pool string_pool;
	ufbxwi_buffer_pool buffers;

	ufbxwi_element_list elements;
	ufbxwi_uint32_list free_element_ids;
	size_t num_elements;

	ufbxwi_object_type_list object_types;
	ufbxwi_element_type_list element_types;
	ufbxwi_prop_type_list prop_types;

	ufbxw_anim_stack default_anim_stack;
	ufbxw_anim_layer default_anim_layer;

	// TODO: Something better, hash set
	ufbxwi_id_list tmp_ids;
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
	{ "Float", "", UFBXW_PROP_DATA_REAL },
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
	{ UFBXWI_Texture },
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
	int32_t int_1;
	int32_t int_neg1;
	ufbxw_real double_1;
	ufbxw_real double_10;
	ufbxw_real double_20;
	ufbxw_real double_45;
	ufbxw_real double_50;
	ufbxw_real double_100;
	ufbxw_vec3 vec3_1;
	ufbxw_vec3 vec3_color;
	ufbxw_string string_empty;
	ufbxw_string string_default;
	ufbxw_string string_lambert;
} ufbxwi_prop_defaults;

static const ufbxwi_prop_defaults ufbxwi_prop_default_data = {
	{ 0 }, // zero
	true,
	1,
	-1,
	(ufbxw_real)1.0,
	(ufbxw_real)10.0,
	(ufbxw_real)20.0,
	(ufbxw_real)45.0,
	(ufbxw_real)50.0,
	(ufbxw_real)100.0,
	{ 1.0f, 1.0f, 1.0f },
	{ (ufbxw_real)0.8, (ufbxw_real)0.8, (ufbxw_real)0.8 },
	{ ufbxwi_empty_char, 0 },
	{ "default", 7 },
	{ "lambert", 7 },
};

enum {
	UFBXWI_PROP_FLAG_EXCLUDE_FROM_TEMPLATE = 0x1000,
};

typedef struct {
	uint16_t name;
	uint8_t type;
	int16_t value_offset;
	int16_t default_offset;
	uint16_t flags;
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
	{ UFBXWI_DefaultAttributeIndex, UFBXW_PROP_TYPE_INT, ufbxwi_field(ufbxwi_node, default_attribute_index), ufbxwi_default(int_neg1) },
	{ UFBXWI_Freeze, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_LODBox, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_Lcl_Translation, UFBXW_PROP_TYPE_LCL_TRANSLATION, ufbxwi_field(ufbxwi_node, lcl_translation), 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_Lcl_Rotation, UFBXW_PROP_TYPE_LCL_ROTATION, ufbxwi_field(ufbxwi_node, lcl_rotation), 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_Lcl_Scaling, UFBXW_PROP_TYPE_LCL_SCALING, ufbxwi_field(ufbxwi_node, lcl_scaling), ufbxwi_default(vec3_1), UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_Visibility, UFBXW_PROP_TYPE_VISIBILITY, ufbxwi_field(ufbxwi_node, visibility), ufbxwi_default(bool_true), UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_Visibility_Inheritance, UFBXW_PROP_TYPE_VISIBILITY_INHERITANCE, ufbxwi_field(ufbxwi_node, visibility_inheritance), ufbxwi_default(bool_true) },
};

static const ufbxwi_prop_desc ufbxwi_mesh_props[] = {
	{ UFBXWI_Color, UFBXW_PROP_TYPE_COLOR_RGB, ufbxwi_default(vec3_color) },
	{ UFBXWI_BBoxMin, UFBXW_PROP_TYPE_VECTOR3D },
	{ UFBXWI_BBoxMax, UFBXW_PROP_TYPE_VECTOR3D },
	{ UFBXWI_Primary_Visibility, UFBXW_PROP_TYPE_BOOL, ufbxwi_default(bool_true) },
	{ UFBXWI_Casts_Shadows, UFBXW_PROP_TYPE_BOOL, ufbxwi_default(bool_true) },
	{ UFBXWI_Receive_Shadows, UFBXW_PROP_TYPE_BOOL, ufbxwi_default(bool_true) },
};

static const ufbxwi_prop_desc ufbxwi_light_props[] = {
	{ UFBXWI_Color, UFBXW_PROP_TYPE_COLOR, ufbxwi_default(vec3_1), 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_LightType, UFBXW_PROP_TYPE_ENUM, },
	{ UFBXWI_CastLightOnObject, UFBXW_PROP_TYPE_BOOL, ufbxwi_default(bool_true) },
	{ UFBXWI_DrawVolumetricLight, UFBXW_PROP_TYPE_BOOL, ufbxwi_default(bool_true) },
	{ UFBXWI_DrawGroundProjection, UFBXW_PROP_TYPE_BOOL, ufbxwi_default(bool_true) },
	{ UFBXWI_DrawFrontFacingVolumetricLight, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_Intensity, UFBXW_PROP_TYPE_NUMBER, ufbxwi_default(double_100), 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_InnerAngle, UFBXW_PROP_TYPE_NUMBER, 0, 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_OuterAngle, UFBXW_PROP_TYPE_NUMBER, ufbxwi_default(double_45), 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_Fog, UFBXW_PROP_TYPE_NUMBER, ufbxwi_default(double_50), 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_DecayType, UFBXW_PROP_TYPE_ENUM, },
	{ UFBXWI_DecayStart, UFBXW_PROP_TYPE_NUMBER, 0, 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_FileName, UFBXW_PROP_TYPE_STRING, ufbxwi_default(string_empty) },
	{ UFBXWI_EnableNearAttenuation, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_NearAttenuationStart, UFBXW_PROP_TYPE_NUMBER, 0, 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_NearAttenuationEnd, UFBXW_PROP_TYPE_NUMBER, 0, 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_EnableFarAttenuation, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_FarAttenuationStart, UFBXW_PROP_TYPE_NUMBER, 0, 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_FarAttenuationEnd, UFBXW_PROP_TYPE_NUMBER, 0, 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_CastShadows, UFBXW_PROP_TYPE_BOOL, ufbxwi_default(bool_true) },
	{ UFBXWI_ShadowColor, UFBXW_PROP_TYPE_COLOR, 0, 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_AreaLightShape, UFBXW_PROP_TYPE_ENUM, },
	{ UFBXWI_LeftBarnDoor, UFBXW_PROP_TYPE_FLOAT, ufbxwi_default(double_20), 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_RightBarnDoor, UFBXW_PROP_TYPE_FLOAT, ufbxwi_default(double_20), 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_TopBarnDoor, UFBXW_PROP_TYPE_FLOAT, ufbxwi_default(double_20), 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_BottomBarnDoor, UFBXW_PROP_TYPE_FLOAT, ufbxwi_default(double_20), 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_EnableBarnDoor, UFBXW_PROP_TYPE_USER_BOOL, 0, 0, UFBXW_PROP_FLAG_ANIMATABLE },
};

static const ufbxwi_prop_desc ufbxwi_material_lambert_props[] = {
	{ UFBXWI_ShadingModel, UFBXW_PROP_TYPE_STRING, ufbxwi_default(string_lambert) },
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

static const ufbxwi_prop_desc ufbxwi_file_texture_props[] = {
	{ UFBXWI_TextureTypeUse, UFBXW_PROP_TYPE_ENUM, },
	{ UFBXWI_Texture_alpha, UFBXW_PROP_TYPE_NUMBER, ufbxwi_default(double_1), 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_CurrentMappingType, UFBXW_PROP_TYPE_ENUM, },
	{ UFBXWI_WrapModeU, UFBXW_PROP_TYPE_ENUM, },
	{ UFBXWI_WrapModeV, UFBXW_PROP_TYPE_ENUM, },
	{ UFBXWI_UVSwap, UFBXW_PROP_TYPE_BOOL, },
	{ UFBXWI_PremultiplyAlpha, UFBXW_PROP_TYPE_BOOL, ufbxwi_default(bool_true) },
	{ UFBXWI_Translation, UFBXW_PROP_TYPE_VECTOR, 0, 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_Rotation, UFBXW_PROP_TYPE_VECTOR, 0, 0, UFBXW_PROP_FLAG_ANIMATABLE  },
	{ UFBXWI_Scaling, UFBXW_PROP_TYPE_VECTOR, ufbxwi_default(vec3_1), 0, UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_TextureRotationPivot, UFBXW_PROP_TYPE_VECTOR3D, },
	{ UFBXWI_TextureScalingPivot, UFBXW_PROP_TYPE_VECTOR3D, },
	{ UFBXWI_CurrentTextureBlendMode, UFBXW_PROP_TYPE_ENUM, ufbxwi_default(int_1) },
	{ UFBXWI_UVSet, UFBXW_PROP_TYPE_STRING, ufbxwi_default(string_default) },
	{ UFBXWI_UseMaterial, UFBXW_PROP_TYPE_BOOL },
	{ UFBXWI_UseMipMap, UFBXW_PROP_TYPE_BOOL },
};

static const ufbxwi_prop_desc ufbxwi_anim_prop_props[] = {
	{ UFBXWI_d, UFBXW_PROP_TYPE_COMPOUND },
};

static const ufbxwi_prop_desc ufbxwi_anim_layer_props[] = {
	{ UFBXWI_Weight, UFBXW_PROP_TYPE_NUMBER, ufbxwi_field(ufbxwi_anim_layer, weight), ufbxwi_default(double_100), UFBXW_PROP_FLAG_ANIMATABLE },
	{ UFBXWI_Mute, UFBXW_PROP_TYPE_BOOL },
	{ UFBXWI_Solo, UFBXW_PROP_TYPE_BOOL },
	{ UFBXWI_Lock, UFBXW_PROP_TYPE_BOOL },
	{ UFBXWI_Color, UFBXW_PROP_TYPE_COLOR_RGB, ufbxwi_default(vec3_color), },
	{ UFBXWI_BlendMode, UFBXW_PROP_TYPE_ENUM, },
	{ UFBXWI_RotationAccumulationMode, UFBXW_PROP_TYPE_ENUM, },
	{ UFBXWI_ScaleAccumulationMode, UFBXW_PROP_TYPE_ENUM, },
	{ UFBXWI_BlendModeBypass, UFBXW_PROP_TYPE_ULONGLONG, },
};

static const ufbxwi_prop_desc ufbxwi_anim_stack_props[] = {
	{ UFBXWI_Description, UFBXW_PROP_TYPE_STRING, ufbxwi_default(string_empty) },
	{ UFBXWI_LocalStart, UFBXW_PROP_TYPE_TIME, },
	{ UFBXWI_LocalStop, UFBXW_PROP_TYPE_TIME, },
	{ UFBXWI_ReferenceStart, UFBXW_PROP_TYPE_TIME, },
	{ UFBXWI_ReferenceStop, UFBXW_PROP_TYPE_TIME, },
};

static const ufbxwi_prop_desc ufbxwi_document_props[] = {
	{ UFBXWI_SourceObject, UFBXW_PROP_TYPE_OBJECT },
	{ UFBXWI_ActiveAnimStackName, UFBXW_PROP_TYPE_STRING },
};

#define UFBXWI_CONN_BIT_ANYTHING UINT64_C(0x0)
#define UFBXWI_CONN_BIT_ELEMENT UINT64_C(0x1)
#define UFBXWI_CONN_BIT_PROPERTY UINT64_C(0x2)

#define UFBXWI_CONN_BIT_TYPE_ANY UINT64_C(0x0)
#define UFBXWI_CONN_BIT_TYPE_NODE UINT64_C(0x10)
#define UFBXWI_CONN_BIT_TYPE_NODE_ATTRIBUTE UINT64_C(0x20)
#define UFBXWI_CONN_BIT_TYPE_MATERIAL UINT64_C(0x40)
#define UFBXWI_CONN_BIT_TYPE_TEXTURE UINT64_C(0x80)
#define UFBXWI_CONN_BIT_TYPE_ANIM_CURVE UINT64_C(0x100)
#define UFBXWI_CONN_BIT_TYPE_ANIM_PROP UINT64_C(0x200)
#define UFBXWI_CONN_BIT_TYPE_ANIM_LAYER UINT64_C(0x400)
#define UFBXWI_CONN_BIT_TYPE_ANIM_STACK UINT64_C(0x800)

#define UFBXWI_CONN_BIT_ELEMENT_NODE (UFBXWI_CONN_BIT_ELEMENT | UFBXWI_CONN_BIT_TYPE_NODE)
#define UFBXWI_CONN_BIT_ELEMENT_NODE_ATTRIBUTE (UFBXWI_CONN_BIT_ELEMENT | UFBXWI_CONN_BIT_TYPE_NODE_ATTRIBUTE)
#define UFBXWI_CONN_BIT_ELEMENT_MATERIAL (UFBXWI_CONN_BIT_ELEMENT | UFBXWI_CONN_BIT_TYPE_MATERIAL)
#define UFBXWI_CONN_BIT_ELEMENT_TEXTURE (UFBXWI_CONN_BIT_ELEMENT | UFBXWI_CONN_BIT_TYPE_TEXTURE)
#define UFBXWI_CONN_BIT_ELEMENT_ANIM_CURVE (UFBXWI_CONN_BIT_ELEMENT | UFBXWI_CONN_BIT_TYPE_ANIM_CURVE)
#define UFBXWI_CONN_BIT_ELEMENT_ANIM_PROP (UFBXWI_CONN_BIT_ELEMENT | UFBXWI_CONN_BIT_TYPE_ANIM_PROP)
#define UFBXWI_CONN_BIT_ELEMENT_ANIM_LAYER (UFBXWI_CONN_BIT_ELEMENT | UFBXWI_CONN_BIT_TYPE_ANIM_LAYER)
#define UFBXWI_CONN_BIT_ELEMENT_ANIM_STACK (UFBXWI_CONN_BIT_ELEMENT | UFBXWI_CONN_BIT_TYPE_ANIM_STACK)

#define UFBXWI_CONN_BIT_PROPERTY_ANY (UFBXWI_CONN_BIT_PROPERTY | UFBXWI_CONN_BIT_TYPE_ANY)
#define UFBXWI_CONN_BIT_PROPERTY_MATERIAL (UFBXWI_CONN_BIT_PROPERTY | UFBXWI_CONN_BIT_TYPE_MATERIAL)
#define UFBXWI_CONN_BIT_PROPERTY_ANIM_PROP (UFBXWI_CONN_BIT_PROPERTY | UFBXWI_CONN_BIT_TYPE_ANIM_PROP)

typedef struct {
	uint16_t data_size;
	uint64_t conn_bits;
} ufbxwi_element_type_info;

static const ufbxwi_element_type_info ufbxwi_element_type_infos[] = {
	{ 0 }, // 0
	{ 0 }, // CUSTOM
	{ sizeof(ufbxwi_node), UFBXWI_CONN_BIT_TYPE_NODE },
	{ sizeof(ufbxwi_node_attribute), UFBXWI_CONN_BIT_TYPE_NODE_ATTRIBUTE },
	{ sizeof(ufbxwi_mesh), UFBXWI_CONN_BIT_TYPE_NODE_ATTRIBUTE },
	{ sizeof(ufbxwi_light), UFBXWI_CONN_BIT_TYPE_NODE_ATTRIBUTE },
	{ sizeof(ufbxwi_material), UFBXWI_CONN_BIT_TYPE_MATERIAL },
	{ sizeof(ufbxwi_texture), UFBXWI_CONN_BIT_TYPE_TEXTURE },
	{ sizeof(ufbxwi_anim_curve), UFBXWI_CONN_BIT_TYPE_ANIM_CURVE },
	{ sizeof(ufbxwi_anim_prop), UFBXWI_CONN_BIT_TYPE_ANIM_PROP },
	{ sizeof(ufbxwi_anim_layer), UFBXWI_CONN_BIT_TYPE_ANIM_LAYER },
	{ sizeof(ufbxwi_anim_stack), UFBXWI_CONN_BIT_TYPE_ANIM_STACK },
	{ sizeof(ufbxwi_template) },
	{ sizeof(ufbxwi_scene_info) },
	{ sizeof(ufbxwi_global_settings) },
	{ sizeof(ufbxwi_document) },
};

ufbxw_static_assert(ufbxwi_element_type_infos_count, ufbxwi_arraycount(ufbxwi_element_type_infos) == UFBXW_ELEMENT_TYPE_COUNT);

typedef enum {
	UFBXWI_CONN_TYPE_ID = 0x1,
	UFBXWI_CONN_TYPE_CONN = 0x2,
	UFBXWI_CONN_TYPE_ID_LIST = 0x3,
	UFBXWI_CONN_TYPE_CONN_LIST = 0x4,
	UFBXWI_CONN_TYPE_DATA_MASK = 0xf,

	UFBXWI_CONN_TYPE_UNORDERED = 0x10,
	UFBXWI_CONN_TYPE_SPARSE = 0x20,
} ufbxwi_conn_type;

#define ufbxwi_conn_make(conn_type, type, field) (uint32_t)((conn_type) << 24 | (offsetof(type, field)))
#define ufbxwi_conn_offset(conn) (uint32_t)((conn) & 0xffffff)
#define ufbxwi_conn_type(conn) (ufbxwi_conn_type)((conn) >> 24)

#define ufbxwi_conn_id(type, field) ufbxwi_conn_make(UFBXWI_CONN_TYPE_ID, type, field)
#define ufbxwi_conn_conn(type, field) ufbxwi_conn_make(UFBXWI_CONN_TYPE_CONN, type, field)
#define ufbxwi_conn_id_list(type, field) ufbxwi_conn_make(UFBXWI_CONN_TYPE_ID_LIST, type, field)
#define ufbxwi_conn_id_list_ex(type, field, flags) ufbxwi_conn_make(UFBXWI_CONN_TYPE_ID_LIST | (flags), type, field)
#define ufbxwi_conn_conn_list(type, field) ufbxwi_conn_make(UFBXWI_CONN_TYPE_CONN_LIST, type, field)
#define ufbxwi_conn_conn_list_ex(type, field, flags) ufbxwi_conn_make(UFBXWI_CONN_TYPE_CONN_LIST | (flags), type, field)

typedef struct {
	const char *debug_name;
	uint64_t src_mask;
	uint64_t dst_mask;
	uint32_t src_conn;
	uint32_t dst_conn;
} ufbxwi_connection_info;

static const ufbxwi_connection_info ufbxwi_connection_infos[] = {
	{ 0 }, // Invalid
	{ "Node Parent", UFBXWI_CONN_BIT_ELEMENT_NODE, UFBXWI_CONN_BIT_ELEMENT_NODE, ufbxwi_conn_id(ufbxwi_node, parent), ufbxwi_conn_id_list(ufbxwi_node, children) },
	{ "Node Attribute", UFBXWI_CONN_BIT_ELEMENT_NODE_ATTRIBUTE, UFBXWI_CONN_BIT_ELEMENT_NODE, ufbxwi_conn_id_list(ufbxwi_node_attribute, instances), ufbxwi_conn_id(ufbxwi_node, attribute) },
	{ "Node Material", UFBXWI_CONN_BIT_ELEMENT_MATERIAL, UFBXWI_CONN_BIT_ELEMENT_NODE, ufbxwi_conn_id_list_ex(ufbxwi_material, conn_nodes, UFBXWI_CONN_TYPE_UNORDERED), ufbxwi_conn_id_list_ex(ufbxwi_node, materials, UFBXWI_CONN_TYPE_SPARSE) },
	{ "Material Texture", UFBXWI_CONN_BIT_ELEMENT_TEXTURE, UFBXWI_CONN_BIT_PROPERTY_MATERIAL, ufbxwi_conn_conn_list_ex(ufbxwi_texture, conn_materials, UFBXWI_CONN_TYPE_UNORDERED), ufbxwi_conn_conn_list(ufbxwi_material, textures) },
	{ "Animated Property", UFBXWI_CONN_BIT_ELEMENT_ANIM_PROP, UFBXWI_CONN_BIT_PROPERTY_ANY, ufbxwi_conn_conn(ufbxwi_anim_prop, prop), ufbxwi_conn_conn_list_ex(ufbxwi_element_data, anim_props, UFBXWI_CONN_TYPE_UNORDERED) },
	{ "Animation Curve Property", UFBXWI_CONN_BIT_ELEMENT_ANIM_CURVE, UFBXWI_CONN_BIT_PROPERTY_ANIM_PROP, ufbxwi_conn_conn(ufbxwi_anim_curve, prop), ufbxwi_conn_conn_list(ufbxwi_anim_prop, curves) },
	{ "Animation Property Layer", UFBXWI_CONN_BIT_ELEMENT_ANIM_PROP, UFBXWI_CONN_BIT_ELEMENT_ANIM_LAYER, ufbxwi_conn_id(ufbxwi_anim_prop, layer), ufbxwi_conn_id_list_ex(ufbxwi_anim_layer, anim_props, UFBXWI_CONN_TYPE_UNORDERED) },
	{ "Animation Layer Stack", UFBXWI_CONN_BIT_ELEMENT_ANIM_LAYER, UFBXWI_CONN_BIT_ELEMENT_ANIM_STACK, ufbxwi_conn_id(ufbxwi_anim_layer, stack), ufbxwi_conn_id_list(ufbxwi_anim_stack, layers) },
	{ "User", UFBXWI_CONN_BIT_ANYTHING, UFBXWI_CONN_BIT_ANYTHING, ufbxwi_conn_conn_list(ufbxwi_element_data, user_conns_src), ufbxwi_conn_conn_list(ufbxwi_element_data, user_conns_dst) },
};

ufbxw_static_assert(ufbxwi_connection_infos_counst, ufbxwi_arraycount(ufbxwi_connection_infos) == UFBXW_CONNECTION_TYPE_COUNT);

enum {
	UFBXWI_CONNECT_FLAG_DISCONNECT_SRC = 0x1,
	UFBXWI_CONNECT_FLAG_DISCONNECT_DST = 0x2,
};

static bool ufbxwi_conn_add(ufbxw_scene *scene, ufbxwi_conn_type type, void *data, ufbxw_id id, ufbxwi_token src_prop, ufbxwi_token dst_prop)
{
	switch (type & UFBXWI_CONN_TYPE_DATA_MASK) {
	case UFBXWI_CONN_TYPE_ID: {
		ufbxw_id *d = (ufbxw_id*)data;
		if (*d == ufbxw_null_id) {
			*d = id;
			return true;
		}
	} break;
	case UFBXWI_CONN_TYPE_CONN: {
		ufbxwi_conn *d = (ufbxwi_conn*)data;
		if (d->id == ufbxw_null_id) {
			d->id = id;
			d->src_prop = src_prop;
			d->dst_prop = dst_prop;
			return true;
		}
	} break;
	case UFBXWI_CONN_TYPE_ID_LIST: {
		ufbxwi_id_list *d = (ufbxwi_id_list*)data;
		ufbxwi_check_return(ufbxwi_id_list_add(&scene->ator, d, id), false);
		return true;
	} break;
	case UFBXWI_CONN_TYPE_CONN_LIST: {
		ufbxwi_conn_list *d = (ufbxwi_conn_list*)data;
		ufbxwi_conn *conn = ufbxwi_list_push_uninit(&scene->ator, d, ufbxwi_conn);
		ufbxwi_check_return(conn, false);
		conn->id = id;
		conn->src_prop = src_prop;
		conn->dst_prop = dst_prop;
		return true;
	} break;
	}

	return false;
}

static bool ufbxwi_conn_remove_one(ufbxw_scene *scene, ufbxwi_conn_type type, void *data, ufbxw_id id)
{
	switch (type & UFBXWI_CONN_TYPE_DATA_MASK) {
	case UFBXWI_CONN_TYPE_ID: {
		ufbxw_id *d = (ufbxw_id*)data;
		if (*d == id) {
			*d = ufbxw_null_id;
			return true;
		}
	} break;
	case UFBXWI_CONN_TYPE_CONN: {
		ufbxwi_conn *d = (ufbxwi_conn*)data;
		if (d->id == id) {
			d->id = ufbxw_null_id;
			d->src_prop = d->dst_prop = UFBXWI_TOKEN_NONE;
			return true;
		}
	} break;
	case UFBXWI_CONN_TYPE_ID_LIST: {
		ufbxwi_id_list *d = (ufbxwi_id_list*)data;
		// TODO: Unordered/sparse
		return ufbxwi_id_list_remove_one(d, id);
	} break;
	case UFBXWI_CONN_TYPE_CONN_LIST: {
		ufbxwi_conn_list *d = (ufbxwi_conn_list*)data;
		// TODO: Unordered/sparse
		ufbxwi_for_list(ufbxwi_conn, conn, *d) {
			if (conn->id == id) {
				ufbxwi_conn *last = d->data + d->count - 1;
				if (conn != last) {
					memmove(conn, conn + 1, ufbxwi_to_size(last - conn));
				}
				return true;
			}
		}
	} break;
	}

	return false;
}

static void ufbxwi_conn_remove_all(ufbxw_scene *scene, ufbxwi_conn_type type, void *data, ufbxw_id id)
{
	switch (type & UFBXWI_CONN_TYPE_DATA_MASK) {
	case UFBXWI_CONN_TYPE_ID: {
		ufbxw_id *d = (ufbxw_id*)data;
		if (*d == id) {
			*d = ufbxw_null_id;
		}
	} break;
	case UFBXWI_CONN_TYPE_CONN: {
		ufbxwi_conn *d = (ufbxwi_conn*)data;
		if (d->id == id) {
			d->id = ufbxw_null_id;
			d->src_prop = d->dst_prop = UFBXWI_TOKEN_NONE;
		}
	} break;
	case UFBXWI_CONN_TYPE_ID_LIST: {
		ufbxwi_id_list *d = (ufbxwi_id_list*)data;
		// TODO: Unordered/sparse
		ufbxw_id *ids = d->data;
		size_t dst = 0, count = d->count;
		for (size_t src = 0; src < count; src++) {
			if (ids[src] != id) {
				if (dst != src) ids[dst] = ids[src];
				dst++;
			}
		}
		d->count = dst;
	} break;
	case UFBXWI_CONN_TYPE_CONN_LIST: {
		ufbxwi_conn_list *d = (ufbxwi_conn_list*)data;
		// TODO: Unordered/sparse
		ufbxwi_conn *conns = d->data;
		size_t dst = 0, count = d->count;
		for (size_t src = 0; src < count; src++) {
			if (conns[src].id != id) {
				if (dst != src) conns[dst] = conns[src];
				dst++;
			}
		}
		d->count = dst;
	} break;
	}
}

static int ufbxwi_cmp_id(const void *va, const void *vb)
{
	ufbxw_id a = *(const ufbxw_id*)va, b = *(const ufbxw_id*)vb;
	if (a != b) return a < b ? -1 : +1;
	return 0;
}

static bool ufbxwi_conn_collect_ids(ufbxw_scene *scene, ufbxwi_id_list *ids, ufbxwi_conn_type type, const void *data)
{
	switch (type & UFBXWI_CONN_TYPE_DATA_MASK) {
	case UFBXWI_CONN_TYPE_ID: {
		const ufbxw_id *d = (const ufbxw_id*)data;
		if (*d != ufbxw_null_id) {
			ufbxwi_check_return(ufbxwi_list_push_copy(&scene->ator, ids, ufbxw_id, d), false);
			return true;
		}
	} break;
	case UFBXWI_CONN_TYPE_CONN: {
		const ufbxwi_conn *d = (const ufbxwi_conn*)data;
		if (d->id != ufbxw_null_id) {
			ufbxwi_check_return(ufbxwi_list_push_copy(&scene->ator, ids, ufbxw_id, &d->id), false);
			return true;
		}
	} break;
	case UFBXWI_CONN_TYPE_ID_LIST: {
		ufbxwi_id_list d = *(const ufbxwi_id_list*)data;
		ufbxwi_check_return(ufbxwi_list_push_copy_n(&scene->ator, ids, ufbxw_id, d.count, d.data), false);
	} break;
	case UFBXWI_CONN_TYPE_CONN_LIST: {
		ufbxwi_conn_list d = *(const ufbxwi_conn_list*)data;
		ufbxw_id *dst = ufbxwi_list_push_uninit_n(&scene->ator, ids, ufbxw_id, d.count);
		ufbxwi_check_return(dst, false);
		for (size_t i = 0; i < d.count; i++) {
			dst[i] = d.data[i].id;
		}
	} break;
	}

	// Deduplicate found IDs
	if (ids->count > 1) {
		// TODO: Better sort, especially for small values
		qsort(ids->data, ids->count, sizeof(ufbxw_id), &ufbxwi_cmp_id);
		ufbxw_id prev = ufbxw_null_id;
		ufbxw_id *dst = ids->data;
		ufbxwi_for_list(ufbxw_id, p_id, *ids) {
			ufbxw_id id = *p_id;
			if (id != prev) {
				*dst++ = id;
				prev = id;
			}
		}
		ids->count = dst - ids->data;
	}

	return true;
}

static bool ufbxwi_conn_collect_conns(const ufbxw_scene *scene, ufbxwi_allocator *ator, ufbxwi_conn_list *conns, ufbxwi_conn_type type, const void *data)
{
	switch (type & UFBXWI_CONN_TYPE_DATA_MASK) {
	case UFBXWI_CONN_TYPE_ID: {
		const ufbxw_id *d = (const ufbxw_id*)data;
		if (*d != ufbxw_null_id) {
			ufbxwi_conn *conn = ufbxwi_list_push_uninit(ator, conns, ufbxwi_conn);
			ufbxwi_check_return_err(ator->error, conn, false);
			conn->id = *d;
			conn->src_prop = UFBXWI_TOKEN_NONE;
			conn->dst_prop = UFBXWI_TOKEN_NONE;
			return true;
		}
	} break;
	case UFBXWI_CONN_TYPE_CONN: {
		const ufbxwi_conn *d = (const ufbxwi_conn*)data;
		if (d->id != ufbxw_null_id) {
			ufbxwi_check_return_err(ator->error, ufbxwi_list_push_copy(ator, conns, ufbxwi_conn, d), false);
			return true;
		}
	} break;
	case UFBXWI_CONN_TYPE_ID_LIST: {
		ufbxwi_id_list d = *(const ufbxwi_id_list*)data;
		ufbxwi_conn *dst = ufbxwi_list_push_uninit_n(ator, conns, ufbxwi_conn, d.count);
		ufbxwi_check_return_err(ator->error, dst, false);
		for (size_t i = 0; i < d.count; i++) {
			dst[i].id = d.data[i];
			dst[i].src_prop = UFBXWI_TOKEN_NONE;
			dst[i].dst_prop = UFBXWI_TOKEN_NONE;
		}
	} break;
	case UFBXWI_CONN_TYPE_CONN_LIST: {
		ufbxwi_conn_list d = *(const ufbxwi_conn_list*)data;
		ufbxwi_check_return_err(ator->error, ufbxwi_list_push_copy_n(ator, conns, ufbxwi_conn, d.count, d.data), false);
	} break;
	}

	return true;
}

static void ufbxwi_conn_clear(ufbxw_scene *scene, ufbxwi_conn_type type, const void *data)
{
	switch (type & UFBXWI_CONN_TYPE_DATA_MASK) {
	case UFBXWI_CONN_TYPE_ID: {
		ufbxw_id *d = (ufbxw_id*)data;
		*d = ufbxw_null_id;
	} break;
	case UFBXWI_CONN_TYPE_CONN: {
		ufbxwi_conn *d = (ufbxwi_conn*)data;
		d->id = ufbxw_null_id;
		d->src_prop = d->dst_prop = UFBXWI_TOKEN_NONE;
	} break;
	case UFBXWI_CONN_TYPE_ID_LIST: {
		ufbxwi_id_list *d = (ufbxwi_id_list*)data;
		d->count = 0;
	} break;
	case UFBXWI_CONN_TYPE_CONN_LIST: {
		ufbxwi_conn_list *d = (ufbxwi_conn_list*)data;
		d->count = 0;
	} break;
	}
}

static bool ufbxwi_init_node(ufbxw_scene *scene, void *data)
{
	ufbxwi_node *node = (ufbxwi_node*)data;
	node->lcl_scaling = ufbxwi_one_vec3;
	node->geometric_scaling = ufbxwi_one_vec3;
	node->visibility = true;
	node->visibility_inheritance = true;
	return true;
}

static bool ufbxwi_init_material_lambert(ufbxw_scene *scene, void *data)
{
	ufbxwi_material *material = (ufbxwi_material*)data;
	material->shading_model = ufbxwi_c_str("lambert");
	return true;
}

static bool ufbxwi_init_file_texture(ufbxw_scene *scene, void *data)
{
	ufbxwi_texture *texture = (ufbxwi_texture*)data;
	texture->type = ufbxwi_c_str("TextureVideoClip");
	texture->filename.data = ufbxwi_empty_char;
	texture->relative_filename.data = ufbxwi_empty_char;
	return true;
}

static bool ufbxwi_init_anim_layer(ufbxw_scene *scene, void *data)
{
	ufbxwi_anim_layer *layer = (ufbxwi_anim_layer*)data;
	layer->weight = (ufbxw_real)100.0;
	return true;
}

enum {
	UFBXWI_ELEMENT_TYPE_FLAG_EAGER_PROPS = 0x1,
};

struct ufbxwi_element_type_desc {
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
};

static const ufbxwi_element_type_desc ufbxwi_element_types[] = {
	{
		UFBXW_ELEMENT_TEMPLATE, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE,
		NULL, 0, NULL,
		UFBXWI_ELEMENT_FLAG_ALLOW_NO_OBJECT_ID,
	},
	{
		UFBXW_ELEMENT_NODE, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE, UFBXWI_Model, UFBXWI_Model, UFBXWI_FbxNode,
		ufbxwi_node_props, ufbxwi_arraycount(ufbxwi_node_props), &ufbxwi_init_node,
		0,
	},
	{
		UFBXW_ELEMENT_MESH, UFBXWI_TOKEN_NONE, UFBXWI_Mesh, UFBXWI_Geometry, UFBXWI_Geometry, UFBXWI_FbxMesh,
		ufbxwi_mesh_props, ufbxwi_arraycount(ufbxwi_mesh_props), NULL,
		0,
	},
	{
		UFBXW_ELEMENT_LIGHT, UFBXWI_TOKEN_NONE, UFBXWI_Light, UFBXWI_NodeAttribute, UFBXWI_NodeAttribute, UFBXWI_FbxLight,
		ufbxwi_light_props, ufbxwi_arraycount(ufbxwi_light_props), NULL,
		0,
	},
	{
		UFBXW_ELEMENT_SCENE_INFO, UFBXWI_TOKEN_NONE, UFBXWI_UserData, UFBXWI_SceneInfo, UFBXWI_SceneInfo, UFBXWI_TOKEN_NONE,
		NULL, 0, NULL,
		UFBXWI_ELEMENT_FLAG_ALLOW_NO_OBJECT_ID,
	},
	{
		UFBXW_ELEMENT_GLOBAL_SETTINGS, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE, UFBXWI_GlobalSettings, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE,
		NULL, 0, NULL,
		0,
	},
	{
		UFBXW_ELEMENT_DOCUMENT, UFBXWI_TOKEN_NONE, UFBXWI_Document, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE,
		ufbxwi_document_props, ufbxwi_arraycount(ufbxwi_document_props), NULL,
		UFBXWI_ELEMENT_FLAG_ALLOW_NO_OBJECT_ID,
	},
	{
		UFBXW_ELEMENT_ANIM_CURVE, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_EMPTY, UFBXWI_AnimationCurve, UFBXWI_AnimCurve, UFBXWI_TOKEN_NONE,
		NULL, 0, NULL,
		0,
	},
	{
		UFBXW_ELEMENT_ANIM_PROP, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_EMPTY, UFBXWI_AnimationCurveNode, UFBXWI_AnimCurveNode, UFBXWI_FbxAnimCurveNode,
		ufbxwi_anim_prop_props, ufbxwi_arraycount(ufbxwi_anim_prop_props), NULL,
		UFBXWI_ELEMENT_TYPE_FLAG_EAGER_PROPS,
	},
	{
		UFBXW_ELEMENT_ANIM_LAYER, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_EMPTY, UFBXWI_AnimationLayer, UFBXWI_AnimLayer, UFBXWI_FbxAnimLayer,
		ufbxwi_anim_layer_props, ufbxwi_arraycount(ufbxwi_anim_layer_props), &ufbxwi_init_anim_layer,
		0,
	},
	{
		UFBXW_ELEMENT_ANIM_STACK, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_EMPTY, UFBXWI_AnimationStack, UFBXWI_AnimStack, UFBXWI_FbxAnimStack,
		ufbxwi_anim_stack_props, ufbxwi_arraycount(ufbxwi_anim_stack_props), NULL,
		0,
	},
	{
		UFBXW_ELEMENT_MATERIAL, UFBXWI_FbxSurfaceLambert, UFBXWI_TOKEN_EMPTY, UFBXWI_Material, UFBXWI_Material, UFBXWI_FbxSurfaceLambert,
		ufbxwi_material_lambert_props, ufbxwi_arraycount(ufbxwi_material_lambert_props), &ufbxwi_init_material_lambert,
		UFBXWI_ELEMENT_TYPE_FLAG_EAGER_PROPS,
	},
	{
		UFBXW_ELEMENT_TEXTURE, UFBXWI_FbxFileTexture, UFBXWI_TOKEN_EMPTY, UFBXWI_Texture, UFBXWI_Texture, UFBXWI_FbxFileTexture,
		ufbxwi_file_texture_props, ufbxwi_arraycount(ufbxwi_file_texture_props), &ufbxwi_init_file_texture,
		0,
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
			uint32_t slot = (hash + scan) & mask;
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
		uint32_t slot = (hash + scan) & mask;
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
		uint32_t slot = (hash + scan) & mask;
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
		props->count++;
	}

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

static ufbxwi_forceinline bool ufbxwi_init_element_type_imp(ufbxw_scene *scene, ufbxwi_element_type *et)
{
	const ufbxwi_element_type_desc *desc = et->desc;
	et->initialized = true;

	if (desc->num_props > 0) {
		ufbxwi_check_return(ufbxwi_props_rehash(scene, &et->props, desc->num_props), false);

		ufbxwi_for(const ufbxwi_prop_desc, pd, desc->props, desc->num_props) {
			ufbxwi_prop *prop = ufbxwi_props_add_prop(scene, &et->props, pd->name);
			ufbxwi_check_return(prop, false);
			prop->type = pd->type;
			prop->value_offset = pd->value_offset;
			prop->flags = pd->flags & 0xff; // TODO: Better mapping, compact flags
		}
	}

	if (desc->tmpl_type != UFBXWI_TOKEN_NONE) {
		ufbxw_id template_id = ufbxw_create_element(scene, UFBXW_ELEMENT_TEMPLATE);
		ufbxwi_check_return(template_id, 0);

		ufbxwi_element *tmpl_elem = ufbxwi_get_element(scene, template_id);
		ufbxw_assert(tmpl_elem);

		tmpl_elem->name = scene->string_pool.tokens.data[desc->tmpl_type];

		ufbxwi_template *tmpl_data = (ufbxwi_template*)tmpl_elem->data;
		tmpl_data->type = desc->tmpl_type;

		ufbxwi_check_return(ufbxwi_props_rehash(scene, &tmpl_elem->props, desc->num_props), 0);

		ufbxwi_for(const ufbxwi_prop_desc, pd, desc->props, desc->num_props) {
			if (pd->flags & UFBXWI_PROP_FLAG_EXCLUDE_FROM_TEMPLATE) continue;

			ufbxwi_prop *prop = ufbxwi_props_add_prop(scene, &tmpl_elem->props, pd->name);
			ufbxwi_check_return(prop, false);
			prop->type = pd->type;
			prop->value_offset = pd->value_offset <= 0 ? pd->value_offset : pd->default_offset;
			prop->flags = pd->flags & 0xff; // TODO: Better mapping, compact flags
		}
		et->template_id = template_id;
	}

	return true;
}

static ufbxwi_forceinline bool ufbxwi_init_element_type(ufbxw_scene *scene, ufbxwi_element_type *et)
{
	if (et->initialized) return true;
	return ufbxwi_init_element_type_imp(scene, et);
}

static ufbxw_id ufbxwi_create_element(ufbxw_scene *scene, ufbxw_element_type type, ufbxwi_token class_type)
{
	uint32_t type_id = ufbxwi_find_element_type_id(scene, type, class_type);
	ufbxwi_check_return(type_id != ~0u, ufbxw_null_id);

	const ufbxwi_element_type_info *type_info = &ufbxwi_element_type_infos[type];
	ufbxwi_element_type *element_type = &scene->element_types.data[type_id];
	ufbxwi_check_return(ufbxwi_init_element_type(scene, element_type), 0);

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
	element->conn_bits = type_info->conn_bits;

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
		ufbxwi_check_return(element_type->init_fn(scene, data), 0);
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

static bool ufbxwi_connect_imp(ufbxw_scene *scene, ufbxw_connection_type type, ufbxw_id src_id, ufbxw_id dst_id, ufbxwi_token src_prop, ufbxwi_token dst_prop, uint32_t flags)
{
	ufbxwi_element *src_elem = ufbxwi_get_element(scene, src_id);
	ufbxwi_element *dst_elem = ufbxwi_get_element(scene, dst_id);
	if (!src_elem || !dst_elem) return false;

	ufbxwi_connection_info info = ufbxwi_connection_infos[type];
	uint64_t src_bits = src_elem->conn_bits | (src_prop != UFBXWI_TOKEN_NONE ? UFBXWI_CONN_BIT_PROPERTY : UFBXWI_CONN_BIT_ELEMENT);
	uint64_t dst_bits = dst_elem->conn_bits | (dst_prop != UFBXWI_TOKEN_NONE ? UFBXWI_CONN_BIT_PROPERTY : UFBXWI_CONN_BIT_ELEMENT);
	if ((src_bits & info.src_mask) != info.src_mask) return false;
	if ((dst_bits & info.dst_mask) != info.dst_mask) return false;

	ufbxwi_conn_type src_type = ufbxwi_conn_type(info.src_conn);
	ufbxwi_conn_type dst_type = ufbxwi_conn_type(info.dst_conn);
	void *src_data = (char*)src_elem->data + ufbxwi_conn_offset(info.src_conn);
	void *dst_data = (char*)dst_elem->data + ufbxwi_conn_offset(info.dst_conn);

	bool disconnect = false;
	if (src_type == UFBXWI_CONN_TYPE_ID && *(ufbxw_id*)src_data != ufbxw_null_id) {
		if ((flags & UFBXWI_CONNECT_FLAG_DISCONNECT_SRC) == 0) return false;
		disconnect = true;
	}
	if (dst_type == UFBXWI_CONN_TYPE_ID && *(ufbxw_id*)dst_data != ufbxw_null_id) {
		if ((flags & UFBXWI_CONNECT_FLAG_DISCONNECT_DST) == 0) return false;
		disconnect = true;
	}

	if (disconnect) {
		ufbxwi_conn_remove_one(scene, src_type, src_data, dst_id);
		ufbxwi_conn_remove_one(scene, dst_type, dst_data, src_id);
	}

	ufbxwi_conn_add(scene, src_type, src_data, dst_id, src_prop, dst_prop);
	ufbxwi_conn_add(scene, dst_type, dst_data, src_id, src_prop, dst_prop);
	return true;
}

static ufbxwi_forceinline ufbxwi_connect(ufbxw_scene *scene, ufbxw_connection_type type, ufbxw_id src_id, ufbxw_id dst_id, uint32_t flags)
{
	ufbxwi_connect_imp(scene, type, src_id, dst_id, UFBXWI_TOKEN_NONE, UFBXWI_TOKEN_NONE, flags);
}

static void ufbxwi_disconnect_all_dst(ufbxw_scene *scene, ufbxw_connection_type type, ufbxw_id src_id)
{
	ufbxwi_element *src_elem = ufbxwi_get_element(scene, src_id);
	if (!src_elem) return;

	ufbxwi_connection_info info = ufbxwi_connection_infos[type];
	uint64_t src_bits = src_elem->conn_bits | (UFBXWI_CONN_BIT_PROPERTY | UFBXWI_CONN_BIT_ELEMENT);
	if ((src_bits & info.src_mask) != info.src_mask) return;

	ufbxwi_conn_type src_type = ufbxwi_conn_type(info.src_conn);
	void *src_data = (char*)src_elem->data + ufbxwi_conn_offset(info.src_conn);

	scene->tmp_ids.count = 0;
	ufbxwi_conn_collect_ids(scene, &scene->tmp_ids, src_type, src_data);
	ufbxwi_conn_clear(scene, src_type, src_data);

	ufbxwi_for_list(ufbxw_id, p_id, scene->tmp_ids) {
		ufbxw_id dst_id = *p_id;
		ufbxwi_element *dst_elem = ufbxwi_get_element(scene, dst_id);
		if (!dst_elem) continue;

		ufbxwi_conn_type dst_type = ufbxwi_conn_type(info.dst_conn);
		void *dst_data = (char*)dst_elem->data + ufbxwi_conn_offset(info.dst_conn);

		uint64_t dst_bits = dst_elem->conn_bits | (UFBXWI_CONN_BIT_PROPERTY | UFBXWI_CONN_BIT_ELEMENT);
		ufbxw_assert((dst_bits & info.dst_mask) == info.dst_mask);

		ufbxwi_conn_remove_all(scene, dst_type, dst_data, src_id);
	}
}

static void ufbxwi_disconnect_all_src(ufbxw_scene *scene, ufbxw_connection_type type, ufbxw_id dst_id)
{
	ufbxwi_element *dst_elem = ufbxwi_get_element(scene, dst_id);
	if (!dst_elem) return;

	ufbxwi_connection_info info = ufbxwi_connection_infos[type];
	uint64_t dst_bits = dst_elem->conn_bits | (UFBXWI_CONN_BIT_PROPERTY | UFBXWI_CONN_BIT_ELEMENT);
	if ((dst_bits & info.dst_mask) != info.dst_mask) return;

	ufbxwi_conn_type dst_type = ufbxwi_conn_type(info.dst_conn);
	void *dst_data = (char*)dst_elem->data + ufbxwi_conn_offset(info.dst_conn);

	scene->tmp_ids.count = 0;
	ufbxwi_conn_collect_ids(scene, &scene->tmp_ids, dst_type, dst_data);
	ufbxwi_conn_clear(scene, dst_type, dst_data);

	ufbxwi_for_list(ufbxw_id, p_id, scene->tmp_ids) {
		ufbxw_id src_id = *p_id;
		ufbxwi_element *src_elem = ufbxwi_get_element(scene, src_id);
		if (!src_elem) continue;

		ufbxwi_conn_type src_type = ufbxwi_conn_type(info.src_conn);
		void *src_data = (char*)src_elem->data + ufbxwi_conn_offset(info.src_conn);

		uint64_t src_bits = src_elem->conn_bits | (UFBXWI_CONN_BIT_PROPERTY | UFBXWI_CONN_BIT_ELEMENT);
		ufbxw_assert((src_bits & info.src_mask) != info.src_mask);

		ufbxwi_conn_remove_all(scene, src_type, src_data, dst_id);
	}
}

static void ufbxwi_collect_src_connections(const ufbxw_scene *scene, ufbxwi_allocator *ator, ufbxwi_conn_list *conns, ufbxw_connection_type type, ufbxwi_element *dst_elem)
{
	ufbxwi_connection_info info = ufbxwi_connection_infos[type];
	uint64_t dst_bits = dst_elem->conn_bits | (UFBXWI_CONN_BIT_PROPERTY | UFBXWI_CONN_BIT_ELEMENT);
	if ((dst_bits & info.dst_mask) != info.dst_mask) return;

	ufbxwi_conn_type dst_type = ufbxwi_conn_type(info.dst_conn);
	void *dst_data = (char*)dst_elem->data + ufbxwi_conn_offset(info.dst_conn);

	ufbxwi_conn_collect_conns(scene, ator, conns, dst_type, dst_data);
}

static ufbxw_anim_prop ufbxwi_animate_prop(ufbxw_scene *scene, ufbxw_id id, ufbxwi_token prop, ufbxw_anim_layer layer)
{
	ufbxwi_element *element = ufbxwi_get_element(scene, id);
	if (!element) return ufbxw_null_anim_prop;

	ufbxwi_prop *p = ufbxwi_element_find_prop(scene, element, prop);
	if (!p) return ufbxw_null_anim_prop;

	size_t curve_count = 0;

	ufbxwi_prop_type prop_type = scene->prop_types.data[p->type];
	switch (prop_type.data_type) {
	case UFBXW_PROP_DATA_BOOL:
	case UFBXW_PROP_DATA_INT32:
	case UFBXW_PROP_DATA_INT64:
	case UFBXW_PROP_DATA_REAL:
	case UFBXW_PROP_DATA_REAL_STRING:
	case UFBXW_PROP_DATA_USER_REAL:
	case UFBXW_PROP_DATA_USER_INT:
	case UFBXW_PROP_DATA_USER_ENUM:
		curve_count = 1;
		break;

	case UFBXW_PROP_DATA_VEC2:
		curve_count = 2;
		break;

	case UFBXW_PROP_DATA_VEC3:
		curve_count = 3;
		break;
	}

	if (curve_count == 0) return ufbxw_null_anim_prop;

	const char *anim_name = "";
	switch (p->token) {
	case UFBXWI_Lcl_Translation:
		anim_name = "T";
		break;
	case UFBXWI_Lcl_Rotation:
		anim_name = "R";
		break;
	case UFBXWI_Lcl_Scaling:
		anim_name = "S";
		break;
	default:
		anim_name = scene->string_pool.tokens.data[p->token].data;
		break;
	}

	ufbxw_anim_prop anim = { ufbxw_create_element(scene, UFBXW_ELEMENT_ANIM_PROP) };
	ufbxw_set_name(scene, anim.id, anim_name);

	ufbxwi_connect_imp(scene, UFBXW_CONNECTION_ANIM_PROPERTY, anim.id, id, 0, prop, 0);

	if (layer.id != 0) {
		ufbxwi_connect(scene, UFBXW_CONNECTION_ANIM_PROP_LAYER, anim.id, layer.id, 0);
	}

	static const ufbxwi_token curve_props[] = {
		UFBXWI_d_X, UFBXWI_d_Y, UFBXWI_d_Z,
	};

	ufbxwi_token first_curve_prop = curve_props[0];

	// For single channel propertes, use the property name
	// TODO: Flexible buffer
	char name_buf[256];
	if (curve_count == 1) {
		ufbxw_string prop_name = scene->string_pool.tokens.data[prop];
		int name_len = snprintf(name_buf, sizeof(name_buf), "d|%s", prop_name.data);
		first_curve_prop = ufbxwi_intern_token(&scene->string_pool, name_buf, (size_t)name_len);
	}

	for (size_t i = 0; i < curve_count; i++) {
		ufbxw_anim_curve curve = { ufbxw_create_element(scene, UFBXW_ELEMENT_ANIM_CURVE) };

		ufbxwi_token prop_name = i == 0 ? first_curve_prop : curve_props[i];

		// Manually add the properties so we can point them to the known default values
		ufbxwi_element *anim_elem = ufbxwi_get_element(scene, anim.id);
		ufbxwi_prop *p = ufbxwi_props_add_prop(scene, &anim_elem->props, prop_name);
		p->type = UFBXW_PROP_TYPE_NUMBER;
		p->flags = UFBXW_PROP_FLAG_ANIMATABLE;
		p->value_offset = (int32_t)(offsetof(ufbxwi_anim_prop, defaults) + i * sizeof(ufbxw_real));

		ufbxwi_connect_imp(scene, UFBXW_CONNECTION_ANIM_CURVE_PROP, curve.id, anim.id, 0, prop_name, 0);
	}

	return anim;
}

static ufbxwi_forceinline ufbxwi_node *ufbxwi_get_node_data(ufbxw_scene *scene, ufbxw_node id) { return (ufbxwi_node*)ufbxwi_get_element_data(scene, id.id); }
static ufbxwi_forceinline ufbxwi_mesh *ufbxwi_get_mesh_data(ufbxw_scene *scene, ufbxw_mesh id) { return (ufbxwi_mesh*)ufbxwi_get_element_data(scene, id.id); }
static ufbxwi_forceinline ufbxwi_anim_curve *ufbxwi_get_anim_curve(ufbxw_scene *scene, ufbxw_anim_curve id) { return (ufbxwi_anim_curve*)ufbxwi_get_element_data(scene, id.id); }
static ufbxwi_forceinline ufbxwi_anim_prop *ufbxwi_get_anim_prop(ufbxw_scene *scene, ufbxw_anim_prop id) { return (ufbxwi_anim_prop*)ufbxwi_get_element_data(scene, id.id); }
static ufbxwi_forceinline ufbxwi_anim_layer *ufbxwi_get_anim_layer(ufbxw_scene *scene, ufbxw_anim_layer id) { return (ufbxwi_anim_layer*)ufbxwi_get_element_data(scene, id.id); }
static ufbxwi_forceinline ufbxwi_anim_stack *ufbxwi_get_anim_stack(ufbxw_scene *scene, ufbxw_anim_stack id) { return (ufbxwi_anim_stack*)ufbxwi_get_element_data(scene, id.id); }

static ufbxwi_forceinline ufbxw_node ufbxwi_assert_node(ufbxw_id id) { ufbxw_assert(ufbxwi_id_type(id) == UFBXW_ELEMENT_NODE); ufbxw_node v = { id }; return v; }
static ufbxwi_forceinline ufbxw_anim_curve ufbxwi_assert_anim_curve(ufbxw_id id) { ufbxw_assert(ufbxwi_id_type(id) == UFBXW_ELEMENT_ANIM_CURVE); ufbxw_anim_curve v = { id }; return v; }

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
		et->desc = desc;

		et->object_type_id = ~0u;
		for (uint32_t i = 0; i < scene->object_types.count; i++) {
			if (scene->object_types.data[i].type == desc->object_type) {
				et->object_type_id = i;
				break;
			}
		}
		ufbxw_assert(et->object_type_id != ~0u || (desc->flags & UFBXWI_ELEMENT_FLAG_ALLOW_NO_OBJECT_ID) != 0);
	}

	return true;
}

static void ufbxwi_create_defaults(ufbxw_scene *scene)
{
	if (!scene->opts.no_default_scene_info) {
		ufbxw_id id = ufbxw_create_element(scene, UFBXW_ELEMENT_SCENE_INFO);
		ufbxw_set_name(scene, id, "GlobalInfo");

		// TODO: Fill these out
		ufbxw_add_string(scene, id, "DocumentUrl", UFBXW_PROP_TYPE_URL, "test.fbx");
		ufbxw_add_string(scene, id, "SrcDocumentUrl", UFBXW_PROP_TYPE_URL, "test.fbx");
	}

	if (!scene->opts.no_default_global_settings) {
		ufbxw_id id = ufbxw_create_element(scene, UFBXW_ELEMENT_GLOBAL_SETTINGS);

		// TODO: Make these (and the rest) into actual fast access fields
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

		ufbxw_set_string(scene, id, "ActiveAnimStackName", "Take 001");
	}

	if (!scene->opts.no_default_anim_stack) {
		ufbxw_anim_stack default_stack = ufbxw_create_anim_stack(scene);
		ufbxw_set_name(scene, default_stack.id, "Take 001");
		scene->default_anim_stack = default_stack;
	}

	if (!scene->opts.no_default_anim_layer) {
		ufbxw_anim_layer default_layer = ufbxw_create_anim_layer(scene, scene->default_anim_stack);
		ufbxw_set_name(scene, default_layer.id, "BaseLayer");
		scene->default_anim_layer = default_layer;
	}
}

static void ufbxwi_init_scene(ufbxw_scene *scene)
{
	scene->string_pool.ator = &scene->ator;
	scene->string_pool.error = &scene->error;

	ufbxwi_buffer_pool_init(&scene->buffers, &scene->ator, &scene->error);

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

// -- Geometry

static size_t ufbxwi_stream_triangle_faces(void *user, int32_t *dst, size_t dst_size, size_t offset)
{
	for (size_t i = 0; i < dst_size; i++) {
		*dst = (int32_t)((offset + i) * 3);
	}
	return dst_size;
}

#if 0
static bool ufbxwi_transform_polygon_vertex_order(ufbxwi_buffer_pool *pool, ufbxwi_buffer_transform *transform)
{
	const int32_t *indices = (const int32_t*)transform->src[0].begin;
	const int32_t *indices_end = (const int32_t*)transform->src[0].end;
	const int32_t *face_sizes = (const int32_t*)transform->src[1].begin;
	const int32_t *face_sizes_end = (const int32_t*)transform->src[1].end;
	int32_t *order = (int32_t*)transform->dst[0].begin;
	int32_t *order_end = (int32_t*)transform->dst[0].end;

	size_t space = ufbxwi_min_sz(ufbxwi_to_size(indices_end - indices), ufbxwi_to_size(order_end - order));

	while (face_sizes != face_sizes_end) {
		int32_t face_size_int = face_sizes[0];
		if (face_size_int < 0) return false;

		size_t face_size = (size_t)face_size_int;
		if (face_size > space) break;

		size_t last = face_size - 1;
		for (size_t i = 0; i < last; i++) {
			order[i] = indices[i];
		}
		order[last] = ~indices[last];

		face_sizes += 1;
		indices += face_size;
		order += face_size;
		space -= face_size;
	}

	transform->src[0].begin = (void*)indices;
	transform->src[1].begin = (void*)face_sizes;
	transform->dst[0].begin = (void*)order;
	return true;
}
#endif

// -- Saving

typedef struct {
	uint32_t reference_count;
	ufbxw_id template_id;
} ufbxwi_save_object_type;

UFBXWI_LIST_TYPE(ufbxwi_save_object_type_list, ufbxwi_save_object_type);

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
	ufbxwi_save_object_type_list object_types;
	ufbxwi_conn_list tmp_conns;

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
	// TODO: Sanitize \n's and other special characters
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
		case 'c': {
			char str = va_arg(args, char);
			ufbxwi_write(sc, &str, 1);
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
			// TODO: Force saving properties that are animated
			if (tmpl != NULL && tmpl->type == base->type) {
				const void *base_data = ufbxwi_resolve_value_offset(data_buffer, base->value_offset);
				const void *tmpl_data = ufbxwi_resolve_value_offset(template_buffer, tmpl->value_offset);
				size_t data_size = ufbxwi_prop_data_infos[scene->prop_types.data[base->type].data_type].size;
				if (data_size == 0 || !memcmp(base_data, tmpl_data, data_size)) {
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

		char flags[16];
		char *flag = flags;
		if (p->flags & UFBXW_PROP_FLAG_ANIMATABLE) *flag++ = 'A';
		*flag++ = '\0';
		ufbxw_assert(flag <= flags + sizeof(flags));

		switch (type->data_type) {
			case UFBXW_PROP_DATA_NONE: {
				ufbxwi_dom_value(sc, "P", "SSSC", name, type->type, type->sub_type, flags);
			} break;
			case UFBXW_PROP_DATA_BOOL: {
				const bool *d = (const bool*)data;
				ufbxwi_dom_value(sc, "P", "SSSCI", name, type->type, type->sub_type, flags, *d ? 1 : 0);
			} break;
			case UFBXW_PROP_DATA_INT32: {
				const int32_t *d = (const int32_t*)data;
				ufbxwi_dom_value(sc, "P", "SSSCI", name, type->type, type->sub_type, flags, *d);
			} break;
			case UFBXW_PROP_DATA_INT64: {
				const int64_t *d = (const int64_t*)data;
				ufbxwi_dom_value(sc, "P", "SSSCL", name, type->type, type->sub_type, flags, *d);
			} break;
			case UFBXW_PROP_DATA_REAL: {
				const ufbxw_real *d = (const ufbxw_real*)data;
				ufbxwi_dom_value(sc, "P", "SSSCD", name, type->type, type->sub_type, flags, *d);
			} break;
			case UFBXW_PROP_DATA_VEC2: {
				const ufbxw_vec2 *d = (const ufbxw_vec2*)data;
				ufbxwi_dom_value(sc, "P", "SSSCDD", name, type->type, type->sub_type, flags, d->x, d->y);
			} break;
			case UFBXW_PROP_DATA_VEC3: {
				const ufbxw_vec3 *d = (const ufbxw_vec3*)data;
				ufbxwi_dom_value(sc, "P", "SSSCDDD", name, type->type, type->sub_type, flags, d->x, d->y, d->z);
			} break;
			case UFBXW_PROP_DATA_VEC4: {
				const ufbxw_vec4 *d = (const ufbxw_vec4*)data;
				ufbxwi_dom_value(sc, "P", "SSSCDDDD", name, type->type, type->sub_type, flags, d->x, d->y, d->z, d->w);
			} break;
			case UFBXW_PROP_DATA_STRING: {
				const ufbxw_string *d = (const ufbxw_string*)data;
				ufbxwi_dom_value(sc, "P", "SSSCS", name, type->type, type->sub_type, flags, *d);
			} break;
			case UFBXW_PROP_DATA_ID: {
				ufbxwi_dom_value(sc, "P", "SSSC", name, type->type, type->sub_type, flags);
			} break;
			case UFBXW_PROP_DATA_REAL_STRING: {
				const ufbxw_real_string *d = (const ufbxw_real_string*)data;
				ufbxwi_dom_value(sc, "P", "SSSCDS", name, type->type, type->sub_type, flags, d->value, d->string);
			} break;
			case UFBXW_PROP_DATA_BLOB: {
				const ufbxw_blob *d = (const ufbxw_blob*)data;
				ufbxw_assert(0 && "TODO");
			} break;
			case UFBXW_PROP_DATA_USER_INT: {
				const ufbxw_user_int *d = (const ufbxw_user_int*)data;
				ufbxwi_dom_value(sc, "P", "SSSCIII", name, type->type, type->sub_type, flags, d->value, d->min_value, d->max_value);
			} break;
			case UFBXW_PROP_DATA_USER_REAL: {
				const ufbxw_user_real *d = (const ufbxw_user_real*)data;
				ufbxwi_dom_value(sc, "P", "SSSCDDD", name, type->type, type->sub_type, flags, d->value, d->min_value, d->max_value);
			} break;
			case UFBXW_PROP_DATA_USER_ENUM: {
				const ufbxw_user_enum *d = (const ufbxw_user_enum*)data;
				ufbxwi_dom_value(sc, "P", "SSSCIS", name, type->type, type->sub_type, flags, d->value, d->options);
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

static void ufbxwi_save_anim_curve_keys(ufbxw_save_context *sc, ufbxwi_element *element)
{
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
		ufbxwi_node *node = (ufbxwi_node*)element->data;

		ufbxwi_element *attrib = ufbxwi_get_element(scene, node->attribute);
		if (attrib) {
			const ufbxwi_element_type *attrib_et = &scene->element_types.data[attrib->type_id];
			sub_type = attrib_et->sub_type;
		} else {
			sub_type = UFBXWI_TOKEN_EMPTY; // TODO: What is the correct thing here?
		}
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
		ufbxwi_dom_value(sc, "Version", "I", 232);
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
	} else if (type == UFBXW_ELEMENT_MATERIAL) {
		const ufbxwi_material *material = (const ufbxwi_material*)element->data;
		ufbxwi_dom_value(sc, "Version", "I", 120);
		ufbxwi_dom_value(sc, "ShadingModel", "S", material->shading_model);
		ufbxwi_dom_value(sc, "MultiLayer", "I", material->multi_layer ? 1 : 0);
	} else if (type == UFBXW_ELEMENT_TEXTURE) {
		const ufbxwi_texture *texture = (const ufbxwi_texture*)element->data;
		ufbxwi_dom_value(sc, "Type", "S", texture->type);
		ufbxwi_dom_value(sc, "Version", "I", 202);
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

	if (type == UFBXW_ELEMENT_NODE) {
		// TODO: Use actual values for these
		ufbxwi_dom_value(sc, "Shading", "c", 'T');
		ufbxwi_dom_value(sc, "Culling", "C", "CullingOff");
	}

	if (type == UFBXW_ELEMENT_LIGHT) {
		// TODO: Use actual values for these
		ufbxwi_dom_value(sc, "TypeFlags", "C", "Light");
		ufbxwi_dom_value(sc, "GeometryVersion", "I", 124);
	}

	if (type == UFBXW_ELEMENT_TEXTURE) {
		const ufbxwi_texture *texture = (const ufbxwi_texture*)element->data;
		ufbxwi_dom_value(sc, "FileName", "S", texture->filename);
		ufbxwi_dom_value(sc, "RelativeFilename", "S", texture->relative_filename);
	}

	if (type == UFBXW_ELEMENT_ANIM_CURVE) {
		ufbxwi_dom_value(sc, "Default", "D", 0.0); // Type?
		ufbxwi_dom_value(sc, "KeyVer", "I", 4009);
		ufbxwi_save_anim_curve_keys(sc, element);
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

static void ufbxwi_save_definitions(ufbxw_save_context *sc)
{
	ufbxw_scene *scene = sc->scene;

	size_t object_type_count = 0;
	ufbxwi_for_list(ufbxwi_save_object_type, obj_type, sc->object_types) {
		if (obj_type->reference_count == 0) continue;
		object_type_count++;
	}

	ufbxwi_dom_open(sc, "Definitions", "");
	ufbxwi_dom_value(sc, "Version", "I", 100);
	ufbxwi_dom_value(sc, "Count", "I", (int32_t)object_type_count);

	for (size_t i = 0; i < sc->object_types.count; i++) {
		const ufbxwi_object_type *scene_obj_type = &scene->object_types.data[i];
		ufbxwi_save_object_type *obj_type = &sc->object_types.data[i];
		if (obj_type->reference_count == 0) continue;

		ufbxwi_dom_open(sc, "ObjectType", "T", scene_obj_type->type);
		ufbxwi_dom_value(sc, "Count", "I", (int32_t)obj_type->reference_count);

		ufbxwi_element *tmpl_elem = ufbxwi_get_element(scene, obj_type->template_id);
		if (tmpl_elem) {
			ufbxwi_template *tmpl = (ufbxwi_template*)tmpl_elem->data;
			ufbxwi_dom_open(sc, "PropertyTemplate", "T", tmpl->type);
			ufbxwi_save_props(sc, tmpl_elem->data, &tmpl_elem->props, NULL, NULL, NULL);
			ufbxwi_dom_close(sc);
		}

		ufbxwi_dom_close(sc);
	}

	ufbxwi_dom_close(sc);
}

static void ufbxwi_save_objects(ufbxw_save_context *sc)
{
	ufbxw_scene *scene = sc->scene;

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
}

static void ufbxwi_save_connections(ufbxw_save_context *sc)
{
	ufbxw_scene *scene = sc->scene;

	ufbxwi_dom_open(sc, "Connections", "");

	// Connect root nodes to root

	ufbxwi_for_list(ufbxwi_element, src_element, scene->elements) {
		ufbxw_id src_id = src_element->id;
		ufbxw_element_type src_type = ufbxwi_id_type(src_id);
		if (src_type != UFBXW_ELEMENT_NODE) continue;

		ufbxwi_node *node = (ufbxwi_node*)src_element->data;
		if (node->parent.id == 0) {
			if (sc->opts.ascii) {
				if (sc->opts.debug_comments) {
					ufbxwi_dom_comment(sc, "\n\t;Model::%s, Model::RootNode (Root Node)\n", src_element->name.data);
				} else {
					ufbxwi_dom_comment(sc, "\n\t;Model::%s, Model::RootNode\n", src_element->name.data);
				}
			}
			ufbxwi_dom_value(sc, "C", "CLL", "OO", src_id, 0);
		}
	}

	// TODO: This could be accelerated with a bit-mask of active connection types

	ufbxwi_for_list(ufbxwi_element, dst_element, scene->elements) {
		ufbxw_id dst_id = dst_element->id;

		for (uint32_t i = 1; i < UFBXW_CONNECTION_TYPE_COUNT; i++) {
			ufbxw_connection_type conn_type = (ufbxw_connection_type)i;

			sc->tmp_conns.count = 0;
			ufbxwi_collect_src_connections(scene, &sc->ator, &sc->tmp_conns, conn_type, dst_element);

			ufbxwi_for_list(ufbxwi_conn, p_conn, sc->tmp_conns) {
				ufbxwi_conn conn = *p_conn;
				ufbxw_id src_id = conn.id;

				ufbxwi_element *src_element = ufbxwi_get_element(scene, conn.id);
				ufbxw_assert(src_element);

				if (sc->opts.ascii) {
					const ufbxwi_element_type *src_et = &scene->element_types.data[src_element->type_id];
					const ufbxwi_element_type *dst_et = &scene->element_types.data[dst_element->type_id];
					const char *src_type = scene->string_pool.tokens.data[src_et->fbx_type].data;
					const char *dst_type = scene->string_pool.tokens.data[dst_et->fbx_type].data;
					const char *src_name = src_element->name.data;
					const char *dst_name = dst_element->name.data;

					if (sc->opts.debug_comments) {
						const char *conn_name = ufbxwi_connection_infos[conn_type].debug_name;
						ufbxwi_dom_comment(sc, "\n\t;%s::%s, %s::%s (%s)\n", src_type, src_name, dst_type, dst_name, conn_name);
					} else {
						ufbxwi_dom_comment(sc, "\n\t;%s::%s, %s::%s\n", src_type, src_name, dst_type, dst_name);
					}
				}

				if (conn.src_prop == UFBXWI_TOKEN_NONE && conn.dst_prop == UFBXWI_TOKEN_NONE) {
					ufbxwi_dom_value(sc, "C", "CLL", "OO", src_id, dst_id);
				} else if (conn.src_prop == UFBXWI_TOKEN_NONE && conn.dst_prop != UFBXWI_TOKEN_NONE) {
					ufbxwi_dom_value(sc, "C", "CLLT", "OP", src_id, dst_id, conn.dst_prop);
				} else if (conn.src_prop != UFBXWI_TOKEN_NONE && conn.dst_prop == UFBXWI_TOKEN_NONE) {
					ufbxwi_dom_value(sc, "C", "CLTL", "PO", src_id, conn.src_prop, dst_id);
				} else if (conn.src_prop != UFBXWI_TOKEN_NONE && conn.dst_prop != UFBXWI_TOKEN_NONE) {
					ufbxwi_dom_value(sc, "C", "CLTLT", "PP", src_id, conn.src_prop, dst_id, conn.dst_prop);
				}
			}
		}
	}

	ufbxwi_dom_close(sc);
}

static void ufbxwi_save_takes(ufbxw_save_context *sc)
{
	ufbxwi_dom_open(sc, "Takes", "");

	ufbxwi_dom_value(sc, "Current", "C", "Take 001");

	ufbxwi_dom_open(sc, "Take", "C", "Take 001");
	ufbxwi_dom_value(sc, "FileName", "C", "Take_001.tak");
	ufbxwi_dom_value(sc, "LocalTime", "LL", (int64_t)1924423250, (int64_t)230930790000);
	ufbxwi_dom_value(sc, "ReferenceTime", "LL", (int64_t)1924423250, (int64_t)230930790000);
	ufbxwi_dom_close(sc);

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
	ufbxwi_save_definitions(sc);

	ufbxwi_dom_section(sc, "Object properties");
	ufbxwi_save_objects(sc);

	ufbxwi_dom_section(sc, "Object connections");
	ufbxwi_save_connections(sc);

	// Differently formatted section...
	ufbxwi_dom_comment(sc, ";Takes section\n;%.52s\n\n", ufbxwi_dom_section_str);

	ufbxwi_save_takes(sc);
}

static void ufbxwi_save_init(ufbxw_save_context *sc)
{
	ufbxw_scene *scene = sc->scene;

	ufbxwi_save_object_type *object_types = ufbxwi_list_push_zero_n(&sc->ator, &sc->object_types, ufbxwi_save_object_type, scene->object_types.count);
	ufbxwi_check_err(&sc->error, object_types);

	ufbxwi_for_list(ufbxwi_element, element, scene->elements) {
		const ufbxwi_element_type *et = &scene->element_types.data[element->type_id];
		if (et->object_type_id != ~0u) {
			ufbxwi_save_object_type *object_type = &object_types[et->object_type_id];
			object_type->reference_count++;

			// TODO: Better prioritization?
			if (object_type->template_id == 0 && et->template_id != 0) {
				object_type->template_id = et->template_id;
			}
		}
	}

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

	ufbxwi_save_init(sc);
	ufbxwi_save_root(sc);
	ufbxwi_write_flush(sc);

	return true;
}

// -- API

ufbxw_abi_data const ufbxw_string ufbxw_empty_string = { ufbxwi_empty_char, 0 };
ufbxw_abi_data const ufbxw_vec2 ufbxw_zero_vec2 = { 0.0f, 0.0f };
ufbxw_abi_data const ufbxw_vec3 ufbxw_zero_vec3 = { 0.0f, 0.0f, 0.0f };
ufbxw_abi_data const ufbxw_vec4 ufbxw_zero_vec4 = { 0.0f, 0.0f, 0.0f, 0.0f };

ufbxw_abi void ufbxw_retain_buffer(ufbxw_scene *scene, ufbxw_buffer_id buffer)
{
	ufbxwi_buffer *buf = ufbxwi_get_buffer(&scene->buffers, buffer);
	ufbxw_assert(buf);
	if (!buf) return;

	ufbxw_assert(buf->user_refcount > 0);
	buf->user_refcount++;
	buf->refcount++;
}

ufbxw_abi void ufbxw_free_buffer(ufbxw_scene *scene, ufbxw_buffer_id buffer)
{
	ufbxwi_buffer *buf = ufbxwi_get_buffer(&scene->buffers, buffer);
	ufbxw_assert(buf);
	if (!buf) return;

	ufbxw_assert(buf->user_refcount > 0);
	buf->user_refcount--;

	ufbxwi_free_buffer(&scene->buffers, buffer);
}

ufbxw_abi void ufbxw_buffer_set_deleter(ufbxw_scene *scene, ufbxw_buffer_id buffer, ufbxw_buffer_deleter_fn *fn, void *user)
{
	ufbxwi_buffer *buf = ufbxwi_get_buffer(&scene->buffers, buffer);
	if (!buf) return;

	ufbxw_assert(buf->deleter_fn == NULL);
	buf->deleter_fn = fn;
	buf->deleter_user = user;
}

ufbxw_abi ufbxw_int_buffer ufbxw_create_int_buffer(ufbxw_scene *scene, size_t count)
{
	ufbxw_buffer_id id = ufbxwi_create_ownded_buffer(&scene->buffers, UFBXWI_BUFFER_TYPE_INT, count);
	return ufbxwi_to_user_int_buffer(&scene->buffers, id);
}

ufbxw_abi ufbxw_int_buffer ufbxw_copy_int_array(ufbxw_scene *scene, const int32_t *data, size_t count)
{
	ufbxw_buffer_id id = ufbxwi_create_copy_buffer(&scene->buffers, UFBXWI_BUFFER_TYPE_INT, data, count);
	return ufbxwi_to_user_int_buffer(&scene->buffers, id);
}

ufbxw_abi ufbxw_int_buffer ufbxw_external_int_array(ufbxw_scene *scene, const int32_t *data, size_t count)
{
	ufbxw_buffer_id id = ufbxwi_create_external_buffer(&scene->buffers, UFBXWI_BUFFER_TYPE_INT, data, count);
	return ufbxwi_to_user_int_buffer(&scene->buffers, id);
}

ufbxw_abi ufbxw_int_buffer ufbxw_defer_int_stream(ufbxw_scene *scene, ufbxw_int_stream_fn *fn, void *user, size_t count)
{
	ufbxwi_stream_fn stream_fn;
	stream_fn.int_fn = fn;
	ufbxw_buffer_id id = ufbxwi_create_stream_buffer(&scene->buffers, UFBXWI_BUFFER_TYPE_INT, stream_fn, user);
	return ufbxwi_to_user_int_buffer(&scene->buffers, id);
}

ufbxw_abi ufbxw_vec3_buffer ufbxw_create_vec3_buffer(ufbxw_scene *scene, size_t count)
{
	ufbxw_buffer_id id = ufbxwi_create_ownded_buffer(&scene->buffers, UFBXWI_BUFFER_TYPE_VEC3, count);
	return ufbxwi_to_user_vec3_buffer(&scene->buffers, id);
}

ufbxw_abi ufbxw_vec3_buffer ufbxw_copy_vec3_array(ufbxw_scene *scene, const ufbxw_vec3 *data, size_t count);

// TODO: Lock/unlock version for Rust
ufbxw_abi ufbxw_int_list ufbxw_edit_int_buffer(ufbxw_scene *scene, ufbxw_int_buffer buffer)
{
	ufbxw_assert(ufbxwi_buffer_id_type(buffer.id) == UFBXWI_BUFFER_TYPE_VEC3);
	ufbxwi_buffer *buf = ufbxwi_get_buffer(&scene->buffers, buffer.id);
	ufbxw_int_list result = { NULL, 0 };
	if (buf && buf->state == UFBXWI_BUFFER_STATE_OWNED) {
		result.data = buf->data.owned.data;
		result.count = buf->count;
	}
	return result;
}

ufbxw_abi ufbxw_vec3_list ufbxw_edit_vec3_buffer(ufbxw_scene *scene, ufbxw_vec3_buffer buffer)
{
	ufbxw_assert(ufbxwi_buffer_id_type(buffer.id) == UFBXWI_BUFFER_TYPE_VEC3);
	ufbxwi_buffer *buf = ufbxwi_get_buffer(&scene->buffers, buffer.id);
	ufbxw_vec3_list result = { NULL, 0 };
	if (buf && buf->state == UFBXWI_BUFFER_STATE_OWNED) {
		result.data = buf->data.owned.data;
		result.count = buf->count;
	}
	return result;
}

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

	// TODO: This could be accelerated with a bit-mask of active connection types
	for (uint32_t i = 1; i < UFBXW_CONNECTION_TYPE_COUNT; i++) {
		ufbxw_connection_type conn_type = (ufbxw_connection_type)i;
		ufbxwi_disconnect_all_dst(scene, conn_type, id);
		ufbxwi_disconnect_all_src(scene, conn_type, id);
	}

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
	for (uint32_t i = 1; i < UFBXW_CONNECTION_TYPE_COUNT; i++) {
		ufbxw_connection_type conn_type = (ufbxw_connection_type)i;
		if (ufbxwi_connect(scene, conn_type, src, dst, 0)) break;
	}
}

ufbxw_abi void ufbxw_connect_prop(ufbxw_scene *scene, ufbxw_id src, const char *src_prop, ufbxw_id dst, const char *dst_prop)
{
	ufbxw_connect_prop_len(scene, src, src_prop, src_prop ? strlen(src_prop) : 0, dst, dst_prop, dst_prop ? strlen(dst_prop) : 0);
}

ufbxw_abi void ufbxw_connect_prop_len(ufbxw_scene *scene, ufbxw_id src, const char *src_prop, size_t src_prop_len, ufbxw_id dst, const char *dst_prop, size_t dst_prop_len)
{
	ufbxwi_token src_token = src_prop_len > 0 ? ufbxwi_intern_token(&scene->string_pool, src_prop, src_prop_len) : UFBXWI_TOKEN_NONE;
	ufbxwi_token dst_token = dst_prop_len > 0 ? ufbxwi_intern_token(&scene->string_pool, dst_prop, dst_prop_len) : UFBXWI_TOKEN_NONE;

	for (uint32_t i = 1; i < UFBXW_CONNECTION_TYPE_COUNT; i++) {
		ufbxw_connection_type conn_type = (ufbxw_connection_type)i;
		if (ufbxwi_connect_imp(scene, conn_type, src, dst, src_token, dst_token, 0)) break;
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

ufbxw_abi ufbxw_anim_prop ufbxw_animate_prop(ufbxw_scene *scene, ufbxw_id id, const char *prop, ufbxw_anim_layer layer)
{
	return ufbxw_animate_prop_len(scene, id, prop, strlen(prop), layer);
}

ufbxw_abi ufbxw_anim_prop ufbxw_animate_prop_len(ufbxw_scene *scene, ufbxw_id id, const char *prop, size_t prop_len, ufbxw_anim_layer layer)
{
	ufbxwi_token token = ufbxwi_intern_token(&scene->string_pool, prop, prop_len);
	if (!token) return ufbxw_null_anim_prop;

	return ufbxwi_animate_prop(scene, id, token, layer);
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

ufbxw_abi ufbxw_anim_prop ufbxw_node_animate_translation(ufbxw_scene *scene, ufbxw_node node, ufbxw_anim_layer layer)
{
	return ufbxwi_animate_prop(scene, node.id, UFBXWI_Lcl_Translation, layer);
}

ufbxw_abi ufbxw_anim_prop ufbxw_node_animate_rotation(ufbxw_scene *scene, ufbxw_node node, ufbxw_anim_layer layer)
{
	return ufbxwi_animate_prop(scene, node.id, UFBXWI_Lcl_Rotation, layer);
}

ufbxw_abi ufbxw_anim_prop ufbxw_node_animate_scaling(ufbxw_scene *scene, ufbxw_node node, ufbxw_anim_layer layer)
{
	return ufbxwi_animate_prop(scene, node.id, UFBXWI_Lcl_Scaling, layer);
}

ufbxw_abi void ufbxw_node_set_parent(ufbxw_scene *scene, ufbxw_node node, ufbxw_node parent)
{
	if (parent.id == ufbxw_null_id) {
		ufbxwi_disconnect_all_dst(scene, UFBXW_CONNECTION_NODE_PARENT, node.id);
	} else {
		ufbxwi_connect(scene, UFBXW_CONNECTION_NODE_PARENT, node.id, parent.id, UFBXWI_CONNECT_FLAG_DISCONNECT_SRC);
	}
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

ufbxw_abi void ufbxw_mesh_set_vertices(ufbxw_scene *scene, ufbxw_mesh mesh, ufbxw_vec3_buffer vertices)
{
	ufbxwi_mesh *data = ufbxwi_get_mesh_data(scene, mesh);
	ufbxwi_check(data);

	ufbxwi_set_buffer_from_user(&scene->buffers, &data->vertices.id, vertices.id);
}

ufbxw_abi void ufbxw_mesh_set_triangles(ufbxw_scene *scene, ufbxw_mesh mesh, ufbxw_int_buffer indices)
{
	size_t index_count = ufbxwi_get_buffer_size(&scene->buffers, indices.id);
	ufbxw_assert(index_count % 3 == 0);

	size_t tri_count = index_count / 3;
	ufbxw_int_buffer face_offsets = ufbxw_defer_int_stream(scene, &ufbxwi_stream_triangle_faces, NULL, tri_count + 1);
	ufbxwi_check(face_offsets.id);

	ufbxwi_mesh *md = ufbxwi_get_mesh_data(scene, mesh);
	ufbxwi_check(md);

	ufbxwi_set_buffer(&scene->buffers, &md->polygon_vertex_order.id, 0);
	ufbxwi_set_buffer_from_user(&scene->buffers, &md->vertex_indices.id, indices.id);
	ufbxwi_set_buffer(&scene->buffers, &md->face_offsets.id, face_offsets.id);
}

ufbxw_abi void ufbxw_mesh_set_polygons(ufbxw_scene *scene, ufbxw_mesh mesh, ufbxw_int_buffer indices, ufbxw_int_buffer face_offsets)
{
	ufbxwi_mesh *md = ufbxwi_get_mesh_data(scene, mesh);
	ufbxwi_check(md);

	ufbxwi_set_buffer(&scene->buffers, &md->polygon_vertex_order.id, 0);
	ufbxwi_set_buffer_from_user(&scene->buffers, &md->vertex_indices.id, indices.id);
	ufbxwi_set_buffer_from_user(&scene->buffers, &md->face_offsets.id, face_offsets.id);
}

ufbxw_abi void ufbxw_mesh_set_fbx_polygon_vertex_order(ufbxw_scene *scene, ufbxw_mesh mesh, ufbxw_int_buffer polygon_vertex_order)
{
	ufbxwi_mesh *md = ufbxwi_get_mesh_data(scene, mesh);
	ufbxwi_check(md);

	ufbxwi_set_buffer(&scene->buffers, &md->vertex_indices.id, 0);
	ufbxwi_set_buffer(&scene->buffers, &md->face_offsets.id, 0);
	ufbxwi_set_buffer_from_user(&scene->buffers, &md->polygon_vertex_order.id, polygon_vertex_order.id);
}

ufbxw_vec3_buffer ufbxw_mesh_get_vertices(ufbxw_scene *scene, ufbxw_mesh mesh)
{
	ufbxwi_mesh *md = ufbxwi_get_mesh_data(scene, mesh);
	ufbxwi_check_return(md);
}

ufbxw_int_buffer ufbxw_mesh_get_vertex_indices(ufbxw_scene *scene, ufbxw_mesh mesh);
ufbxw_int_buffer ufbxw_mesh_get_face_offsets(ufbxw_scene *scene, ufbxw_mesh mesh);

ufbxw_abi void ufbxw_mesh_add_instance(ufbxw_scene *scene, ufbxw_mesh mesh, ufbxw_node node)
{
	ufbxwi_connect(scene, UFBXW_CONNECTION_NODE_ATTRIBUTE, mesh.id, node.id, 0);
	// ufbxw_connect(scene, mesh.id, node.id);
}

ufbxw_abi ufbxw_anim_stack ufbxw_get_default_anim_stack(ufbxw_scene *scene)
{
	return scene->default_anim_stack;
}

ufbxw_abi ufbxw_anim_stack ufbxw_create_anim_stack(ufbxw_scene *scene)
{
	ufbxw_anim_stack stack = { ufbxw_create_element(scene, UFBXW_ELEMENT_ANIM_STACK) };
	return stack;
}

ufbxw_abi ufbxw_anim_layer ufbxw_anim_stack_get_layer(ufbxw_scene *scene, ufbxw_anim_stack stack, size_t index)
{
	ufbxwi_anim_stack *s = ufbxwi_get_anim_stack(scene, stack);
	if (s && index < s->layers.count) {
		ufbxw_anim_layer layer = { s->layers.data[index] };
		return layer;
	} else {
		return ufbxw_null_anim_layer;
	}
}

ufbxw_abi ufbxw_anim_layer ufbxw_get_default_anim_layer(ufbxw_scene *scene)
{
	return scene->default_anim_layer;
}

ufbxw_abi ufbxw_anim_layer ufbxw_create_anim_layer(ufbxw_scene *scene, ufbxw_anim_stack stack)
{
	ufbxw_anim_layer layer = { ufbxw_create_element(scene, UFBXW_ELEMENT_ANIM_LAYER) };

	if (stack.id != 0) {
		ufbxw_anim_layer_set_stack(scene, layer, stack);
	}

	return layer;
}

ufbxw_abi void ufbxw_anim_layer_set_weight(ufbxw_scene *scene, ufbxw_anim_layer layer, ufbxw_real weight)
{
	ufbxwi_anim_layer *l = ufbxwi_get_anim_layer(scene, layer);
	l->weight = weight;
}

ufbxw_abi void ufbxw_anim_layer_set_stack(ufbxw_scene *scene, ufbxw_anim_layer layer, ufbxw_anim_stack stack)
{
	ufbxwi_connect(scene, UFBXW_CONNECTION_ANIM_LAYER_STACK, layer.id, stack.id, UFBXWI_CONNECT_FLAG_DISCONNECT_SRC);
}

ufbxw_abi ufbxw_anim_curve ufbxw_anim_get_curve(ufbxw_scene *scene, ufbxw_anim_prop anim, size_t index)
{
	ufbxwi_anim_prop *p = ufbxwi_get_anim_prop(scene, anim);
	if (p && index < p->curves.count) {
		ufbxw_anim_curve curve = { p->curves.data[index].id };
		return curve;
	} else {
		return ufbxw_null_anim_curve;
	}
}

ufbxw_abi void ufbxw_anim_add_keyframe_real(ufbxw_scene *scene, ufbxw_anim_prop anim, ufbxw_ktime time, ufbxw_real value, uint32_t type)
{
	ufbxwi_anim_prop *ap = ufbxwi_get_anim_prop(scene, anim);
	if (!ap || ap->curves.count < 1) return;

	ufbxwi_conn *curves = ap->curves.data;
	ufbxw_anim_curve_add_keyframe(scene, ufbxwi_assert_anim_curve(curves[0].id), time, value, type);
}

ufbxw_abi void ufbxw_anim_add_keyframe_vec2(ufbxw_scene *scene, ufbxw_anim_prop anim, ufbxw_ktime time, ufbxw_vec2 value, uint32_t type)
{
	ufbxwi_anim_prop *ap = ufbxwi_get_anim_prop(scene, anim);
	if (!ap || ap->curves.count < 2) return;

	ufbxwi_conn *curves = ap->curves.data;
	ufbxw_anim_curve_add_keyframe(scene, ufbxwi_assert_anim_curve(curves[0].id), time, value.x, type);
	ufbxw_anim_curve_add_keyframe(scene, ufbxwi_assert_anim_curve(curves[1].id), time, value.y, type);
}

ufbxw_abi void ufbxw_anim_add_keyframe_vec3(ufbxw_scene *scene, ufbxw_anim_prop anim, ufbxw_ktime time, ufbxw_vec3 value, uint32_t type)
{
	ufbxwi_anim_prop *ap = ufbxwi_get_anim_prop(scene, anim);
	if (!ap || ap->curves.count < 3) return;

	ufbxwi_conn *curves = ap->curves.data;
	ufbxw_anim_curve_add_keyframe(scene, ufbxwi_assert_anim_curve(curves[0].id), time, value.x, type);
	ufbxw_anim_curve_add_keyframe(scene, ufbxwi_assert_anim_curve(curves[1].id), time, value.y, type);
	ufbxw_anim_curve_add_keyframe(scene, ufbxwi_assert_anim_curve(curves[2].id), time, value.z, type);
}

ufbxw_abi void ufbxw_anim_add_keyframe_real_key(ufbxw_scene *scene, ufbxw_anim_prop anim, ufbxw_keyframe_real key)
{
	ufbxwi_anim_prop *ap = ufbxwi_get_anim_prop(scene, anim);
	if (!ap || ap->curves.count < 1) return;

	ufbxwi_conn *curves = ap->curves.data;
	ufbxw_anim_curve_add_keyframe_key(scene, ufbxwi_assert_anim_curve(curves[0].id), key);
}

ufbxw_abi void ufbxw_anim_add_keyframe_vec2_key(ufbxw_scene *scene, ufbxw_anim_prop anim, ufbxw_keyframe_vec2 key)
{
	ufbxwi_anim_prop *ap = ufbxwi_get_anim_prop(scene, anim);
	if (!ap || ap->curves.count < 2) return;

	ufbxwi_conn *curves = ap->curves.data;

	{
		ufbxw_keyframe_real k = { key.time, key.value.x, key.flags, key.weight_left, key.weight_right, key.slope_left.x, key.slope_right.x };
		ufbxw_anim_curve_add_keyframe_key(scene, ufbxwi_assert_anim_curve(curves[0].id), k);
	}

	{
		ufbxw_keyframe_real k = { key.time, key.value.y, key.flags, key.weight_left, key.weight_right, key.slope_left.y, key.slope_right.y };
		ufbxw_anim_curve_add_keyframe_key(scene, ufbxwi_assert_anim_curve(curves[1].id), k);
	}
}

ufbxw_abi void ufbxw_anim_add_keyframe_vec3_key(ufbxw_scene *scene, ufbxw_anim_prop anim, ufbxw_keyframe_vec3 key)
{
	ufbxwi_anim_prop *ap = ufbxwi_get_anim_prop(scene, anim);
	if (!ap || ap->curves.count < 2) return;

	ufbxwi_conn *curves = ap->curves.data;

	{
		ufbxw_keyframe_real k = { key.time, key.value.x, key.flags, key.weight_left, key.weight_right, key.slope_left.x, key.slope_right.x };
		ufbxw_anim_curve_add_keyframe_key(scene, ufbxwi_assert_anim_curve(curves[0].id), k);
	}

	{
		ufbxw_keyframe_real k = { key.time, key.value.y, key.flags, key.weight_left, key.weight_right, key.slope_left.y, key.slope_right.y };
		ufbxw_anim_curve_add_keyframe_key(scene, ufbxwi_assert_anim_curve(curves[1].id), k);
	}

	{
		ufbxw_keyframe_real k = { key.time, key.value.z, key.flags, key.weight_left, key.weight_right, key.slope_left.z, key.slope_right.z };
		ufbxw_anim_curve_add_keyframe_key(scene, ufbxwi_assert_anim_curve(curves[2].id), k);
	}
}

ufbxw_abi void ufbxw_anim_set_layer(ufbxw_scene *scene, ufbxw_anim_prop anim, ufbxw_anim_layer layer)
{
	ufbxwi_connect(scene, UFBXW_CONNECTION_ANIM_PROP_LAYER, anim.id, layer.id, UFBXWI_CONNECT_FLAG_DISCONNECT_SRC);
}

ufbxw_abi void ufbxw_anim_curve_add_keyframe(ufbxw_scene *scene, ufbxw_anim_curve curve, ufbxw_ktime time, ufbxw_real value, uint32_t type)
{
	ufbxwi_anim_curve *c = ufbxwi_get_anim_curve(scene, curve);
	if (!c) return;

	// TODO: Pass by pointer to some implementation function
	// TODO: Assert that type does not depend on user tangents
	ufbxw_keyframe_real key = { time, value, type };
	ufbxw_anim_curve_add_keyframe_key(scene, curve, key);
}

ufbxw_abi void ufbxw_anim_curve_add_keyframe_key(ufbxw_scene *scene, ufbxw_anim_curve curve, ufbxw_keyframe_real key)
{
	ufbxwi_anim_curve *c = ufbxwi_get_anim_curve(scene, curve);
	if (!c) return;

	ufbxwi_list_push_copy(&scene->ator, &c->keys, ufbxw_keyframe_real, &key);
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
	ufbxwi_element_type *et = &scene->element_types.data[type_id];
	ufbxwi_check_return(ufbxwi_init_element_type(scene, et), 0);
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
