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

typedef struct {
	ufbxwi_error *error;
	ufbxw_allocator ator;
	size_t max_size;
	size_t current_size;
	size_t num_allocs;
	size_t max_allocs;
} ufbxwi_allocator;

static ufbxwi_forceinline bool ufbxwi_does_overflow(size_t total, size_t a, size_t b)
{
	// If `a` and `b` have at most 4 bits per `size_t` byte, the product can't overflow.
	if (((a | b) >> sizeof(size_t)*4) != 0) {
		if (a != 0 && total / a != b) return true;
	}
	return false;
}

static ufbxwi_noinline void *ufbxwi_alloc_size(ufbxwi_allocator *ator, size_t size, size_t n)
{
	// Always succeed with an empty non-NULL buffer for empty allocations
	ufbxw_assert(size > 0);
	if (n == 0) return (void*)ufbxwi_zero_size_buffer;

	size_t total = size * n;
	ufbxwi_check_return_err(ator->error, !ufbxwi_does_overflow(total, size, n), NULL);
	ufbxwi_check_return_err(ator->error, total <= SIZE_MAX / 2, NULL); // Make sure it's always safe to double allocations
	ufbxwi_check_return_err(ator->error, total < ator->max_size - ator->current_size, NULL);
	ufbxwi_check_return_err(ator->error, ator->num_allocs < ator->max_allocs, NULL);
	ator->num_allocs++;

	void *ptr;
	if (ator->ator.alloc_fn) {
		ptr = ator->ator.alloc_fn(ator->ator.user, total);
	} else if (ator->ator.realloc_fn) {
		ptr = ator->ator.realloc_fn(ator->ator.user, NULL, 0, total);
	} else {
		ptr = ufbxw_malloc(total);
	}

	ufbxwi_check_return_err(ator->error, ptr, NULL);
	ufbxw_assert(ufbxwi_is_aligned_mask(ptr, ufbxwi_size_align_mask(total)));

	ator->current_size += total;

	return ptr;
}

static ufbxwi_noinline void ufbxwi_free_size(ufbxwi_allocator *ator, size_t size, void *ptr, size_t n)
{
	ufbxw_assert(size > 0);
	if (n == 0) return;
	ufbxw_assert(ptr);

	size_t total = size * n;

	// The old values have been checked by a previous allocate call
	ufbxw_assert(!ufbxwi_does_overflow(total, size, n));
	ufbxw_assert(total <= ator->current_size);

	ator->current_size -= total;

	if (ator->ator.alloc_fn || ator->ator.realloc_fn) {
		// Don't call default free() if there is an user-provided `alloc_fn()`
		if (ator->ator.free_fn) {
			ator->ator.free_fn(ator->ator.user, ptr, total);
		} else if (ator->ator.realloc_fn) {
			ator->ator.realloc_fn(ator->ator.user, ptr, total, 0);
		}
	} else {
		ufbxw_free(ptr, total);
	}
}

#define ufbxwi_alloc(ator, type, n) ufbxwi_maybe_null((type*)ufbxwi_alloc_size((ator), sizeof(type), (n)))
#define ufbxwi_free(ator, type, ptr, n) ufbxwi_free_size((ator), sizeof(type), (ptr), (n))

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
	new_capacity = ufbxwi_max_sz(new_capacity, 128 / size);
	char *new_data = ufbxwi_alloc_size(ator, size, new_capacity);
	ufbxwi_check_return_err(ator->error, new_data, NULL);

	list->data = new_data;
	list->capacity = new_capacity;
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
	ufbxwi_free_size(ator, size, list->data, list->capacity);
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

} ufbxwi_element;

UFBXWI_LIST_TYPE(ufbxwi_element_list, ufbxwi_element);

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

static void ufbxwi_free_scene(ufbxw_scene *scene)
{
	ufbxwi_list_free(&scene->ator, &scene->elements);
	ufbxwi_list_free(&scene->ator, &scene->free_element_ids);
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
	ator.max_size = SIZE_MAX;

	ufbxw_scene *scene = ufbxwi_alloc(&ator, ufbxw_scene, 1);
	if (!scene) return NULL;

	memset(scene, 0, sizeof(ufbxw_scene));
	scene->opts = *opts;
	scene->ator = ator;
	return scene;
}

ufbxw_abi void ufbxw_free_scene(ufbxw_scene *scene)
{
	ufbxwi_free_scene(scene);

	ufbxwi_allocator ator = scene->ator;
	ufbxwi_free(&ator, ufbxw_scene, scene, 1);

	ufbxw_assert(ator.current_size == 0);
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
	
	scene->num_elements++;

	return ufbxwi_make_id(type, generation, index);
}

ufbxw_abi void ufbxw_delete_element(ufbxw_scene *scene, ufbxw_id id)
{
	ufbxwi_element *element = ufbxwi_get_element(scene, id);
	ufbxwi_check(element);

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
