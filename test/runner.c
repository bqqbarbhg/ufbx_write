#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <stdint.h>
#include <stdbool.h>
void ufbxwt_assert_fail_imp(const char *file, uint32_t line, const char *expr);
static void ufbxwt_assert_fail(const char *file, uint32_t line, const char *expr) {
	ufbxwt_assert_fail_imp(file, line, expr);
}

#include "../ufbx_write.h"

#define ufbxwt_arraycount(arr) (sizeof(arr) / sizeof(*(arr)))

// -- Thread local

#define UFBXWT_HAS_THREADLOCAL 1

#if defined(_MSC_VER)
	#define ufbxwt_threadlocal __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__)
	#define ufbxwt_threadlocal __thread
#else
	#define ufbxwt_threadlocal
	#undef UFBXWT_HAS_THREADLOCAL
	#define UFBXWT_HAS_THREADLOCAL 0
#endif

#ifndef USE_SETJMP
#if !defined(__wasm__) && UFBXWT_HAS_THREADLOCAL
	#define USE_SETJMP 1
#else
	#define USE_SETJMP 0
#endif
#endif

#if USE_SETJMP

#include <setjmp.h>

#define ufbxwt_jmp_buf jmp_buf
#define ufbxwt_setjmp(env) setjmp(env)
#define ufbxwt_longjmp(env, status, file, line, expr) longjmp(env, status)

#else

#define ufbxwt_jmp_buf int
#define ufbxwt_setjmp(env) (0)

static void ufbxwt_longjmp(int env, int value, const char *file, uint32_t line, const char *expr)
{
	fprintf(stderr, "\nAssertion failed: %s:%u: %s\n", file, line, expr);
	exit(1);
}

#endif

#define ufbxwt_assert(cond) do { \
		if (!(cond)) ufbxwt_assert_fail_imp(__FILE__, __LINE__, #cond); \
	} while (0)

typedef struct {
	bool failed;
	const char *file;
	uint32_t line;
	const char *expr;
} ufbxwt_fail;

typedef struct {
	const char *group;
	const char *name;
	void (*func)(void);

	ufbxwt_fail fail;
} ufbxwt_test;

ufbxwt_test *g_current_test;

char data_root[256];

#define UFBXWT_IMPL 1
#define UFBXWT_TEST(name) void ufbxwt_test_fn_##name(void)
#define UFBXWT_TEST_GROUP ""

#include "all_tests.h"

#undef UFBXWT_IMPL
#undef UFBXWT_TEST
#undef UFBXWT_TEST_GROUP

#define UFBXWT_IMPL 0
#define UFBXWT_TEST(name) { UFBXWT_TEST_GROUP, #name, &ufbxwt_test_fn_##name },
#define UFBXWT_TEST_GROUP ""

ufbxwt_test g_tests[] = {
	#include "all_tests.h"
};

ufbxwt_jmp_buf g_test_jmp;
int g_verbose;

char g_log_buf[16*1024];
uint32_t g_log_pos;

char g_hint[8*1024];

ufbxw_error g_error;

#undef UFBXWT_IMPL
#undef UFBXWT_TEST
#undef UFBXWT_TEST_GROUP

typedef struct {
	const char *name;
	uint32_t num_total;
	uint32_t num_ran;
	uint32_t num_ok;
} ufbxwt_test_stats;

ufbxwt_test_stats g_test_groups[ufbxwt_arraycount(g_tests)];
size_t g_num_groups = 0;

ufbxwt_test_stats *ufbxwt_get_test_group(const char *name)
{
	for (size_t i = g_num_groups; i > 0; --i) {
		ufbxwt_test_stats *group = &g_test_groups[i - 1];
		if (!strcmp(group->name, name)) return group;
	}

	ufbxwt_test_stats *group = &g_test_groups[g_num_groups++];
	group->name = name;
	return group;
}

ufbxwt_threadlocal ufbxwt_jmp_buf *t_jmp_buf;

void ufbxwt_assert_fail_imp(const char *file, uint32_t line, const char *expr)
{
	if (t_jmp_buf) {
		ufbxwt_longjmp(*t_jmp_buf, 1, file, line, expr);
	}

	g_current_test->fail.failed = 1;
	g_current_test->fail.file = file;
	g_current_test->fail.line = line;
	g_current_test->fail.expr = expr;

	ufbxwt_longjmp(g_test_jmp, 1, file, line, expr);
}

