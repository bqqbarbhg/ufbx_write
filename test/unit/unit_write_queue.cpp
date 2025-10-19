#include "unit_test.h"

#define UFBXW_UNIT_TEST 1
#define UFBXWI_FEATURE_ATOMICS 1
#define UFBXWI_FEATURE_THREAD_POOL 1
#define UFBXWI_FEATURE_ERROR 1
#define UFBXWI_FEATURE_ALLOCATOR 1
#define UFBXWI_FEATURE_LIST 1
#define UFBXWI_FEATURE_WRITE_QUEUE 1
#include "../../ufbx_write.c"

#include "util_allocator.h"

#include <vector>

#define UFBXWT_UNIT_CATEGORY "write_queue"

struct ufbxwt_vector_stream
{
	std::vector<char> buffer;

	bool write(uint64_t offset, const void *data, size_t size) {
		size_t begin = (size_t)offset;
		size_t end = begin + size;

		buffer.resize(end);
		memcpy(buffer.data() + begin, data, size);

		return true;
	}

	ufbxw_write_stream stream() {
		ufbxw_write_stream s = { };

		s.write_fn = [](void *user, uint64_t offset, const void *data, size_t size) {
			return ((ufbxwt_vector_stream*)user)->write(offset, data, size);
		};

		s.user = (void*)this;

		return s;
	}
};

UFBXWT_UNIT_TEST(write_queue_simple)
{
	ufbxwi_error error = { };
	ufbxwt_allocator ator { error };
	ufbxwt_vector_stream stream;

	ufbxwi_write_queue wq = { };
	ufbxwi_write_queue_init(&wq, &ator.ator, &error, stream.stream(), 128);

	const size_t write_length = 16 * 1024;

	for (size_t i = 0; i < write_length; i++) {
		char value[] = { (char)i };
		ufbxwi_queue_write(&wq, value, 1);
	}

	ufbxwi_write_queue_finish(&wq);
	ufbxwi_write_queue_free(&wq);

	ufbxwt_assert(stream.buffer.size() == write_length);
	for (size_t i = 0; i < write_length; i++) {
		ufbxwt_assert(stream.buffer[i] == (char)i);
	}
}

UFBXWT_UNIT_TEST(write_queue_direct)
{
	ufbxwi_error error = { };
	ufbxwt_allocator ator { error };
	ufbxwt_vector_stream stream;

	ufbxwi_write_queue wq = { };
	ufbxwi_write_queue_init(&wq, &ator.ator, &error, stream.stream(), 32);

	ufbxwi_queue_write(&wq, "Hello ", 6);

	std::vector<char> filler;
	filler.resize(1024, 'z');
	ufbxwi_queue_write(&wq, filler.data(), filler.size());

	ufbxwi_queue_write(&wq, " world", 6);

	ufbxwi_write_queue_finish(&wq);
	ufbxwi_write_queue_free(&wq);

	for (size_t i = 0; i < 6; i++) {
		ufbxwt_assert(stream.buffer[i] == "Hello "[i]);
	}
	for (size_t i = 6; i < 6 + 1024; i++) {
		ufbxwt_assert(stream.buffer[i] == 'z');
	}
	for (size_t i = 6 + 1024; i < 6 + 1024 + 6; i++) {
		ufbxwt_assert(stream.buffer[i] == " world"[i - (6 + 1024)]);
	}
}

