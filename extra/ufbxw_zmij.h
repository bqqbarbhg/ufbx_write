#ifndef UFBXW_ZMIJ_H_INCLUDED
#define UFBXW_ZMIJ_H_INCLUDED

// Adapter for Zmij C++ implementation, for float/double formatting.
//
// https://github.com/dtolnay/zmij
//
// Only supports floating point formatting with `UFBXW_ASCII_FLOAT_FORMAT_ROUND_TRIP`.
//
// In one C++ translation unit, do the following:
//
//     #include "path/to/ufbx_write.h"
//     #include "path/to/zmij.h"
//
//     #define UFBXW_ZMIJ_IMPLEMENTATION
//     #include "path/to/ufbxw_zmij.h"
//
// You can still use `ufbxw_zmij_setup()` from C.

#include <stddef.h>

#if !defined(ufbxw_zmij_abi)
	#if defined(UFBXW_ZMIJ_STATIC)
		#define ufbxw_zmij_abi static
	#else
		#define ufbxw_zmij_abi
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
//     ufbxw_zmij_setup(&opts.ascii_formatter);
//
ufbxw_zmij_abi void ufbxw_zmij_setup(struct ufbxw_ascii_formatter *formatter);

#if defined(__cplusplus)
}
#endif

#endif

#ifdef UFBXW_ZMIJ_IMPLEMENTATION
#ifndef UFBXW_ZMIJ_H_IMPLEMENTED
#define UFBXW_ZMIJ_H_IMPLEMENTED

#if !defined(__cplusplus)
	#error "ufbxw_zmij.h should be implemented in a C++ file, though it can be used in another C file"
#endif

#if !defined(UFBXW_VERSION)
	#error "Please include ufbx_write.h before implementing ufbxw_zmij.h"
#endif

#if !defined(ZMIJ_H_)
	#error "Please include zmij.h before implementing ufbxw_zmij.h"
#endif

#include <stdint.h>

static size_t ufbxw_zmij_format_float(void *user, char *dst, size_t dst_size, const float *src, size_t src_count, ufbxw_ascii_float_format format)
{
	if (format != UFBXW_ASCII_FLOAT_FORMAT_ROUND_TRIP) return SIZE_MAX;

	char *d = dst;
	for (size_t i = 0; i < src_count; i++) {
		d = zmij::write(d, (size_t)zmij::float_buffer_size, src[i]);
		*d++ = ',';
	}
	return (size_t)(d - dst);
}

static size_t ufbxw_zmij_format_double(void *user, char *dst, size_t dst_size, const double *src, size_t src_count, ufbxw_ascii_float_format format)
{
	if (format != UFBXW_ASCII_FLOAT_FORMAT_ROUND_TRIP) return SIZE_MAX;

	char *d = dst;
	for (size_t i = 0; i < src_count; i++) {
		d = zmij::write(d, (size_t)zmij::double_buffer_size, src[i]);
		*d++ = ',';
	}
	return (size_t)(d - dst);
}

#if defined(__cplusplus)
extern "C" {
#endif

ufbxw_zmij_abi void ufbxw_zmij_setup(struct ufbxw_ascii_formatter *formatter)
{
	formatter->format_float_fn = &ufbxw_zmij_format_float;
	formatter->format_double_fn = &ufbxw_zmij_format_double;
}

#if defined(__cplusplus)
}
#endif

#endif
#endif
