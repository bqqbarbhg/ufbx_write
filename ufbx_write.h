#ifndef UFBXW_UFBX_WRITE_H_INCLUDED
#define UFBXW_UFBX_WRITE_H_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef ufbxw_abi
#define ufbxw_abi
#endif

#ifndef ufbxw_assert
	#if defined(UFBXW_NO_ASSERT) || defined(UFBXW_NO_LIBC)
		#define ufbxw_assert(cond) (void)0
	#else
		#include <assert.h>
		#define ufbxw_assert(cond) assert(cond)
	#endif
#endif

#if defined(__cplusplus)
	#define UFBXW_LIST_TYPE(p_name, p_type) struct p_name { p_type *data; size_t count; \
		p_type &operator[](size_t index) const { ufbx_assert(index < count); return data[index]; } \
		p_type *begin() const { return data; } \
		p_type *end() const { return data + count; } \
	}
#else
	#define UFBXW_LIST_TYPE(p_name, p_type) typedef struct p_name { p_type *data; size_t count; } p_name
#endif

// UTF-8
typedef struct ufbxw_string {
	const char *data;
	size_t length;
} ufbxw_string;

typedef double ufbxw_real;

typedef struct ufbxw_vec2 {
	ufbxw_real x, y;
} ufbxw_vec2;

typedef struct ufbxw_vec3 {
	ufbxw_real x, y, z;
} ufbxw_vec3;

typedef struct ufbxw_bool3 {
	bool x, y, z;
} ufbxw_bool3;

typedef struct ufbxw_quat {
	ufbxw_real x, y, z, w;
} ufbxw_quat;

typedef enum ufbxw_rotation_order {
	UFBXW_ROTATION_ORDER_XYZ,
	UFBXW_ROTATION_ORDER_XZY,
	UFBXW_ROTATION_ORDER_YZX,
	UFBXW_ROTATION_ORDER_YXZ,
	UFBXW_ROTATION_ORDER_ZXY,
	UFBXW_ROTATION_ORDER_ZYX,
	UFBXW_ROTATION_ORDER_SPHERIC,
} ufbxw_rotation_order;

typedef struct ufbxw_scene ufbxw_scene;

typedef enum ufbxw_element_type {
	UFBXW_ELEMENT_CUSTOM = 1,
	UFBXW_ELEMENT_NODE,

	UFBXW_ELEMENT_CUSTOM_NODE_ATTRIBUTE,
	UFBXW_ELEMENT_MESH,
} ufbxw_element_type;

typedef uint64_t ufbxw_id;
typedef struct ufbxw_node { ufbxw_id id; } ufbxw_node;
typedef struct ufbxw_mesh { ufbxw_id id; } ufbxw_mesh;

// -- Memory callbacks

typedef void *ufbxw_alloc_fn(void *user, size_t size);
typedef void ufbxw_free_fn(void *user, void *ptr, size_t size);
typedef void ufbxw_free_allocator_fn(void *user);

typedef struct ufbxw_allocator {
	ufbxw_alloc_fn *alloc_fn;
	ufbxw_free_fn *free_fn;
	ufbxw_free_allocator_fn *free_allocator_fn;
	void *user;
} ufbxw_allocator;

// -- Error

typedef struct ufbxw_error {
	bool failed;
} ufbxw_error;

// -- Scene API

typedef struct ufbxw_scene_opts {
	uint32_t _begin_zero;

	ufbxw_allocator allocator;

	uint32_t _end_zero;
} ufbxw_scene_opts;

ufbxw_abi ufbxw_scene *ufbxw_create_scene(const ufbxw_scene_opts *opts);
ufbxw_abi void ufbxw_free_scene(ufbxw_scene *scene);

ufbxw_abi bool ufbxw_get_error(ufbxw_scene *scene, ufbxw_error *error);

ufbxw_abi ufbxw_id ufbxw_create_element(ufbxw_scene *scene, ufbxw_element_type type);
ufbxw_abi void ufbxw_delete_element(ufbxw_scene *scene, ufbxw_id id);
ufbxw_abi size_t ufbxw_get_num_elements(ufbxw_scene *scene);
ufbxw_abi size_t ufbxw_get_elements(ufbxw_scene *scene, ufbxw_id *elements, size_t num_elements);

ufbxw_abi void ufbxw_set_name(ufbxw_scene *scene, ufbxw_id id, const char *name);
ufbxw_abi void ufbxw_set_name_len(ufbxw_scene *scene, ufbxw_id id, const char *name, size_t name_len);
ufbxw_abi ufbxw_string ufbxw_get_name(ufbxw_scene *scene, ufbxw_id id);

typedef enum ufbxw_prop {
	UFBXW_P_Lcl_Translation,
	UFBXW_P_Lcl_Rotation,
	UFBXW_P_Lcl_Scaling,
} ufbxw_prop;

// -- Node

ufbxw_abi ufbxw_node ufbxw_create_node(ufbxw_scene *scene);
ufbxw_abi ufbxw_node ufbxw_as_node(ufbxw_id id);

// -- Mesh

ufbxw_abi ufbxw_mesh ufbxw_create_mesh(ufbxw_scene *scene);
ufbxw_abi ufbxw_mesh ufbxw_as_mesh(ufbxw_id id);

// -- Writing API

typedef struct ufbxw_save_opts {
	uint32_t _begin_zero;

	bool ascii;

	uint32_t _end_zero;
} ufbxw_save_opts;

ufbxw_abi bool ufbxw_save_file(ufbxw_scene *scene, const char *filename, const ufbxw_save_opts *opts, ufbxw_error *error);
ufbxw_abi bool ufbxw_save_file_len(ufbxw_scene *scene, const char *filename, size_t filename_len, const ufbxw_save_opts *opts, ufbxw_error *error);

#endif

