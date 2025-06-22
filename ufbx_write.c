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

#if 0

#define ufbxw_fabs fabs
#define ufbxw_asin asin
#define ufbxw_atan2 atan2
#define ufbxw_copysign copysign

struct ufbxw_writer
{
	ufbxw_error_fn *error_fn;
	void *error_user;

	ufbxw_error error;
	char error_buffer[256]; // TODO: Make this dynamic
};

#define UFBXWI_PI ((ufbx_real)3.14159265358979323846)
#define UFBXWI_DPI (3.14159265358979323846)
#define UFBXWI_DEG_TO_RAD ((ufbx_real)(UFBXWI_PI / 180.0))
#define UFBXWI_RAD_TO_DEG ((ufbx_real)(180.0 / UFBXWI_PI))
#define UFBXWI_DEG_TO_RAD_DOUBLE (UFBXWI_DPI / 180.0)
#define UFBXWI_RAD_TO_DEG_DOUBLE (180.0 / UFBXWI_DPI)
#define UFBXWI_MM_TO_INCH ((ufbx_real)0.0393700787)

static ufbxw_string ufbxw_string_c(const char *data)
{
	ufbxw_string s = { data, strlen(data) };
	return s;
}

static void ufbxwi_check_fail(ufbxw_writer *w, const char *func, const char *message)
{
	w->error.function = ufbxw_string_c(func);
	w->error.message = ufbxw_string_c(message);

	if (w->error_fn) {
		w->error_fn(w->error_user, &w->error);
	}
}

static void ufbxwi_check_failf(ufbxw_writer *w, const char *func, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(w->error_buffer, sizeof(w->error_buffer), fmt, args);
	va_end(args);

	ufbxwi_check_fail(w, func, w->error_buffer);
}

#define ufbxwi_check(w, cond, msg) do { \
		if (!(cond)) { \
			ufbxwi_check_fail((w), __FUNCTION__, (msg)); \
		} \
	} while (0)

#define ufbxwi_checkf(w, cond, ...) do { \
		if (!(cond)) { \
			ufbxwi_check_failf((w), __FUNCTION__, __VA_ARGS__); \
		} \
	} while (0)

// -- ..

bool ufbxwi_triangle_face_size_fn(void *user, int32_t *dst, size_t offset, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		dst[i] = 3;
	}
	return true;
}

// -- API

ufbxw_abi ufbxw_writer *ufbxw_create_writer()
{
	ufbxw_writer *w = calloc(1, sizeof(ufbxw_writer));
	return w;
}

ufbxw_abi void ufbxw_free_writer(ufbxw_writer *w)
{
	free(w);
}

void ufbxw_set_error_callback(ufbxw_writer *w, ufbxw_error_fn *fn, void *user)
{
}

// -- Data

ufbxw_abi ufbxw_int_buffer ufbxw_int_array(const int32_t *data, size_t count)
{
}

ufbxw_abi ufbxw_int_buffer ufbxw_int_func(ufbxw_buffer_int_fn *fn, void *user)
{
}

ufbxw_abi ufbxw_vec2_buffer ufbxw_vec2_array(const ufbxw_vec2 *data, size_t count)
{
}

ufbxw_abi ufbxw_vec2_buffer ufbxw_vec2_func(ufbxw_buffer_vec2_fn *fn, void *user)
{
}

ufbxw_abi ufbxw_vec3_buffer ufbxw_vec3_array(const ufbxw_vec3 *data, size_t count)
{
}

ufbxw_abi ufbxw_vec3_buffer ufbxw_vec3_func(ufbxw_buffer_vec3_fn *fn, void *user)
{
}

// -- Low level

ufbxw_abi ufbxw_id ufbxw_create_element(ufbxw_writer *w, ufbxw_element_type type)
{
	return 0;
}

ufbxw_abi void ufbxw_set_name_len(ufbxw_writer *w, ufbxw_id id, const char *name, size_t name_len)
{
	ufbxw_set_string_len(w, id, UFBXW_ELEMENT_NAME, name, name_len);
}

ufbxw_abi void ufbxw_set_name(ufbxw_writer *w, ufbxw_id id, const char *name)
{
	ufbxw_element_set_name_len(w, id, name, strlen(name));
}

ufbxw_abi void ufbxw_connect(ufbxw_writer *w, ufbxw_id src, ufbxw_id dst)
{
}

ufbxw_abi void ufbxw_set_string_len(ufbxw_writer *w, ufbxw_id id, ufbxw_prop prop, const char *value, size_t value_len)
{
}

ufbxw_abi void ufbxw_set_string(ufbxw_writer *w, ufbxw_id id, ufbxw_prop prop, const char *value)
{
	ufbxw_set_string_len(w, id, prop, value, strlen(value));
}

ufbxw_abi void ufbxw_set_int(ufbxw_writer *w, ufbxw_id id, ufbxw_prop prop, int64_t value)
{
}

