#ifndef UFBXW_ZMIJ_C_H_INCLUDED
#define UFBXW_ZMIJ_C_H_INCLUDED

// Adapter for Zmij C implementation, for float/double formatting.
//
// https://github.com/vitaut/zmij
//
// Only supports floating point formatting with `UFBXW_ASCII_FLOAT_FORMAT_ROUND_TRIP`.
//
// In one translation unit, do the following:
//
//     #include "path/to/ufbx_write.h"
//     #include "path/to/zmij-c.h"
//
//     #define UFBXW_ZMIJ_C_IMPLEMENTATION
//     #include "path/to/ufbxw_zmij_c.h"

#include <stddef.h>

#if !defined(ufbxw_zmij_c_abi)
	#if defined(UFBXW_ZMIJ_C_STATIC)
		#define ufbxw_zmij_c_abi static
	#else
		#define ufbxw_zmij_c_abi
	#endif
#endif

#if defined(__cplusplus)
extern "C" {
#endif

// Setup float/double formatter callbacks to Zmij.
//
// NOTE: This only sets `format_float_fn` and `format_double_fn`, and will use default integer formatters.
//
// This call preserves `format_int_fn/format_float_fn()`, so you can call this after another adapter:
//
//     ufbxw_<formatter>_setup(&opts.ascii_formatter);
//     ufbxw_zmij_c_setup(&opts.ascii_formatter);
//
ufbxw_zmij_c_abi void ufbxw_zmij_c_setup(struct ufbxw_ascii_formatter *formatter);

#if defined(__cplusplus)
}
#endif

#endif

#ifdef UFBXW_ZMIJ_C_IMPLEMENTATION
#ifndef UFBXW_ZMIJ_C_H_IMPLEMENTED
#define UFBXW_ZMIJ_C_H_IMPLEMENTED

#if !defined(UFBXW_VERSION)
	#error "Please include ufbx_write.h before implementing ufbxw_zmij_c.h"
#endif

#if !defined(ZMIJ_C_H_)
	#error "Please include zmij-c.h before implementing ufbxw_zmij_c.h"
#endif

#include <stdint.h>

#define ufbxw_zmij_static_assert(desc, cond) typedef char ufbxw_zmij_static_assert_##desc[(cond)?1:-1]

ufbxw_zmij_static_assert(float_buffer_size, zmij_float_buffer_size <= UFBXW_ASCII_FORMAT_FLOAT_CHARS + UFBXW_ASCII_FORMAT_EXTRA_BUFFER_CHARS);
ufbxw_zmij_static_assert(double_buffer_size, zmij_double_buffer_size <= UFBXW_ASCII_FORMAT_DOUBLE_CHARS + UFBXW_ASCII_FORMAT_EXTRA_BUFFER_CHARS);

static size_t ufbxw_zmij_c_format_float(void *user, char *dst, size_t dst_size, const float *src, size_t src_count, ufbxw_ascii_float_format format)
{
	if (format != UFBXW_ASCII_FLOAT_FORMAT_ROUND_TRIP) return SIZE_MAX;

	char *d = dst;
	for (size_t i = 0; i < src_count; i++) {
		d += zmij_write_float(d, zmij_float_buffer_size, src[i]);
		*d++ = ',';
	}
	return (size_t)(d - dst);
}

static size_t ufbxw_zmij_c_format_double(void *user, char *dst, size_t dst_size, const double *src, size_t src_count, ufbxw_ascii_float_format format)
{
	if (format != UFBXW_ASCII_FLOAT_FORMAT_ROUND_TRIP) return SIZE_MAX;

	char *d = dst;
	for (size_t i = 0; i < src_count; i++) {
		d += zmij_write_double(d, zmij_double_buffer_size, src[i]);
		*d++ = ',';
	}
	return (size_t)(d - dst);
}

#if defined(__cplusplus)
extern "C" {
#endif

ufbxw_zmij_c_abi void ufbxw_zmij_c_setup(struct ufbxw_ascii_formatter *formatter)
{
	formatter->format_float_fn = &ufbxw_zmij_c_format_float;
	formatter->format_double_fn = &ufbxw_zmij_c_format_double;
}

#if defined(__cplusplus)
}
#endif

#endif
#endif
