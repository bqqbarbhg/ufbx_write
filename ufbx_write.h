#ifndef UFBXW_UFBX_WRITE_H_INCLUDED
#define UFBXW_UFBX_WRITE_H_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef ufbxw_abi
#define ufbxw_abi
#endif

#ifndef ufbxw_abi
	#if defined(UFBXW_STATIC)
		#define ufbxw_abi static
	#else
		#define ufbxw_abi
	#endif
#endif

#ifndef ufbxw_abi_data
	#if defined(UFBXW_STATIC)
		#define ufbxw_abi_data static
	#else
		#define ufbxw_abi_data extern
	#endif
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

typedef struct ufbxw_vec4 {
	ufbxw_real x, y, z, w;
} ufbxw_vec4;

typedef struct ufbxw_bool3 {
	bool x, y, z;
} ufbxw_bool3;

typedef struct ufbxw_quat {
	ufbxw_real x, y, z, w;
} ufbxw_quat;

ufbxw_abi_data const ufbxw_string ufbxw_empty_string;
ufbxw_abi_data const ufbxw_vec2 ufbxw_zero_vec2;
ufbxw_abi_data const ufbxw_vec3 ufbxw_zero_vec3;
ufbxw_abi_data const ufbxw_vec4 ufbxw_zero_vec4;

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

	UFBXW_ELEMENT_TEMPLATE,
	UFBXW_ELEMENT_SCENE_INFO,
	UFBXW_ELEMENT_DOCUMENT,

	UFBXW_ELEMENT_TYPE_COUNT,
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

	// Generic
	UFBXW_P_Color,

	// Node properties
	UFBXW_P_Lcl_Translation,
	UFBXW_P_Lcl_Rotation,
	UFBXW_P_Lcl_Scaling,
	UFBXW_P_RotationOrder,
	UFBXW_P_PreRotation,
	UFBXW_P_PostRotation,
	UFBXW_P_RotationOffset,
	UFBXW_P_RotationPivot,
	UFBXW_P_ScalingOffset,
	UFBXW_P_ScalingPivot,
	UFBXW_P_QuaternionInterpolate,
	UFBXW_P_GeometricTranslation,
	UFBXW_P_GeometricRotation,
	UFBXW_P_GeometricScaling,
	UFBXW_P_TranslationActive,
	UFBXW_P_TranslationMin,
	UFBXW_P_TranslationMax,
	UFBXW_P_TranslationMinX,
	UFBXW_P_TranslationMinY,
	UFBXW_P_TranslationMinZ,
	UFBXW_P_TranslationMaxX,
	UFBXW_P_TranslationMaxY,
	UFBXW_P_TranslationMaxZ,
	UFBXW_P_RotationSpaceForLimitOnly,
	UFBXW_P_RotationStiffnessX,
	UFBXW_P_RotationStiffnessY,
	UFBXW_P_RotationStiffnessZ,
	UFBXW_P_AxisLen,
	UFBXW_P_RotationActive,
	UFBXW_P_RotationMin,
	UFBXW_P_RotationMax,
	UFBXW_P_RotationMinX,
	UFBXW_P_RotationMinY,
	UFBXW_P_RotationMinZ,
	UFBXW_P_RotationMaxX,
	UFBXW_P_RotationMaxY,
	UFBXW_P_RotationMaxZ,
	UFBXW_P_InheritType,
	UFBXW_P_ScalingActive,
	UFBXW_P_ScalingMin,
	UFBXW_P_ScalingMax,
	UFBXW_P_ScalingMinX,
	UFBXW_P_ScalingMinY,
	UFBXW_P_ScalingMinZ,
	UFBXW_P_ScalingMaxX,
	UFBXW_P_ScalingMaxY,
	UFBXW_P_ScalingMaxZ,
	UFBXW_P_MinDampRangeX,
	UFBXW_P_MinDampRangeY,
	UFBXW_P_MinDampRangeZ,
	UFBXW_P_MaxDampRangeX,
	UFBXW_P_MaxDampRangeY,
	UFBXW_P_MaxDampRangeZ,
	UFBXW_P_MinDampStrengthX,
	UFBXW_P_MinDampStrengthY,
	UFBXW_P_MinDampStrengthZ,
	UFBXW_P_MaxDampStrengthX,
	UFBXW_P_MaxDampStrengthY,
	UFBXW_P_MaxDampStrengthZ,
	UFBXW_P_PreferedAngleX,
	UFBXW_P_PreferedAngleY,
	UFBXW_P_PreferedAngleZ,
	UFBXW_P_LookAtProperty,
	UFBXW_P_UpVectorProperty,
	UFBXW_P_Show,
	UFBXW_P_NegativePercentShapeSupport,
	UFBXW_P_DefaultAttributeIndex,
	UFBXW_P_Freeze,
	UFBXW_P_LODBox,
	UFBXW_P_Visibility,
	UFBXW_P_Visibility_Inheritance,

	// Geometry
	UFBXW_P_BBoxMin,
	UFBXW_P_BBoxMax,
	UFBXW_P_Primary_Visibility,
	UFBXW_P_Casts_Shadows,
	UFBXW_P_Receive_Shadows,

	// Animation layer
	UFBXW_P_Weight,
	UFBXW_P_Mute,
	UFBXW_P_Solo,
	UFBXW_P_Lock,
	UFBXW_P_BlendMode,
	UFBXW_P_RotationAccumulationMode,
	UFBXW_P_ScaleAccumulationMode,
	UFBXW_P_BlendModeBypass,

	// Animation stack
	UFBXW_P_Description,
	UFBXW_P_LocalStart,
	UFBXW_P_LocalStop,
	UFBXW_P_ReferenceStart,
	UFBXW_P_ReferenceStop,

	// Material
	UFBXW_P_ShadingModel,
	UFBXW_P_MultiLayer,
	UFBXW_P_EmissiveColor,
	UFBXW_P_EmissiveFactor,
	UFBXW_P_AmbientColor,
	UFBXW_P_AmbientFactor,
	UFBXW_P_DiffuseColor,
	UFBXW_P_DiffuseFactor,
	UFBXW_P_Bump,
	UFBXW_P_NormalMap,
	UFBXW_P_BumpFactor,
	UFBXW_P_TransparentColor,
	UFBXW_P_TransparencyFactor,
	UFBXW_P_DisplacementColor,
	UFBXW_P_DisplacementFactor,
	UFBXW_P_VectorDisplacementColor,
	UFBXW_P_VectorDisplacementFactor,
	UFBXW_P_SpecularColor,
	UFBXW_P_SpecularFactor,
	UFBXW_P_ShininessExponent,
	UFBXW_P_ReflectionColor,
	UFBXW_P_ReflectionFactor,

	// Camera
	UFBXW_P_Position,
	UFBXW_P_InterestPosition,
	UFBXW_P_BackgroundColor,
	UFBXW_P_AspectWidth,
	UFBXW_P_AspectHeight,
	UFBXW_P_FilmWidth,
	UFBXW_P_FilmHeight,
	UFBXW_P_FilmAspectRatio,
	UFBXW_P_ApertureMode,
	UFBXW_P_GateFit,
	UFBXW_P_FocalLength,
	UFBXW_P_NearPlane,
	UFBXW_P_FarPlane,
	UFBXW_P_AutoComputeClipPanes,
	UFBXW_P_FocusDistance,

	UFBXW_PROP_FIRST_USER,
	UFBXW_PROP_FORCE_32BIT = 0x7fffffff,
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

