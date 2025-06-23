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

	UFBXW_ELEMENT_SCENE_INFO,
	UFBXW_ELEMENT_DOCUMENT,
} ufbxw_element_type;

typedef uint64_t ufbxw_id;
typedef struct ufbxw_node { ufbxw_id id; } ufbxw_node;
typedef struct ufbxw_mesh { ufbxw_id id; } ufbxw_mesh;

#define ufbxw_null_id ((ufbxw_id)0)
#define ufbxw_null_node ((ufbxw_node){0})
#define ufbxw_null_mesh ((ufbxw_mesh){0})

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

	bool no_default_elements;
	bool no_default_scene_info;
	bool no_default_document;

	uint32_t _end_zero;
} ufbxw_scene_opts;

typedef enum ufbxw_prop {
	UFBXW_PROP_NONE,

	UFBXW_P_Lcl_Translation,
	UFBXW_P_Lcl_Rotation,
	UFBXW_P_Lcl_Scaling,

	UFBXW_PROP_FIRST_USER,
} ufbxw_prop;

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

ufbxw_abi void ufbxw_set_string(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop, const char *value);
ufbxw_abi void ufbxw_set_string_len(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop, const char *value, size_t value_len);

ufbxw_abi void ufbxw_connect(ufbxw_scene *scene, ufbxw_id src, ufbxw_id dst);
ufbxw_abi void ufbxw_connect_multi(ufbxw_scene *scene, ufbxw_id src, ufbxw_id dst);
ufbxw_abi void ufbxw_disconnect(ufbxw_scene *scene, ufbxw_id src, ufbxw_id dst);
ufbxw_abi void ufbxw_disconnect_dst(ufbxw_scene *scene, ufbxw_id id, ufbxw_element_type type);
ufbxw_abi void ufbxw_disconnect_src(ufbxw_scene *scene, ufbxw_id id, ufbxw_element_type type);

typedef struct ufbxw_custom_prop {
	ufbxw_string name;
	ufbxw_string type;
	ufbxw_string sub_type;
} ufbxw_custom_prop;

ufbxw_abi ufbxw_prop ufbxw_get_custom_prop(ufbxw_scene *scene, const ufbxw_custom_prop *prop);
ufbxw_abi ufbxw_prop ufbxw_get_custom_prop_c(ufbxw_scene *scene, const char *name, const char *type, const char *sub_type);

// -- Node

ufbxw_abi ufbxw_node ufbxw_create_node(ufbxw_scene *scene);
ufbxw_abi ufbxw_node ufbxw_as_node(ufbxw_id id);

ufbxw_abi void ufbxw_node_set_parent(ufbxw_scene *scene, ufbxw_node node, ufbxw_node parent);
ufbxw_abi ufbxw_node ufbxw_node_get_parent(ufbxw_scene *scene, ufbxw_node node);
ufbxw_abi size_t ufbxw_node_get_num_children(ufbxw_scene *scene, ufbxw_node node);
ufbxw_abi ufbxw_node ufbxw_node_get_child(ufbxw_scene *scene, ufbxw_node node, size_t index);

// -- Mesh

ufbxw_abi ufbxw_mesh ufbxw_create_mesh(ufbxw_scene *scene);
ufbxw_abi ufbxw_mesh ufbxw_as_mesh(ufbxw_id id);

ufbxw_abi void ufbxw_mesh_add_instance(ufbxw_scene *scene, ufbxw_mesh mesh, ufbxw_node node);

// -- Scene info

ufbxw_abi ufbxw_id ufbxw_get_scene_info_id(ufbxw_scene *scene);

// -- IO callbacks

// Write at a specified offset in the file
typedef bool ufbxw_write_fn(void *user, uint64_t offset, const void *data, size_t size);

// Close the file
typedef void ufbxw_close_fn(void *user);

typedef struct ufbxw_write_stream {
	ufbxw_write_fn *write_fn; // < Required
	ufbxw_close_fn *close_fn; // < Optional

	// Context passed to other functions
	void *user;
} ufbxw_write_stream;

ufbxw_abi bool ufbxw_open_file_write(ufbxw_write_stream *stream, const char *path, size_t path_len);

// -- Writing API

typedef struct ufbxw_save_opts {
	uint32_t _begin_zero;

	bool ascii;
	uint32_t version;

	uint32_t _end_zero;
} ufbxw_save_opts;

ufbxw_abi bool ufbxw_save_file(ufbxw_scene *scene, const char *path, const ufbxw_save_opts *opts, ufbxw_error *error);
ufbxw_abi bool ufbxw_save_file_len(ufbxw_scene *scene, const char *path, size_t path_len, const ufbxw_save_opts *opts, ufbxw_error *error);

ufbxw_abi bool ufbxw_save_stream(ufbxw_scene *scene, ufbxw_write_stream *stream, const ufbxw_save_opts *opts, ufbxw_error *error);

#endif