ufbxw_abi void ufbxw_set_vec3(ufbxw_writer *w, ufbxw_id id, ufbxw_prop prop, ufbxw_vec3 value)
{
}

ufbxw_abi ufbxw_node ufbxw_create_node(ufbxw_writer *w)
{
	ufbxw_node node = { ufbxw_create_element(w, UFBXW_ELEMENT_NODE) };
	return node;
}

ufbxw_abi void ufbxw_node_set_name_len(ufbxw_writer *w, ufbxw_node node, const char *name, size_t name_len) { ufbxw_element_set_name_len(w, node.id, name, name_len); }
ufbxw_abi void ufbxw_node_set_name(ufbxw_writer *w, ufbxw_node node, const char *name) { ufbxw_element_set_name(w, node.id, name); }

ufbxw_abi void ufbxw_node_set_parent(ufbxw_writer *w, ufbxw_node node, ufbxw_node parent)
{
	ufbxw_set_int(w, node.id, UFBXW_NODE_PARENT, (int64_t)parent.id);
}

ufbxw_abi void ufbxw_node_set_translation(ufbxw_writer *w, ufbxw_node node, ufbxw_vec3 value)
{
	ufbxw_set_vec3(w, node.id, UFBXW_P_Lcl_Translation, value);
}

ufbxw_abi void ufbxw_node_set_rotation_euler(ufbxw_writer *w, ufbxw_node node, ufbxw_vec3 value)
{
	ufbxw_node_set_rotation_euler_order(w, node, value, UFBXW_ROTATION_ORDER_XYZ);
}

ufbxw_abi void ufbxw_node_set_rotation_euler_order(ufbxw_writer *w, ufbxw_node node, ufbxw_vec3 value, ufbxw_rotation_order order)
{
	ufbxw_set_vec3(w, node.id, UFBXW_P_Lcl_Rotation, value);
	ufbxw_set_int(w, node.id, UFBXW_P_RotationOrder, order);
}

ufbxw_abi void ufbxw_node_set_rotation_quat(ufbxw_writer *w, ufbxw_node node, ufbxw_quat quat)
{
	ufbxw_node_set_rotation_quat_order(w, node, quat, UFBXW_ROTATION_ORDER_XYZ);
}

ufbxw_abi void ufbxw_node_set_rotation_quat_order(ufbxw_writer *w, ufbxw_node node, ufbxw_quat quat, ufbxw_rotation_order order)
{
	ufbxw_vec3 euler = ufbxw_quat_to_euler(quat, order);
	ufbxw_set_vec3(w, node.id, UFBXW_P_Lcl_Rotation, euler);
	ufbxw_set_int(w, node.id, UFBXW_P_RotationOrder, order);
}

ufbxw_abi void ufbxw_node_set_scale(ufbxw_writer *w, ufbxw_node node, ufbxw_vec3 value)
{
	ufbxw_set_vec3(w, node.id, UFBXW_P_Lcl_Scaling, value);
}

// -- Mesh

ufbxw_abi ufbxw_mesh ufbxw_create_mesh(ufbxw_writer *w)
{
	ufbxw_mesh mesh = { ufbxw_create_element(w, UFBXW_ELEMENT_MESH) };
	return mesh;
}

ufbxw_abi void ufbxw_mesh_add(ufbxw_writer *w, ufbxw_mesh mesh, ufbxw_node node)
{
	ufbxw_connect(w, mesh.id, node.id);
}

ufbxw_abi void ufbxw_mesh_set_indices_triangles(ufbxw_writer *w, ufbxw_mesh mesh, ufbxw_int_buffer indices)
{
	ufbxwi_checkf(w, indices.count % 3 == 0, "index count (%zu) must be divisible by 3", indices.count);

	ufbxw_int_buffer face_sizes = ufbxw_int_func(&ufbxwi_triangle_face_size_fn, NULL);
	ufbxw_mesh_set_indices(w, mesh, indices, face_sizes);
}

ufbxw_abi void ufbxw_mesh_set_indices(ufbxw_writer *w, ufbxw_mesh mesh, ufbxw_int_buffer indices, ufbxw_int_buffer face_sizes)
{
}

// -- Utility
 