ufbxw_abi void ufbxw_connect(ufbxw_scene *scene, ufbxw_id src, ufbxw_id dst);
ufbxw_abi void ufbxw_connect_multi(ufbxw_scene *scene, ufbxw_id src, ufbxw_id dst);
ufbxw_abi void ufbxw_disconnect(ufbxw_scene *scene, ufbxw_id src, ufbxw_id dst);
ufbxw_abi void ufbxw_disconnect_dst(ufbxw_scene *scene, ufbxw_id id, ufbxw_element_type type);
ufbxw_abi void ufbxw_disconnect_src(ufbxw_scene *scene, ufbxw_id id, ufbxw_element_type type);

ufbxw_abi void ufbxw_set_real(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop, ufbxw_real value);
ufbxw_abi void ufbxw_set_bool(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop, bool value);
ufbxw_abi void ufbxw_set_int(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop, int64_t value);
ufbxw_abi void ufbxw_set_vec3(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop, ufbxw_vec3 value);
ufbxw_abi void ufbxw_set_vec4(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop, ufbxw_vec4 value);
ufbxw_abi void ufbxw_set_string_len(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop, const char *value, size_t value_len);
ufbxw_abi void ufbxw_set_string(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop, const char *value);

ufbxw_abi ufbxw_real ufbxw_get_real(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop);
ufbxw_abi bool ufbxw_get_bool(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop);
ufbxw_abi int64_t ufbxw_get_int(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop);
ufbxw_abi ufbxw_vec3 ufbxw_get_vec3(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop);
ufbxw_abi ufbxw_vec4 ufbxw_get_vec4(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop);
ufbxw_abi ufbxw_string ufbxw_get_string(ufbxw_scene *scene, ufbxw_id id, ufbxw_prop prop);

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

ufbxw_abi void ufbxw_node_set_translation(ufbxw_scene *scene, ufbxw_node node, ufbxw_vec3 translation);
ufbxw_abi void ufbxw_node_set_rotation(ufbxw_scene *scene, ufbxw_node node, ufbxw_vec3 rotation);
ufbxw_abi void ufbxw_node_set_scaling(ufbxw_scene *scene, ufbxw_node node, ufbxw_vec3 scaling);

ufbxw_abi ufbxw_vec3 ufbxw_node_get_translation(ufbxw_scene *scene, ufbxw_node node);
ufbxw_abi ufbxw_vec3 ufbxw_node_get_rotation(ufbxw_scene *scene, ufbxw_node node);
ufbxw_abi ufbxw_vec3 ufbxw_node_get_scaling(ufbxw_scene *scene, ufbxw_node node);

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

