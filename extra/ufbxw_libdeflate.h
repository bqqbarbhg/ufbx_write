#ifndef UFBXW_LIBDEFLATE_H_INCLUDED
#define UFBXW_LIBDEFLATE_H_INCLUDED

#if !defined(UFBXW_VERSION)
	#error "Please include ufbx_write.h before including ufbxw_libdeflate.h"
#endif

#if !defined(LIBDEFLATE_VERSION_MAJOR)
	#error "Please include libdeflate.h before including ufbxw_libdeflate.h"
#endif

static size_t ufbxw_libdeflate_begin(void *user, size_t input_size)
{
	struct libdeflate_compressor *c = (struct libdeflate_compressor*)user;
	return libdeflate_zlib_compress_bound(c, input_size);
}

static bool ufbxw_libdeflate_advance(void *user, ufbxw_deflate_result *result, void *dst, size_t dst_size, const void *src, size_t src_size)
{
	struct libdeflate_compressor *c = (struct libdeflate_compressor*)user;
	size_t dst_written = libdeflate_zlib_compress(c, src, src_size, dst, dst_size);
	if (dst_written == 0) return false;

	result->bytes_read = src_size;
	result->bytes_written = dst_written;
	return true;
}

static void ufbxw_libdeflate_free(void *user)
{
	struct libdeflate_compressor *c = (struct libdeflate_compressor*)user;
	libdeflate_free_compressor(c);
}

static bool ufbxw_libdeflate_init(void *user, ufbxw_deflate_compressor *compressor, int32_t compression_level)
{
	const struct libdeflate_options *opts = (const struct libdeflate_options*)user;

	struct libdeflate_compressor *c = NULL;
	if (opts) {
		c = libdeflate_alloc_compressor_ex(compression_level, opts);
	} else {
		c = libdeflate_alloc_compressor(compression_level);
	}
	if (!c) return false;

	compressor->begin_fn = &ufbxw_libdeflate_begin;
	compressor->advance_fn = &ufbxw_libdeflate_advance;
	compressor->free_fn = &ufbxw_libdeflate_free;
	compressor->user = c;

	return true;
}

static void ufbxw_libdeflate_setup(ufbxw_deflate_compressor_cb *cb, const struct libdeflate_options *opts)
{
	cb->fn = &ufbxw_libdeflate_init;
	cb->user = (void*)opts;
}

#endif
