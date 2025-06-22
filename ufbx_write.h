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

typedef struct ufbxw_node {
	ufbxw_id id;
} ufbxw_node;

typedef struct ufbxw_mesh {
	ufbxw_id id;
} ufbxw_mesh;

// -- Memory callbacks

typedef void *ufbxw_alloc_fn(void *user, size_t size);
typedef void *ufbxw_realloc_fn(void *user, void *old_ptr, size_t old_size, size_t new_size);
typedef void ufbxw_free_fn(void *user, void *ptr, size_t size);
typedef void ufbxw_free_allocator_fn(void *user);

typedef struct ufbxw_allocator {
	ufbxw_alloc_fn *alloc_fn;
	ufbxw_realloc_fn *realloc_fn;
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

// -- Writing API

typedef struct ufbxw_save_opts {
	uint32_t _begin_zero;

	bool ascii;

	uint32_t _end_zero;
} ufbxw_save_opts;

ufbxw_abi bool ufbxw_save_file(ufbxw_scene *scene, const char *filename, const ufbxw_save_opts *opts, ufbxw_error *error);
ufbxw_abi bool ufbxw_save_file_len(ufbxw_scene *scene, const char *filename, size_t filename_len, const ufbxw_save_opts *opts, ufbxw_error *error);

#if 0

typedef struct ufbxw_writer ufbxw_writer;

typedef uint32_t ufbxw_id;

typedef struct ufbxw_node {
	ufbxw_id id;
} ufbxw_node;

typedef struct ufbxw_mesh {
	ufbxw_id id;
} ufbxw_mesh;

typedef struct ufbxw_vec2 {
	double x, y;
} ufbxw_vec2;

typedef struct ufbxw_vec3 {
	double x, y, z;
} ufbxw_vec3;

typedef struct ufbxw_quat {
	double x, y, z, w;
} ufbxw_quat;

// Order in which Euler-angle rotation axes are applied for a transform
// NOTE: The order in the name refers to the order of axes *applied*,
// not the multiplication order: eg. `UFBX_ROTATION_ORDER_XYZ` is `Z*Y*X`
typedef enum ufbxw_rotation_order {
	UFBXW_ROTATION_ORDER_XYZ,
	UFBXW_ROTATION_ORDER_XZY,
	UFBXW_ROTATION_ORDER_YZX,
	UFBXW_ROTATION_ORDER_YXZ,
	UFBXW_ROTATION_ORDER_ZXY,
	UFBXW_ROTATION_ORDER_ZYX,
} ufbxw_rotation_order;

typedef enum ufbxw_element_type {
	UFBXW_ELEMENT_UNKNOWN,
	UFBXW_ELEMENT_NODE,
	UFBXW_ELEMENT_MESH,
} ufbxw_element_type;

typedef enum ufbxw_prop {

	UFBXW_ELEMENT_NAME,

	UFBXW_NODE_PARENT,

	// FBX Properties
	UFBXW_P_Lcl_Translation,
	UFBXW_P_Lcl_Rotation,
	UFBXW_P_Lcl_Scaling,
	UFBXW_P_RotationOrder,

} ufbxw_prop;

// -- Writer

ufbxw_abi ufbxw_writer *ufbxw_create_writer();
ufbxw_abi void ufbxw_free_writer(ufbxw_writer *w);

typedef struct ufbxw_string {
	const char *data;
	size_t length;
} ufbxw_string;

typedef struct ufbxw_error {
	ufbxw_string message;
	ufbxw_string function;
} ufbxw_error;

typedef void ufbxw_error_fn(void *user, const ufbxw_error *error);

void ufbxw_set_error_callback(ufbxw_writer *w, ufbxw_error_fn *fn, void *user);

// -- Data

typedef struct ufbxw_int_buffer ufbxw_int_buffer;
typedef struct ufbxw_vec2_buffer ufbxw_vec2_buffer;
typedef struct ufbxw_vec3_buffer ufbxw_vec3_buffer;

typedef bool ufbxw_buffer_int_fn(void *user, int32_t *dst, size_t offset, size_t count);
typedef bool ufbxw_buffer_vec2_fn(void *user, int32_t *dst, size_t offset, size_t count);
typedef bool ufbxw_buffer_vec3_fn(void *user, int32_t *dst, size_t offset, size_t count);

struct ufbxw_int_buffer {
	const int32_t *data;
	size_t count;
	ufbxw_buffer_int_fn *fn;
	void *user;
};

struct ufbxw_vec2_buffer {
	const ufbxw_vec2 *data;
	size_t count;
	ufbxw_buffer_vec2_fn *fn;
	void *user;
};

struct ufbxw_vec3_buffer {
	const ufbxw_vec3 *data;
	size_t count;
	ufbxw_buffer_vec3_fn *fn;
	void *user;
};

ufbxw_abi ufbxw_int_buffer ufbxw_int_array(const int32_t *data, size_t count);
ufbxw_abi ufbxw_int_buffer ufbxw_int_func(ufbxw_buffer_int_fn *fn, void *user);

ufbxw_abi ufbxw_vec2_buffer ufbxw_vec2_array(const ufbxw_vec2 *data, size_t count);
ufbxw_abi ufbxw_vec2_buffer ufbxw_vec2_func(ufbxw_buffer_vec2_fn *fn, void *user);

ufbxw_abi ufbxw_vec3_buffer ufbxw_vec3_array(const ufbxw_vec3 *data, size_t count);
ufbxw_abi ufbxw_vec3_buffer ufbxw_vec3_func(ufbxw_buffer_vec3_fn *fn, void *user);

// -- Low-level API

ufbxw_abi ufbxw_id ufbxw_create_element(ufbxw_writer *w, ufbxw_element_type type);

ufbxw_abi void ufbxw_set_name_len(ufbxw_writer *w, ufbxw_id id, const char *name, size_t name_len);
ufbxw_abi void ufbxw_set_name(ufbxw_writer *w, ufbxw_id id, const char *name);
ufbxw_abi void ufbxw_connect(ufbxw_writer *w, ufbxw_id src, ufbxw_id dst);

ufbxw_abi void ufbxw_set_string_len(ufbxw_writer *w, ufbxw_id id, ufbxw_prop prop, const char *value, size_t value_len);
ufbxw_abi void ufbxw_set_string(ufbxw_writer *w, ufbxw_id id, ufbxw_prop prop, const char *value);
ufbxw_abi void ufbxw_set_int(ufbxw_writer *w, ufbxw_id id, ufbxw_prop prop, int64_t value);
ufbxw_abi void ufbxw_set_vec3(ufbxw_writer *w, ufbxw_id id, ufbxw_prop prop, ufbxw_vec3 value);

// -- Node

ufbxw_abi ufbxw_node ufbxw_create_node(ufbxw_writer *w);

ufbxw_abi void ufbxw_node_set_name_len(ufbxw_writer *w, ufbxw_node node, const char *name, size_t name_len);
ufbxw_abi void ufbxw_node_set_name(ufbxw_writer *w, ufbxw_node node, const char *name);
ufbxw_abi void ufbxw_node_set_parent(ufbxw_writer *w, ufbxw_node node, ufbxw_node parent);

ufbxw_abi void ufbxw_node_set_translation(ufbxw_writer *w, ufbxw_node node, ufbxw_vec3 value);
ufbxw_abi void ufbxw_node_set_rotation_euler(ufbxw_writer *w, ufbxw_node node, ufbxw_vec3 value);
ufbxw_abi void ufbxw_node_set_rotation_euler_order(ufbxw_writer *w, ufbxw_node node, ufbxw_vec3 value, ufbxw_rotation_order order);
ufbxw_abi void ufbxw_node_set_rotation_quat(ufbxw_writer *w, ufbxw_node node, ufbxw_quat quat);
ufbxw_abi void ufbxw_node_set_rotation_quat_order(ufbxw_writer *w, ufbxw_node node, ufbxw_quat quat, ufbxw_rotation_order order);
ufbxw_abi void ufbxw_node_set_scale(ufbxw_writer *w, ufbxw_node node, ufbxw_vec3 value);

// -- Mesh

typedef enum ufbxw_mesh_layer_element {
	UFBXW_MESH_LAYER_ELEMENT_POSITION,
	UFBXW_MESH_LAYER_ELEMENT_NORMAL,
	UFBXW_MESH_LAYER_ELEMENT_BINORMAL,
	UFBXW_MESH_LAYER_ELEMENT_UV,
	UFBXW_MESH_LAYER_ELEMENT_COLOR,
	UFBXW_MESH_LAYER_ELEMENT_SMOOTHING,
	UFBXW_MESH_LAYER_ELEMENT_VERTEX_CREASE,
	UFBXW_MESH_LAYER_ELEMENT_EDGE_CREASE,
	UFBXW_MESH_LAYER_ELEMENT_MATERIAL,
} ufbxw_mesh_layer_element;

typedef struct ufbxw_mesh_layer {

} ufbxw_mesh_layer;

ufbxw_abi ufbxw_mesh ufbxw_create_mesh(ufbxw_writer *w);

ufbxw_abi void ufbxw_mesh_add(ufbxw_writer *w, ufbxw_mesh mesh, ufbxw_node node);

ufbxw_abi void ufbxw_mesh_set_indices_triangles(ufbxw_writer *w, ufbxw_mesh mesh, ufbxw_int_buffer indices);
ufbxw_abi void ufbxw_mesh_set_indices(ufbxw_writer *w, ufbxw_mesh mesh, ufbxw_int_buffer indices, ufbxw_int_buffer face_sizes);

// -- Animation



// Utility

ufbxw_abi ufbxw_vec3 ufbxw_quat_to_euler(ufbxw_quat q, ufbxw_rotation_order order);

#endif

#endif