void ufbxwt_logf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if (g_log_pos < sizeof(g_log_buf)) {
		g_log_pos += vsnprintf(g_log_buf + g_log_pos,
			sizeof(g_log_buf) - g_log_pos, fmt, args);
		if (g_log_pos < sizeof(g_log_buf)) {
			g_log_buf[g_log_pos] = '\n';
			g_log_pos++;
		}
	}
	va_end(args);
}

void ufbxwt_hintf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(g_hint, sizeof(g_hint), fmt, args);
	va_end(args);
}

void ufbxwt_log_flush(bool print_always)
{
	if ((g_verbose || print_always) && g_log_pos > 0) {
		int prev_newline = 1;
		for (uint32_t i = 0; i < g_log_pos; i++) {
			if (i >= sizeof(g_log_buf)) break;
			char ch = g_log_buf[i];
			if (ch == '\n') {
				putchar('\n');
				prev_newline = 1;
			} else {
				if (prev_newline) {
					putchar(' ');
					putchar(' ');
				}
				prev_newline = 0;
				putchar(ch);
			}
		}
	}
	g_log_pos = 0;
}

void ufbxwt_log_error(ufbxw_error *err)
{
	// TODO
}

int ufbxwt_run_test(ufbxwt_test *test)
{
	printf("%s: ", test->name);
	fflush(stdout);

	g_error.failed = false;
	g_hint[0] = '\0';

	g_current_test = test;
	if (!ufbxwt_setjmp(g_test_jmp)) {
		test->func();
		printf("OK\n");
		fflush(stdout);
		return 1;
	} else {
		printf("FAIL\n");
		fflush(stdout);

		if (g_hint[0]) {
			printf("Hint: %s\n", g_hint);
		}
		if (g_error.failed) {
			ufbxwt_log_error(&g_error);
		}

		return 0;
	}
}

int main(int argc, char **argv)
{
	uint32_t num_tests = ufbxwt_arraycount(g_tests);
	uint32_t num_ok = 0;
	const char *test_filter = NULL;
	const char *test_group = NULL;

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
			g_verbose = 1;
		}
		if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--test")) {
			if (++i < argc) {
				test_filter = argv[i];
			}
		}
		if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--data")) {
			if (++i < argc) {
				size_t len = strlen(argv[i]);
				if (len + 2 > sizeof(data_root)) {
					fprintf(stderr, "-d: Data root too long");
					return 1;
				}
				memcpy(data_root, argv[i], len);
				char end = argv[i][len - 1];
				if (end != '/' && end != '\\') {
					data_root[len] = '/';
					data_root[len + 1] = '\0';
				}
			}
		}
		if (!strcmp(argv[i], "-g") || !strcmp(argv[i], "--group")) {
			if (++i < argc) {
				test_group = argv[i];
			}
		}
	}

	uint32_t num_ran = 0;
	for (uint32_t i = 0; i < num_tests; i++) {
		ufbxwt_test *test = &g_tests[i];
		ufbxwt_test_stats *group_stats = ufbxwt_get_test_group(test->group);
		group_stats->num_total++;

		if (test_filter && strcmp(test->name, test_filter)) {
			continue;
		}
		if (test_group && strcmp(test->group, test_group)) {
			continue;
		}

		group_stats->num_ran++;
		num_ran++;
		bool print_always = false;
		if (ufbxwt_run_test(test)) {
			num_ok++;
			group_stats->num_ok++;
		} else {
			print_always = true;
		}

		ufbxwt_log_flush(print_always);
	}

	if (num_ok < num_tests) {
		printf("\n");
		for (uint32_t i = 0; i < num_tests; i++) {
			ufbxwt_test *test = &g_tests[i];
			if (test->fail.failed) {
				ufbxwt_fail *fail = &test->fail;
				const char *file = fail->file, *find;
				find = strrchr(file, '/');
				file = find ? find + 1 : file;
				find = strrchr(file, '\\');
				file = find ? find + 1 : file;
				printf("(%s) %s:%u: %s\n", test->name,
					file, fail->line, fail->expr);
			}
		}
	}

	printf("\nTests passed: %u/%u\n", num_ok, num_ran);

	if (g_verbose) {
		size_t num_skipped = 0;
		for (size_t i = 0; i < g_num_groups; i++) {
			ufbxwt_test_stats *group = &g_test_groups[i];
			if (group->num_ran == 0) {
				num_skipped++;
				continue;
			}
			printf("  %s: %u/%u\n", group->name, group->num_ok, group->num_ran);
		}
		if (num_skipped > 0) {
			printf("  .. skipped %zu groups\n", num_skipped);
		}
	}

	return num_ok == num_ran ? 0 : 1;
}


