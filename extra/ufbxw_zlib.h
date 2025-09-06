#ifndef UFBXW_ZLIB_H_INCLUDED
#define UFBXW_ZLIB_H_INCLUDED

#if !defined(UFBXW_VERSION)
	#error "Please include ufbx_write.h before including ufbxw_zlib.h"
#endif

#if !defined(ZLIB_VERNUM)
	#error "Please include zlib.h before including ufbxw_zlib.h"
#endif

#include <stdlib.h>

static size_t ufbxw_zlib_begin(void *user, size_t input_size)
{
	z_stream *zs = (z_stream*)user;
	deflateReset(zs);
	return 0;
}

static bool ufbxw_zlib_advance(void *user, ufbxw_deflate_result *result, void *dst, size_t dst_size, const void *src, size_t src_size)
{
	if (src_size > UINT_MAX) src_size = UINT_MAX;
	if (dst_size > UINT_MAX) dst_size = UINT_MAX;

	z_stream *zs = (z_stream*)user;

    zs->next_in = (z_const Bytef*)src;
    zs->avail_in = (uInt)src_size;

    zs->next_out = (Bytef*)dst;
	zs->avail_out = (uInt)dst_size;

	if (deflate(zs, Z_NO_FLUSH) != Z_OK) {
		if (deflate(zs, Z_PARTIAL_FLUSH) != Z_OK) {
			// TODO: Better flushing
			return false;
		}
	}

	result->bytes_read += (size_t)(zs->next_in - (z_const Bytef*)src);
	result->bytes_written += (size_t)(zs->next_out - (Bytef*)dst);

	return true;
}

static void ufbxw_zlib_free(void *user)
{
	z_stream *zs = (z_stream*)user;
	deflateEnd(zs);
}

static bool ufbxw_zlib_init(void *user, ufbxw_deflate_compressor *compressor, int32_t compression_level)
{
	z_stream *zs = (z_stream*)malloc(sizeof(z_stream));
	if (!zs) return false;
	memset(zs, 0, sizeof(z_stream));

	if (deflateInit(zs, compression_level) != Z_OK) return false;

	return true;
}

static void ufbxw_zlib_setup(ufbxw_deflate_compressor_cb *cb)
{
	cb->fn = &ufbxw_zlib_init;
	cb->user = NULL; // TODO: Allocation options and such
}

#endif
