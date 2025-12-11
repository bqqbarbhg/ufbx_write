#pragma once

#include <stdbool.h>

typedef struct compare_fbx_opts {
	double approx_epsilon;
	bool compare_anim;
	bool evaluate_scene_anim;
} compare_fbx_opts;

#if defined(__cplusplus)
extern "C" {
#endif

bool compare_fbx(const char *src_path, const char *ref_path, const compare_fbx_opts *opts);

#if defined(__cplusplus)
}
#endif