ufbxw_abi ufbxw_vec3 ufbxw_quat_to_euler(ufbxw_quat q, ufbxw_rotation_order order)
{
	// TODO: Derive these rigorously
	const double eps = 0.999999999;

	double vx, vy, vz;
	double t;

	double qx = q.x, qy = q.y, qz = q.z, qw = q.w;

	// Generated by `misc/gen_quat_to_euler.py`
	switch (order) {
	case UFBXW_ROTATION_ORDER_XYZ:
		t = 2.0f*(qw*qy - qx*qz);
		if (ufbxw_fabs(t) < eps) {
			vy = ufbxw_asin(t);
			vz = ufbxw_atan2(2.0f*(qw*qz + qx*qy), 2.0f*(qw*qw + qx*qx) - 1.0f);
			vx = -ufbxw_atan2(-2.0f*(qw*qx + qy*qz), 2.0f*(qw*qw + qz*qz) - 1.0f);
		} else {
			vy = ufbxw_copysign(UFBXWI_DPI*0.5, t);
			vz = ufbxw_atan2(-2.0f*t*(qw*qx - qy*qz), t*(2.0f*qw*qy + 2.0f*qx*qz));
			vx = 0.0f;
		}
		break;
	case UFBXW_ROTATION_ORDER_XZY:
		t = 2.0f*(qw*qz + qx*qy);
		if (ufbxw_fabs(t) < eps) {
			vz = ufbxw_asin(t);
			vy = ufbxw_atan2(2.0f*(qw*qy - qx*qz), 2.0f*(qw*qw + qx*qx) - 1.0f);
			vx = -ufbxw_atan2(-2.0f*(qw*qx - qy*qz), 2.0f*(qw*qw + qy*qy) - 1.0f);
		} else {
			vz = ufbxw_copysign(UFBXWI_DPI*0.5, t);
			vy = ufbxw_atan2(2.0f*t*(qw*qx + qy*qz), -t*(2.0f*qx*qy - 2.0f*qw*qz));
			vx = 0.0f;
		}
		break;
	case UFBXW_ROTATION_ORDER_YZX:
		t = 2.0f*(qw*qz - qx*qy);
		if (ufbxw_fabs(t) < eps) {
			vz = ufbxw_asin(t);
			vx = ufbxw_atan2(2.0f*(qw*qx + qy*qz), 2.0f*(qw*qw + qy*qy) - 1.0f);
			vy = -ufbxw_atan2(-2.0f*(qw*qy + qx*qz), 2.0f*(qw*qw + qx*qx) - 1.0f);
		} else {
			vz = ufbxw_copysign(UFBXWI_DPI*0.5, t);
			vx = ufbxw_atan2(-2.0f*t*(qw*qy - qx*qz), t*(2.0f*qw*qz + 2.0f*qx*qy));
			vy = 0.0f;
		}
		break;
	case UFBXW_ROTATION_ORDER_YXZ:
		t = 2.0f*(qw*qx + qy*qz);
		if (ufbxw_fabs(t) < eps) {
			vx = ufbxw_asin(t);
			vz = ufbxw_atan2(2.0f*(qw*qz - qx*qy), 2.0f*(qw*qw + qy*qy) - 1.0f);
			vy = -ufbxw_atan2(-2.0f*(qw*qy - qx*qz), 2.0f*(qw*qw + qz*qz) - 1.0f);
		} else {
			vx = ufbxw_copysign(UFBXWI_DPI*0.5, t);
			vz = ufbxw_atan2(2.0f*t*(qw*qy + qx*qz), -t*(2.0f*qy*qz - 2.0f*qw*qx));
			vy = 0.0f;
		}
		break;
	case UFBXW_ROTATION_ORDER_ZXY:
		t = 2.0f*(qw*qx - qy*qz);
		if (ufbxw_fabs(t) < eps) {
			vx = ufbxw_asin(t);
			vy = ufbxw_atan2(2.0f*(qw*qy + qx*qz), 2.0f*(qw*qw + qz*qz) - 1.0f);
			vz = -ufbxw_atan2(-2.0f*(qw*qz + qx*qy), 2.0f*(qw*qw + qy*qy) - 1.0f);
		} else {
			vx = ufbxw_copysign(UFBXWI_DPI*0.5, t);
			vy = ufbxw_atan2(-2.0f*t*(qw*qz - qx*qy), t*(2.0f*qw*qx + 2.0f*qy*qz));
			vz = 0.0f;
		}
		break;
	case UFBXW_ROTATION_ORDER_ZYX:
		t = 2.0f*(qw*qy + qx*qz);
		if (ufbxw_fabs(t) < eps) {
			vy = ufbxw_asin(t);
			vx = ufbxw_atan2(2.0f*(qw*qx - qy*qz), 2.0f*(qw*qw + qz*qz) - 1.0f);
			vz = -ufbxw_atan2(-2.0f*(qw*qz - qx*qy), 2.0f*(qw*qw + qx*qx) - 1.0f);
		} else {
			vy = ufbxw_copysign(UFBXWI_DPI*0.5, t);
			vx = ufbxw_atan2(2.0f*t*(qw*qz + qx*qy), -t*(2.0f*qx*qz - 2.0f*qw*qy));
			vz = 0.0f;
		}
		break;
	default:
		vx = vy = vz = 0.0;
		break;
	}

	vx *= UFBXWI_RAD_TO_DEG_DOUBLE;
	vy *= UFBXWI_RAD_TO_DEG_DOUBLE;
	vz *= UFBXWI_RAD_TO_DEG_DOUBLE;

	ufbxw_vec3 v = { vx, vy, vz };
	return v;
}

#endif

#endif
