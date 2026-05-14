#include "ufbxwt_util.h"

#ifdef UFBXWT_HAS_LIBDEFLATE
	#include "../../extra/ufbxw_libdeflate.h"
#endif

#ifdef UFBXWT_HAS_ZLIB
	#include "../../extra/ufbxw_zlib.h"
#endif

#ifdef UFBXWT_HAS_FMTLIB
	#include "../../extra/ufbxw_fmtlib.h"
#endif

#ifdef UFBXWT_HAS_TO_CHARS
	#include "../../extra/ufbxw_to_chars.h"
#endif

#ifdef UFBXWT_HAS_ZMIJ
	#include "../../extra/ufbxw_zmij.h"
	#include "../../extra/ufbxw_zmij_c.h"
#endif

#ifdef UFBXWT_HAS_CPP_THREADS
	#include "../../extra/ufbxw_cpp_threads.h"
#endif

#include <assert.h>

#define ufbxwt_assert(cond) assert(cond)

static bool ufbxwt_bad_deflate_impl(void *user, ufbxw_deflate_compressor *compressor, int32_t compression_level)
{
	return false;
}

bool ufbxwt_deflate_setup(ufbxw_deflate *deflate, ufbxwt_deflate_impl impl)
{
	switch (impl) {
	case UFBXWT_DEFLATE_IMPL_BUILTIN:
		return true;

	case UFBXWT_DEFLATE_IMPL_LIBDEFLATE:
		#if UFBXWT_HAS_LIBDEFLATE
			ufbxw_libdeflate_setup(deflate, NULL);
			return true;
		#endif
		return false;

	case UFBXWT_DEFLATE_IMPL_ZLIB:
		#if UFBXWT_HAS_LIBDEFLATE
			ufbxw_zlib_setup(deflate, NULL);
			return true;
		#endif
		return false;

	case UFBXWT_DEFLATE_IMPL_NONE:
		deflate->create_cb.fn = &ufbxwt_bad_deflate_impl;
		return true;

	default:
		ufbxwt_assert(false);
		break;
	}
	return false;
}

const char *ufbxwt_deflate_impl_name(ufbxwt_deflate_impl impl)
{
	switch (impl) {
	case UFBXWT_DEFLATE_IMPL_BUILTIN: return "builtin";
	case UFBXWT_DEFLATE_IMPL_LIBDEFLATE: return "libdeflate";
	case UFBXWT_DEFLATE_IMPL_ZLIB: return "zlib";
	case UFBXWT_DEFLATE_IMPL_NONE: return "none";
	default: return "";
	}
}

bool ufbxwt_ascii_format_setup(ufbxw_ascii_formatter *formatter, ufbxwt_ascii_format_impl impl)
{
	switch (impl) {
	case UFBXWT_ASCII_FORMAT_IMPL_DEFAULT:
		return true;

	case UFBXWT_ASCII_FORMAT_IMPL_FMTLIB:
		#if UFBXWT_HAS_FMTLIB
			ufbxw_fmtlib_setup(formatter);
			return true;
		#endif
		return false;

	case UFBXWT_ASCII_FORMAT_IMPL_TO_CHARS:
		#if UFBXWT_HAS_TO_CHARS
			ufbxw_to_chars_setup(formatter);
			return true;
		#endif
		return false;

	case UFBXWT_ASCII_FORMAT_IMPL_ZMIJ:
		#if UFBXWT_HAS_ZMIJ
			ufbxw_zmij_setup(formatter);
			return true;
		#endif
		return false;

	case UFBXWT_ASCII_FORMAT_IMPL_ZMIJ_C:
		#if UFBXWT_HAS_ZMIJ
			ufbxw_zmij_c_setup(formatter);
			return true;
		#endif
		return false;

	default:
		ufbxwt_assert(false);
		break;
	}
	return false;
}

const char *ufbxwt_ascii_format_name(ufbxwt_ascii_format_impl impl)
{
	switch (impl) {
	case UFBXWT_ASCII_FORMAT_IMPL_DEFAULT: return "default";
	case UFBXWT_ASCII_FORMAT_IMPL_FMTLIB: return "fmtlib";
	case UFBXWT_ASCII_FORMAT_IMPL_TO_CHARS: return "to_chars";
	case UFBXWT_ASCII_FORMAT_IMPL_ZMIJ: return "zmij";
	case UFBXWT_ASCII_FORMAT_IMPL_ZMIJ_C: return "zmij_c";
	default: return "";
	}
}

bool ufbxwt_ascii_format_supports_float_format(ufbxwt_ascii_format_impl impl, ufbxw_ascii_float_format format)
{
	switch (impl) {
	case UFBXWT_ASCII_FORMAT_IMPL_DEFAULT: return true;
	case UFBXWT_ASCII_FORMAT_IMPL_FMTLIB: return true;
	case UFBXWT_ASCII_FORMAT_IMPL_TO_CHARS: return true;

	case UFBXWT_ASCII_FORMAT_IMPL_ZMIJ:
	case UFBXWT_ASCII_FORMAT_IMPL_ZMIJ_C:
		return format == UFBXW_ASCII_FLOAT_FORMAT_ROUND_TRIP;

	default:
		ufbxwt_assert(false);
		break;
	}
	return false;
}

bool ufbxwt_thread_setup(ufbxw_thread_sync *sync, ufbxw_thread_pool *pool, ufbxwt_thread_impl impl)
{
	switch (impl) {
	case UFBXWT_THREAD_IMPL_NONE:
		return true;

	case UFBXWT_THREAD_IMPL_CPP_THREADS:
		#if UFBXWT_HAS_CPP_THREADS
			ufbxw_cpp_threads_setup_sync(sync);
			ufbxw_cpp_threads_setup_pool(pool);
			return true;
		#endif
		return false;

	default:
		ufbxwt_assert(false);
		break;
	}
	return false;
}

const char *ufbxwt_thread_impl_name(ufbxwt_thread_impl impl)
{
	switch (impl) {
	case UFBXWT_THREAD_IMPL_NONE: return "none";
	case UFBXWT_THREAD_IMPL_CPP_THREADS: return "cpp_threads";
	default: return "";
	}
}

bool ufbxwt_thread_setup_any(ufbxw_thread_sync *sync, ufbxw_thread_pool *pool)
{
	for (uint32_t i = 1; i < UFBXWT_THREAD_IMPL_COUNT; i++) {
		if (ufbxwt_thread_setup(sync, pool, (ufbxwt_thread_impl)i)) {
			return true;
		}
	}

	return false;
}
