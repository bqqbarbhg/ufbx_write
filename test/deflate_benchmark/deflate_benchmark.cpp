#include "../util/cputime.h"
#include "../util/im_arg.h"
#include "../util/json_input.h"
#include "ufbx_deflate.h"

#include <stdio.h>
#include <vector>

bool g_verbose;

struct test_array
{
	const char *array_path;
	char type;
	uint32_t uncompressed_size;
	uint32_t compressed_size;
	uint32_t offset;
};

struct test_file
{
	const char *source_path;
	const char *data_path;
	size_t data_size;
	std::vector<test_array> arrays;
};

std::vector<char> g_src_buffer;
std::vector<char> g_dst_buffer;

const char *deflate_impl_name[] = {
	"zlib",
	"libd",
	"ufbx",
};

struct benchmark_decompress_result {
	uint64_t min_delta[UFBXWTE_DEFLATE_IMPL_COUNT];
	uint64_t max_delta[UFBXWTE_DEFLATE_IMPL_COUNT];
	size_t uncompressed_bytes;
	size_t compressed_bytes;
};

benchmark_decompress_result benchmark_decompress(const test_file &file)
{
	benchmark_decompress_result result = { };
	ufbxwte_deflate_decompressor dc[UFBXWTE_DEFLATE_IMPL_COUNT] = { };

	for (size_t impl = 0; impl < UFBXWTE_DEFLATE_IMPL_COUNT; impl++) {
		ufbxwt_deflate_init_decompressor(&dc[impl], (ufbxwte_deflate_impl)impl);
		result.min_delta[impl] = UINT64_MAX;
		result.max_delta[impl] = 0;
	}

	{
		FILE *f = fopen(file.data_path, "rb");
		fread(g_src_buffer.data(), 1, file.data_size, f);
		fclose(f);
	}

	for (const test_array &arr : file.arrays) {
		result.uncompressed_bytes += arr.uncompressed_size;
		result.compressed_bytes += arr.compressed_size;
	}

	for (size_t round = 0; round < 3; round++) {
		for (size_t impl = 0; impl < UFBXWTE_DEFLATE_IMPL_COUNT; impl++) {
			const uint64_t pre = cputime_cpu_tick();
			for (const test_array &arr : file.arrays) {
				void *dst = g_dst_buffer.data();
				size_t dst_size = arr.uncompressed_size;

				const void *src = g_src_buffer.data() + arr.offset;
				size_t src_size = arr.compressed_size;

				ufbxwte_deflate_decompress(&dc[impl], dst, dst_size, src, src_size);
			}
			const uint64_t post = cputime_cpu_tick();
			const uint64_t delta = post - pre;

			if (delta < result.min_delta[impl]) {
				result.min_delta[impl] = delta;
			}
			if (delta < result.max_delta[impl]) {
				result.max_delta[impl] = delta;
			}
		}
	}

	for (size_t impl = 0; impl < UFBXWTE_DEFLATE_IMPL_COUNT; impl++) {
		ufbxwte_deflate_free_decompressor(&dc[impl]);
	}

	return result;
}

int main(int argc, char **argv)
{
	const char *index_path = nullptr;

	im_arg_begin_c(argc, argv);
	while (im_arg_next()) {
		im_arg_help("--help", "show this help");

		if (im_arg("--verbose", "print verbose output")) {
			g_verbose = true;
		}
		if (im_arg("--index path", "path to index.json")) {
			index_path = im_arg_str(0);
		}
	}

	cputime_begin_init();
	uint64_t pre_json = cputime_cpu_tick();

	jsi_value *root = jsi_parse_file(index_path, NULL);
	uint64_t post_json = cputime_cpu_tick();
	cputime_end_init();

	if (g_verbose) {
		double duration = cputime_cpu_delta_to_sec(NULL, post_json - pre_json);
		printf("loaded index in %.2fms\n", duration * 1e3);
	}

	std::vector<test_file> test_files;
	size_t max_file_size = 0;
	size_t max_uncompressed_size = 0;

	for (const jsi_value &file : jsi_get(root->object, "data")->array) {
		test_file tf;
		tf.source_path = jsi_get(file.object, "source_path")->string;
		tf.data_path = jsi_get(file.object, "data_path")->string;
		tf.data_size = (size_t)jsi_get(file.object, "data_size")->number;

		if (tf.data_size > max_file_size) {
			max_file_size = tf.data_size;
		}

		for (const jsi_value &value : jsi_get(file.object, "arrays")->array) {
			jsi_obj *val = value.object;

			test_array arr = { };
			arr.type = jsi_get(val, "type")->string[0];
			arr.array_path = jsi_get(val, "path")->string;
			arr.uncompressed_size = (int32_t)jsi_get(val, "size_u")->number;
			arr.compressed_size = (int32_t)jsi_get(val, "size_z")->number;
			arr.offset = (int32_t)jsi_get(val, "offset")->number;

			if (arr.uncompressed_size > max_uncompressed_size) {
				max_uncompressed_size = arr.uncompressed_size;
			}

			tf.arrays.push_back(arr);
		}

		test_files.push_back(std::move(tf));
	}

	g_src_buffer.resize(max_file_size);
	g_dst_buffer.resize(max_uncompressed_size);

	for (const test_file &file : test_files) {
		benchmark_decompress_result result = benchmark_decompress(file);
		printf("\n%s\n", file.source_path);
		for (size_t impl = 0; impl < UFBXWTE_DEFLATE_IMPL_COUNT; impl++) {
			double time = (double)cputime_cpu_delta_to_sec(NULL, result.min_delta[impl]);
			printf("%8s: %.2fms (%.2fns/b)\n", deflate_impl_name[impl], time * 1e3, time * 1e9 / (double)result.uncompressed_bytes);
		}
	}

	jsi_free(root);

	return 0;
}
