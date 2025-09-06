#ifndef UFBXW_ZLIB_H_INCLUDED
#define UFBXW_ZLIB_H_INCLUDED

#if !defined(ufbxw_zlib_abi)
	#if defined(UFBXW_zlib_STATIC)
		#define ufbxw_zlib_abi static
	#else
		#define ufbxw_zlib_abi
	#endif
#endif

typedef void* ufbxw_zlib_alloc_fn(void *user, unsigned int items, unsigned int size);
typedef void ufbxw_zlib_free_fn(void *user, void *address);

typedef struct ufbxw_zlib_allocator {
	ufbxw_zlib_alloc_fn *alloc_fn;
	ufbxw_zlib_free_fn *free_fn;
	void *user;
} ufbxw_zlib_allocator;

typedef struct ufbxw_zlib_opts {
	ufbxw_zlib_allocator allocator;
} ufbxw_zlib_opts;

ufbxw_zlib_abi void ufbxw_zlib_setup(struct ufbxw_deflate_compressor_cb *cb, const ufbxw_zlib_opts *opts);

#endif

#ifdef UFBXW_ZLIB_IMPLEMENTATION
#ifndef UFBXW_ZLIB_H_IMPLEMENTED
#define UFBXW_ZLIB_H_IMPLEMENTED

#if !defined(UFBXW_VERSION)
	#error "Please include ufbx_write.h before implementing ufbxw_zlib.h"
#endif

#if !defined(ZLIB_VERNUM)
	#error "Please include zlib.h before implementing ufbxw_zlib.h"
#endif

#include <stdlib.h>

static size_t ufbxw_zlib_begin(void *user, size_t input_size)
{
	z_stream *zs = (z_stream*)user;
	deflateReset(zs);
	return 0;
}

static bool ufbxw_zlib_advance(void *user, ufbxw_deflate_result *result, void *dst, size_t dst_size, const void *src, size_t src_size, uint32_t flags)
{
	if (src_size > UINT_MAX) src_size = UINT_MAX;
	if (dst_size > UINT_MAX) dst_size = UINT_MAX;

	z_stream *zs = (z_stream*)user;

    zs->next_in = (z_const Bytef*)src;
    zs->avail_in = (uInt)src_size;

    zs->next_out = (Bytef*)dst;
	zs->avail_out = (uInt)dst_size;

	int flush = 0;
	if (flags & UFBXW_DEFLATE_ADVANCE_FLUSH) {
		flush = Z_PARTIAL_FLUSH;
	} else if (flags & UFBXW_DEFLATE_ADVANCE_FINISH) {
		flush = Z_FINISH;
	}
	int res = deflate(zs, flush);
	if (res < 0) {
		return false;
	}

	result->bytes_read += (size_t)(zs->next_in - (z_const Bytef*)src);
	result->bytes_written += (size_t)(zs->next_out - (Bytef*)dst);
	return true;
}

static void ufbxw_zlib_free(void *user)
{
	z_stream *zs = (z_stream*)user;
	deflateEnd(zs);
	free(zs);
}

static bool ufbxw_zlib_init(void *user, ufbxw_deflate_compressor *compressor, int32_t compression_level)
{
	z_stream *zs = (z_stream*)malloc(sizeof(z_stream));
	if (!zs) return false;
	memset(zs, 0, sizeof(z_stream));

	if (deflateInit(zs, compression_level) != Z_OK) return false;

	const ufbxw_zlib_opts *opts = (const ufbxw_zlib_opts*)user;
	if (opts) {
		zs->zalloc = opts->allocator.alloc_fn;
		zs->zfree = opts->allocator.free_fn;
		zs->opaque = opts->allocator.user;
	}

	compressor->begin_fn = &ufbxw_zlib_begin;
	compressor->advance_fn = &ufbxw_zlib_advance;
	compressor->free_fn = &ufbxw_zlib_free;
	compressor->user = zs;

	return true;
}

ufbxw_zlib_abi void ufbxw_zlib_setup(ufbxw_deflate_compressor_cb *cb, const ufbxw_zlib_opts *opts)
{
	cb->fn = &ufbxw_zlib_init;
	cb->user = (void*)opts;
}

#endif
#endif
