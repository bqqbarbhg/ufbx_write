#include "unit_test.h"

#define UFBXW_UNIT_TEST 1
#define UFBXWI_FEATURE_ATOMICS 1
#define UFBXWI_FEATURE_THREAD_POOL 1
#define UFBXWI_FEATURE_ERROR 1
#define UFBXWI_FEATURE_ALLOCATOR 1
#define UFBXWI_FEATURE_LIST 1
#define UFBXWI_FEATURE_TASK_QUEUE 1
#include "../../ufbx_write.c"

#include "util_threads.h"

#define UFBXWT_UNIT_CATEGORY "task_queue"

UFBXWT_UNIT_TEST(task_queue_simple)
{
	ufbxwi_error error = { };
}

