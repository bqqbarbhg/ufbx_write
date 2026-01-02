#include "../../ufbx_write.h"
#include "../ufbx/ufbx.h"
#include "../util/ufbxwt_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../compare_fbx/compare_fbx.h"

#define ufbxwt_array_count(arr) (sizeof(arr) / sizeof(*(arr)))
#define ufbxwt_static_assert(desc, cond) typedef char ufbxwt_static_assert_##desc[(cond)?1:-1]

static void ufbxwt_assert_fail(const char *file, uint32_t line, const char *expr) {
	fprintf(stderr, "assert fail: %s (%s:%u)\n", expr, file, line);
	exit(1);
}

#define ufbxwt_assert(cond) do { \
		if (!(cond)) ufbxwt_assert_fail(__FILE__, __LINE__, #cond); \
	} while (0)

static const char *ufbxwt_ufbx_element_type_names[] = {
	"unknown",
	"node",
	"mesh",
	"light",
	"camera",
	"bone",
	"empty",
	"line_curve",
	"nurbs_curve",
	"nurbs_surface",
	"nurbs_trim_surface",
	"nurbs_trim_boundary",
	"procedural_geometry",
	"stereo_camera",
	"camera_switcher",
	"marker",
	"lod_group",
	"skin_deformer",
	"skin_cluster",
	"blend_deformer",
	"blend_channel",
	"blend_shape",
	"cache_deformer",
	"cache_file",
	"material",
	"texture",
	"video",
	"shader",
	"shader_binding",
	"anim_stack",
	"anim_layer",
	"anim_value",
	"anim_curve",
	"display_layer",
	"selection_set",
	"selection_node",
	"character",
	"constraint",
	"audio_layer",
	"audio_clip",
	"pose",
	"metadata_object",
};

ufbxwt_static_assert(element_type_name_count, ufbxwt_array_count(ufbxwt_ufbx_element_type_names) == UFBX_ELEMENT_TYPE_COUNT);

static ufbxw_vec2 to_ufbxw_vec2(ufbx_vec2 v)
{
	ufbxw_vec2 r = { v.x, v.y };
	return r;
}

static ufbxw_vec3 to_ufbxw_vec3(ufbx_vec3 v)
{
	ufbxw_vec3 r = { v.x, v.y, v.z };
	return r;
}

static ufbxw_vec4 to_ufbxw_vec4(ufbx_vec4 v)
{
	ufbxw_vec4 r = { v.x, v.y, v.z, v.w };
	return r;
}

static ufbxw_vec3 to_ufbxw_euler(ufbx_quat v)
{
	ufbx_vec3 euler = ufbx_quat_to_euler(v, UFBX_ROTATION_ORDER_XYZ);
	return to_ufbxw_vec3(euler);
}

static ufbxw_matrix to_ufbxw_matrix(ufbx_matrix v)
{
	ufbxw_matrix r;
	r.m00 = v.m00;
	r.m10 = v.m10;
	r.m20 = v.m20;
	r.m30 = 0.0f;
	r.m01 = v.m01;
	r.m11 = v.m11;
	r.m21 = v.m21;
	r.m31 = 0.0f;
	r.m02 = v.m02;
	r.m12 = v.m12;
	r.m22 = v.m22;
	r.m32 = 0.0f;
	r.m03 = v.m03;
	r.m13 = v.m13;
	r.m23 = v.m23;
	r.m33 = 1.0f;
	return r;
}

static const ufbx_vec3 one_vec3 = { 1.0f, 1.0f, 1.0f };

static ufbxw_int_buffer to_ufbxw_int_buffer(ufbxw_scene *scene, ufbx_int32_list src)
{
	ufbxw_int_buffer buffer = ufbxw_create_int_buffer(scene, src.count);
	ufbxw_int_list dst = ufbxw_edit_int_buffer(scene, buffer);
	memcpy(dst.data, src.data, src.count * sizeof(int32_t));
	return buffer;
}

static ufbxw_int_buffer to_ufbxw_uint_buffer(ufbxw_scene *scene, ufbx_uint32_list src)
{
	ufbxw_int_buffer buffer = ufbxw_create_int_buffer(scene, src.count);
	ufbxw_int_list dst = ufbxw_edit_int_buffer(scene, buffer);
	memcpy(dst.data, src.data, src.count * sizeof(int32_t));
	return buffer;
}

static ufbxw_real_buffer to_ufbxw_real_buffer(ufbxw_scene *scene, ufbx_real_list src)
{
	ufbxw_real_buffer buffer = ufbxw_create_real_buffer(scene, src.count);
	ufbxw_real_list dst = ufbxw_edit_real_buffer(scene, buffer);
	ufbxwt_assert(sizeof(ufbx_real) == sizeof(ufbxw_real));
	memcpy(dst.data, src.data, src.count * sizeof(ufbx_real));
	return buffer;
}

static ufbxw_vec2_buffer to_ufbxw_vec2_buffer(ufbxw_scene *scene, ufbx_vec2_list src)
{
	ufbxw_vec2_buffer buffer = ufbxw_create_vec2_buffer(scene, src.count);
	ufbxw_vec2_list dst = ufbxw_edit_vec2_buffer(scene, buffer);
	ufbxwt_assert(sizeof(ufbx_vec2) == sizeof(ufbxw_vec2));
	memcpy(dst.data, src.data, src.count * sizeof(ufbx_vec2));
	return buffer;
}

static ufbxw_vec3_buffer to_ufbxw_vec3_buffer(ufbxw_scene *scene, ufbx_vec3_list src)
{
	ufbxw_vec3_buffer buffer = ufbxw_create_vec3_buffer(scene, src.count);
	ufbxw_vec3_list dst = ufbxw_edit_vec3_buffer(scene, buffer);
	ufbxwt_assert(sizeof(ufbx_vec3) == sizeof(ufbxw_vec3));
	memcpy(dst.data, src.data, src.count * sizeof(ufbx_vec3));
	return buffer;
}

static ufbxw_vec4_buffer to_ufbxw_vec4_buffer(ufbxw_scene *scene, ufbx_vec4_list src)
{
	ufbxw_vec4_buffer buffer = ufbxw_create_vec4_buffer(scene, src.count);
	ufbxw_vec4_list dst = ufbxw_edit_vec4_buffer(scene, buffer);
	ufbxwt_assert(sizeof(ufbx_vec4) == sizeof(ufbxw_vec4));
	memcpy(dst.data, src.data, src.count * sizeof(ufbx_vec4));
	return buffer;
}

static ufbxw_vec3_buffer to_ufbxw_vec3_buffer_by_index(ufbxw_scene *scene, ufbx_vertex_vec3 src)
{
	ufbxw_vec3_buffer values = ufbxw_create_vec3_buffer(scene, src.indices.count);
	ufbxw_vec3_list data = ufbxw_edit_vec3_buffer(scene, values);
	for (size_t i = 0; i < src.indices.count; i++) {
		data.data[i] = to_ufbxw_vec3(ufbx_get_vertex_vec3(&src, i));
	}
	return values;
}

static ufbxw_inherit_type to_ufbxw_inherit_type(ufbx_inherit_mode mode)
{
	switch (mode) {
	case UFBX_INHERIT_MODE_NORMAL: return UFBXW_INHERIT_TYPE_NORMAL;
	case UFBX_INHERIT_MODE_COMPONENTWISE_SCALE: return UFBXW_INHERIT_TYPE_COMPONENTWISE_SCALE;
	case UFBX_INHERIT_MODE_IGNORE_PARENT_SCALE: return UFBXW_INHERIT_TYPE_IGNORE_PARENT_SCALE;
	default:
		ufbxwt_assert(0 && "unhandled inherit mode");
		return UFBXW_INHERIT_TYPE_NORMAL;
	}
}

static ufbx_element *find_deform_percent_element(ufbx_element *elem, const char *prop)
{
	for (size_t conn_ix = 0; conn_ix < elem->connections_src.count; conn_ix++) {
		ufbx_connection *conn = &elem->connections_src.data[conn_ix];
		if (conn->src_prop.length == 0 || conn->dst_prop.length == 0) continue;
		if (strcmp(conn->src_prop.data, prop) != 0) continue;
		if (strcmp(conn->dst_prop.data, "DeformPercent") != 0) continue;
		return conn->dst;
	}
	return NULL;
}

int main(int argc, char **argv)
{
	const char *output_path = NULL;
	const char *input_path = NULL;
	const char *format = "";
	bool compare = false;

	ufbxwt_deflate_impl deflate_impl = UFBXWT_DEFLATE_IMPL_BUILTIN;
	ufbxwt_ascii_format_impl ascii_impl = UFBXWT_ASCII_FORMAT_IMPL_DEFAULT;
	ufbxwt_thread_impl thread_impl = UFBXWT_THREAD_IMPL_NONE;

	bool advanced_transform = false;
	bool bake_animation = false;

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-f")) {
			if (++i < argc) {
				format = argv[i];
			}
		} else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) {
			if (++i < argc) {
				output_path = argv[i];
			}
		} else if (!strcmp(argv[i], "--deflate")) {
			if (++i < argc) {
				const char *name = argv[i];
				deflate_impl = UFBXWT_DEFLATE_IMPL_COUNT;
				for (int i = 0; i < UFBXWT_DEFLATE_IMPL_COUNT; i++) {
					if (!strcmp(ufbxwt_deflate_impl_name((ufbxwt_deflate_impl)i), name)) {
						deflate_impl = (ufbxwt_deflate_impl)i;
						break;
					}
				}
				ufbxwt_assert(deflate_impl != UFBXWT_DEFLATE_IMPL_COUNT);
			}
		} else if (!strcmp(argv[i], "--ascii")) {
			if (++i < argc) {
				const char *name = argv[i];
				ascii_impl = UFBXWT_ASCII_FORMAT_IMPL_COUNT;
				for (int i = 0; i < UFBXWT_ASCII_FORMAT_IMPL_COUNT; i++) {
					if (!strcmp(ufbxwt_ascii_format_name((ufbxwt_ascii_format_impl)i), name)) {
						ascii_impl = (ufbxwt_ascii_format_impl)i;
						break;
					}
				}
				ufbxwt_assert(ascii_impl != UFBXWT_ASCII_FORMAT_IMPL_COUNT);
			}
		} else if (!strcmp(argv[i], "--thread")) {
			if (++i < argc) {
				const char *name = argv[i];
				thread_impl = UFBXWT_THREAD_IMPL_COUNT;
				for (int i = 0; i < UFBXWT_THREAD_IMPL_COUNT; i++) {
					if (!strcmp(ufbxwt_thread_impl_name((ufbxwt_thread_impl)i), name)) {
						thread_impl = (ufbxwt_thread_impl)i;
						break;
					}
				}
				ufbxwt_assert(thread_impl != UFBXWT_THREAD_IMPL_COUNT);
			}
		} else if (!strcmp(argv[i], "--advanced-transform")) {
			advanced_transform = true;
		} else if (!strcmp(argv[i], "--bake-animation")) {
			bake_animation = true;
		} else if (!strcmp(argv[i], "--compare")) {
			compare = true;
		} else {
			ufbxwt_assert(input_path == NULL);
			input_path = argv[i];
		}
	}

	ufbxwt_assert(input_path != NULL);
	ufbxwt_assert(output_path != NULL);

	ufbx_load_opts load_opts = { 0 };

	ufbx_error load_error;
	ufbx_scene *in_scene = ufbx_load_file(input_path, &load_opts, &load_error);
	if (!in_scene) {
		char err_str[256];
		ufbx_format_error(err_str, sizeof(err_str), &load_error);
		fprintf(stderr, "failed to load: %s\n", err_str);
		exit(2);
	}

	ufbxw_scene_opts out_opts = { 0 };
	out_opts.no_default_anim_stack = true;
	out_opts.no_default_anim_layer = true;
	ufbxw_scene *out_scene = ufbxw_create_scene(&out_opts);

	// Coordinate settings
	{
		ufbxw_coordinate_axes axes;
		axes.right = (ufbxw_coordinate_axis)in_scene->settings.axes.right;
		axes.up = (ufbxw_coordinate_axis)in_scene->settings.axes.up;
		axes.front = (ufbxw_coordinate_axis)in_scene->settings.axes.front;

		// Get the original unit scale factor, without `/ 100.0 * 100.0` roundtrip
		ufbxw_real unit_scale = ufbx_find_real(&in_scene->settings.props, "UnitScaleFactor", 1.0);

		ufbxw_scene_set_coordinate_axes(out_scene, axes);
		ufbxw_scene_set_unit_scale_factor(out_scene, unit_scale);
	}

	// Time mode
	{
		ufbxw_time_mode time_mode = (ufbxw_time_mode)in_scene->settings.time_mode;
		ufbxw_real custom_frame_rate = ufbx_find_real(&in_scene->settings.props, "CustomFrameRate", -1.0);

		ufbxw_scene_set_time_mode(out_scene, time_mode);
		ufbxw_scene_set_custom_frame_rate(out_scene, custom_frame_rate);
	}

	ufbxw_mesh *mesh_ids = (ufbxw_mesh*)calloc(in_scene->meshes.count, sizeof(ufbxw_mesh));
	ufbxw_node *node_ids = (ufbxw_node*)calloc(in_scene->nodes.count, sizeof(ufbxw_node));
	ufbxw_anim_stack *anim_stack_ids = (ufbxw_anim_stack*)calloc(in_scene->anim_stacks.count, sizeof(ufbxw_anim_stack));
	ufbxw_anim_layer *anim_layer_ids = (ufbxw_anim_layer*)calloc(in_scene->anim_layers.count, sizeof(ufbxw_anim_layer));
	ufbxw_skin_deformer *skin_deformer_ids = (ufbxw_skin_deformer*)calloc(in_scene->skin_deformers.count, sizeof(ufbxw_skin_deformer));
	ufbxw_blend_deformer *blend_deformer_ids = (ufbxw_blend_deformer*)calloc(in_scene->blend_deformers.count, sizeof(ufbxw_blend_deformer));
	ufbxw_id *element_ids = (ufbxw_id*)calloc(in_scene->elements.count, sizeof(ufbxw_id));

	for (size_t mesh_ix = 0; mesh_ix < in_scene->meshes.count; mesh_ix++) {
		ufbx_mesh *in_mesh = in_scene->meshes.data[mesh_ix];
		ufbxw_mesh out_mesh = ufbxw_create_mesh(out_scene);
		mesh_ids[mesh_ix] = out_mesh;
		element_ids[in_mesh->element_id] = out_mesh.id;

		ufbxw_vec3_buffer vertices = to_ufbxw_vec3_buffer(out_scene, in_mesh->vertices);
		ufbxw_int_buffer vertex_indices = to_ufbxw_uint_buffer(out_scene, in_mesh->vertex_indices);
		ufbxw_int_buffer face_offsets = ufbxw_create_int_buffer(out_scene, in_mesh->faces.count);

		{
			ufbxw_int_list face_data = ufbxw_edit_int_buffer(out_scene, face_offsets);
			for (size_t i = 0; i < in_mesh->faces.count; i++) {
				face_data.data[i] = (int32_t)in_mesh->faces.data[i].index_begin;
			}
		}

		ufbxw_mesh_set_vertices(out_scene, out_mesh, vertices);
		ufbxw_mesh_set_polygons(out_scene, out_mesh, vertex_indices, face_offsets);

		if (in_mesh->materials.count == 1) {
			ufbxw_mesh_set_single_material(out_scene, out_mesh, 0);
		} else if (in_mesh->materials.count > 1) {
			ufbxw_int_buffer face_material = to_ufbxw_uint_buffer(out_scene, in_mesh->face_material);
			ufbxw_mesh_set_face_material(out_scene, out_mesh, face_material);
		}

		if (in_mesh->num_edges > 0) {
			ufbxw_int_buffer edges = ufbxw_create_int_buffer(out_scene, in_mesh->num_edges);
			ufbxw_int_list edge_data = ufbxw_edit_int_buffer(out_scene, edges);

			for (size_t i = 0; i < in_mesh->edges.count; i++) {
				edge_data.data[i] = (int32_t)in_mesh->edges.data[i].a;
			}

			ufbxw_mesh_set_fbx_edges(out_scene, out_mesh, edges);
		}

		if (in_mesh->vertex_normal.exists) {
			ufbxw_vec3_buffer normals = to_ufbxw_vec3_buffer_by_index(out_scene, in_mesh->vertex_normal);
			ufbxw_mesh_set_normals(out_scene, out_mesh, normals, UFBXW_ATTRIBUTE_MAPPING_POLYGON_VERTEX);
		}

		for (size_t uv_set = 0; uv_set < in_mesh->uv_sets.count; uv_set++) {
			ufbx_uv_set set = in_mesh->uv_sets.data[uv_set];

			ufbxw_vec2_buffer uv_values = to_ufbxw_vec2_buffer(out_scene, set.vertex_uv.values);
			ufbxw_int_buffer uv_indices = to_ufbxw_uint_buffer(out_scene, set.vertex_uv.indices);

			ufbxw_mesh_set_uvs_indexed(out_scene, out_mesh, (int32_t)uv_set, uv_values, uv_indices, UFBXW_ATTRIBUTE_MAPPING_POLYGON_VERTEX);
			ufbxw_mesh_set_attribute_name(out_scene, out_mesh, UFBXW_MESH_ATTRIBUTE_UV, (int32_t)uv_set, set.name.data);

			if (set.vertex_tangent.exists) {
				ufbxw_vec3_buffer tangents = to_ufbxw_vec3_buffer_by_index(out_scene, set.vertex_tangent);
				ufbxw_mesh_set_tangents(out_scene, out_mesh, uv_set, tangents, UFBXW_ATTRIBUTE_MAPPING_POLYGON_VERTEX);
			}

			if (set.vertex_bitangent.exists) {
				ufbxw_vec3_buffer bitangents = to_ufbxw_vec3_buffer_by_index(out_scene, set.vertex_bitangent);
				ufbxw_mesh_set_binormals(out_scene, out_mesh, uv_set, bitangents, UFBXW_ATTRIBUTE_MAPPING_POLYGON_VERTEX);
			}
		}

		for (size_t color_set = 0; color_set < in_mesh->color_sets.count; color_set++) {
			ufbx_color_set set = in_mesh->color_sets.data[color_set];

			ufbxw_vec4_buffer color_values = to_ufbxw_vec4_buffer(out_scene, set.vertex_color.values);
			ufbxw_int_buffer color_indices = to_ufbxw_uint_buffer(out_scene, set.vertex_color.indices);

			ufbxw_mesh_set_colors_indexed(out_scene, out_mesh, (int32_t)color_set, color_values, color_indices, UFBXW_ATTRIBUTE_MAPPING_POLYGON_VERTEX);
			ufbxw_mesh_set_attribute_name(out_scene, out_mesh, UFBXW_MESH_ATTRIBUTE_COLOR, (int32_t)color_set, set.name.data);
		}
	}

	for (size_t texture_ix = 0; texture_ix < in_scene->textures.count; texture_ix++) {
		ufbx_texture *in_texture = in_scene->textures.data[texture_ix];

		ufbxw_texture out_texture = ufbxw_create_texture(out_scene, UFBXW_TEXTURE_FILE);
		element_ids[in_texture->element_id] = out_texture.id;

		ufbxw_texture_set_filename(out_scene, out_texture, in_texture->absolute_filename.data);
		ufbxw_texture_set_relative_filename(out_scene, out_texture, in_texture->relative_filename.data);

		if (in_texture->content.size > 0) {
			ufbxw_byte_buffer buffer = ufbxw_view_byte_array(out_scene, in_texture->content.data, in_texture->content.size);
			ufbxw_texture_set_content(out_scene, out_texture, buffer);
		}
	}

	for (size_t material_ix = 0; material_ix < in_scene->materials.count; material_ix++) {
		ufbx_material *in_material = in_scene->materials.data[material_ix];

		ufbxw_material_type material_type = UFBXW_MATERIAL_CUSTOM;
		switch (in_material->shader_type) {
		case UFBX_SHADER_FBX_LAMBERT:
			material_type = UFBXW_MATERIAL_FBX_LAMBERT;
			break;
		case UFBX_SHADER_FBX_PHONG:
			material_type = UFBXW_MATERIAL_FBX_PHONG;
			break;
		}

		ufbxw_material out_material = ufbxw_create_material(out_scene, material_type);
		ufbxw_set_name(out_scene, out_material.id, in_material->name.data);
		element_ids[in_material->element_id] = out_material.id;

		for (size_t i = 0; i < in_material->props.props.count; i++) {
			const ufbx_prop *prop = &in_material->props.props.data[i];

			if (prop->flags & UFBX_PROP_FLAG_VALUE_INT) {
				ufbxw_add_int(out_scene, out_material.id, prop->name.data, UFBXW_PROP_TYPE_INT, (int32_t)prop->value_int);
			} else if (prop->flags & UFBX_PROP_FLAG_VALUE_REAL) {
				ufbxw_add_real(out_scene, out_material.id, prop->name.data, UFBXW_PROP_TYPE_DOUBLE, prop->value_real);
			} else if (prop->flags & UFBX_PROP_FLAG_VALUE_VEC2) {
				ufbxw_add_vec2(out_scene, out_material.id, prop->name.data, UFBXW_PROP_TYPE_VECTOR, to_ufbxw_vec2(prop->value_vec2));
			} else if (prop->flags & UFBX_PROP_FLAG_VALUE_VEC3) {
				ufbxw_add_vec3(out_scene, out_material.id, prop->name.data, UFBXW_PROP_TYPE_COLOR, to_ufbxw_vec3(prop->value_vec3));
			} else if (prop->flags & UFBX_PROP_FLAG_VALUE_VEC4) {
				ufbxw_add_vec4(out_scene, out_material.id, prop->name.data, UFBXW_PROP_TYPE_COLOR_RGBA, to_ufbxw_vec4(prop->value_vec4));
			}
		}

		for (size_t i = 0; i < in_material->textures.count; i++) {
			ufbx_material_texture in_texture = in_material->textures.data[i];

			ufbxw_id texture_id = element_ids[in_texture.texture->element_id];
			if (texture_id == 0) continue;

			ufbxw_texture out_texture = { texture_id };
			ufbxw_material_set_texture(out_scene, out_material, in_texture.material_prop.data, out_texture);
		}

		if (in_material->shader && in_material->shader->bindings.count > 0) {
			ufbx_shader *in_shader = in_material->shader;
			ufbx_shader_binding* in_binding = in_shader->bindings.data[0];

			ufbxw_implementation out_implementation = ufbxw_create_implementation(out_scene);
			ufbxw_binding_table out_binding = ufbxw_create_binding_table(out_scene);

			ufbxw_implementation_set_binding_table(out_scene, out_implementation, out_binding);
			ufbxw_material_set_implementation(out_scene, out_material, out_implementation);

			for (size_t entry_ix = 0; entry_ix < in_binding->prop_bindings.count; entry_ix++) {
				ufbx_shader_prop_binding binding = in_binding->prop_bindings.data[entry_ix];
				ufbxw_binding_table_add_entry(out_scene, out_binding, binding.material_prop.data, binding.shader_prop.data);
			}
		}
	}

	for (size_t light_ix = 0; light_ix < in_scene->lights.count; light_ix++) {
		ufbx_light *in_light = in_scene->lights.data[light_ix];
		ufbxw_light out_light = ufbxw_create_light(out_scene, ufbxw_null_node);
		element_ids[in_light->element_id] = out_light.id;

		ufbxw_light_set_intensity(out_scene, out_light, in_light->intensity * 100.0);
		ufbxw_light_set_color(out_scene, out_light, to_ufbxw_vec3(in_light->color));
		ufbxw_light_set_type(out_scene, out_light, (ufbxw_light_type)in_light->type);
		ufbxw_light_set_decay(out_scene, out_light, (ufbxw_light_decay)in_light->decay);
		ufbxw_light_set_inner_angle(out_scene, out_light, in_light->inner_angle);
		ufbxw_light_set_outer_angle(out_scene, out_light, in_light->outer_angle);
	}

	for (size_t bone_ix = 0; bone_ix < in_scene->bones.count; bone_ix++) {
		ufbx_bone *in_bone = in_scene->bones.data[bone_ix];
		ufbxw_bone out_bone = ufbxw_create_bone(out_scene, ufbxw_null_node);
		element_ids[in_bone->element_id] = out_bone.id;
	}

	// Create nodes in the original element order
	for (size_t elem_ix = 0; elem_ix < in_scene->elements.count; elem_ix++) {
		ufbx_element *in_elem = in_scene->elements.data[elem_ix];
		if (in_elem->type != UFBX_ELEMENT_NODE) continue;

		ufbx_node *in_node = ufbx_as_node(in_elem);
		if (in_node->is_root) {
			continue;
		}

		ufbxw_node out_node = ufbxw_create_node(out_scene);
		node_ids[in_node->typed_id] = out_node;
		element_ids[in_node->element_id] = out_node.id;

		ufbxw_set_name(out_scene, out_node.id, in_node->name.data);

		ufbxw_node_set_inherit_type(out_scene, out_node, to_ufbxw_inherit_type(in_node->inherit_mode));

		if (advanced_transform) {
			ufbxw_node_set_translation(out_scene, out_node, to_ufbxw_vec3(ufbx_find_vec3(&in_node->props, "Lcl Translation", ufbx_zero_vec3)));
			ufbxw_node_set_rotation(out_scene, out_node, to_ufbxw_vec3(ufbx_find_vec3(&in_node->props, "Lcl Rotation", ufbx_zero_vec3)));
			ufbxw_node_set_scaling(out_scene, out_node, to_ufbxw_vec3(ufbx_find_vec3(&in_node->props, "Lcl Scaling", one_vec3)));
			ufbxw_node_set_rotation_order(out_scene, out_node, (ufbxw_rotation_order)in_node->rotation_order);

			ufbxw_node_set_pre_rotation(out_scene, out_node, to_ufbxw_vec3(ufbx_find_vec3(&in_node->props, "PreRotation", ufbx_zero_vec3)));
			ufbxw_node_set_post_rotation(out_scene, out_node, to_ufbxw_vec3(ufbx_find_vec3(&in_node->props, "PostRotation", ufbx_zero_vec3)));
			ufbxw_node_set_rotation_offset(out_scene, out_node, to_ufbxw_vec3(ufbx_find_vec3(&in_node->props, "RotationOffset", ufbx_zero_vec3)));
			ufbxw_node_set_rotation_pivot(out_scene, out_node, to_ufbxw_vec3(ufbx_find_vec3(&in_node->props, "RotationPivot", ufbx_zero_vec3)));
			ufbxw_node_set_scaling_offset(out_scene, out_node, to_ufbxw_vec3(ufbx_find_vec3(&in_node->props, "ScalingOffset", ufbx_zero_vec3)));
			ufbxw_node_set_scaling_pivot(out_scene, out_node, to_ufbxw_vec3(ufbx_find_vec3(&in_node->props, "ScalingPivot", ufbx_zero_vec3)));

			if (in_node->has_geometry_transform) {
				ufbxw_node_set_geometric_translation(out_scene, out_node, to_ufbxw_vec3(ufbx_find_vec3(&in_node->props, "GeometricTranslation", ufbx_zero_vec3)));
				ufbxw_node_set_geometric_rotation(out_scene, out_node, to_ufbxw_vec3(ufbx_find_vec3(&in_node->props, "GeometricRotation", ufbx_zero_vec3)));
				ufbxw_node_set_geometric_scaling(out_scene, out_node, to_ufbxw_vec3(ufbx_find_vec3(&in_node->props, "GeometricScaling", one_vec3)));
			}
		} else {
			ufbxw_node_set_translation(out_scene, out_node, to_ufbxw_vec3(in_node->local_transform.translation));
			ufbxw_node_set_rotation(out_scene, out_node, to_ufbxw_euler(in_node->local_transform.rotation));
			ufbxw_node_set_scaling(out_scene, out_node, to_ufbxw_vec3(in_node->local_transform.scale));

			if (in_node->has_geometry_transform) {
				ufbxw_node_set_geometric_translation(out_scene, out_node, to_ufbxw_vec3(in_node->geometry_transform.translation));
				ufbxw_node_set_geometric_rotation(out_scene, out_node, to_ufbxw_euler(in_node->geometry_transform.rotation));
				ufbxw_node_set_geometric_scaling(out_scene, out_node, to_ufbxw_vec3(in_node->geometry_transform.scale));
			}
		}

		if (in_node->mesh) {
			ufbxw_mesh_add_instance(out_scene, mesh_ids[in_node->mesh->typed_id], out_node);
		} else if (in_node->attrib) {
			ufbxw_id attrib_id = element_ids[in_node->attrib->element_id];
			if (attrib_id != 0) {
				ufbxw_node_set_attribute(out_scene, out_node, attrib_id);
			}
		}

		for (size_t i = 0; i < in_node->materials.count; i++) {
			ufbxw_id material_id = element_ids[in_node->materials.data[i]->element_id];
			if (material_id == 0) continue;

			ufbxw_material out_material = { material_id };
			ufbxw_node_set_material(out_scene, out_node, i, out_material);
		}
	}

	// Hook the parents
	for (size_t node_ix = 0; node_ix < in_scene->nodes.count; node_ix++) {
		ufbx_node *in_node = in_scene->nodes.data[node_ix];

		if (in_node->parent && !in_node->parent->is_root) {
			ufbxw_node out_node = node_ids[in_node->typed_id];
			ufbxw_node parent_node = node_ids[in_node->parent->typed_id];

			ufbxw_node_set_parent(out_scene, out_node, parent_node);
		}
	}

	// Skinning 
	for (size_t skin_ix = 0; skin_ix < in_scene->skin_deformers.count; skin_ix++) {
		ufbx_skin_deformer *in_skin = in_scene->skin_deformers.data[skin_ix];
		ufbxw_skin_deformer out_skin = ufbxw_create_skin_deformer(out_scene, ufbxw_null_mesh);

		skin_deformer_ids[skin_ix] = out_skin;
		element_ids[in_skin->element_id] = out_skin.id;

		for (size_t cluster_ix = 0; cluster_ix < in_skin->clusters.count; cluster_ix++) {
			ufbx_skin_cluster* in_cluster = in_skin->clusters.data[cluster_ix];

			ufbxw_node out_node = node_ids[in_cluster->bone_node->typed_id];
			ufbxw_skin_cluster out_cluster = ufbxw_create_skin_cluster(out_scene, out_skin, out_node);

			ufbxw_int_buffer indices = to_ufbxw_uint_buffer(out_scene, in_cluster->vertices);
			ufbxw_real_buffer weights = to_ufbxw_real_buffer(out_scene, in_cluster->weights);

			ufbxw_skin_cluster_set_weights(out_scene, out_cluster, indices, weights);

			ufbxw_skin_cluster_set_transform(out_scene, out_cluster, to_ufbxw_matrix(in_cluster->mesh_node_to_bone));
			ufbxw_skin_cluster_set_link_transform(out_scene, out_cluster, to_ufbxw_matrix(in_cluster->bind_to_world));
		}
	}

	// Blend deformers
	for (size_t blend_ix = 0; blend_ix < in_scene->blend_deformers.count; blend_ix++) {
		ufbx_blend_deformer *in_blend = in_scene->blend_deformers.data[blend_ix];
		ufbxw_blend_deformer out_blend = ufbxw_create_blend_deformer(out_scene, ufbxw_null_mesh);
		element_ids[in_blend->element_id] = out_blend.id;
		blend_deformer_ids[blend_ix] = out_blend;

		for (size_t channel_ix = 0; channel_ix < in_blend->channels.count; channel_ix++) {
			ufbx_blend_channel *in_channel = in_blend->channels.data[channel_ix];
			ufbxw_blend_channel out_channel = ufbxw_create_blend_channel(out_scene, out_blend);
			element_ids[in_channel->element_id] = out_channel.id;

			for (size_t shape_ix = 0; shape_ix < in_channel->keyframes.count; shape_ix++) {
				ufbx_blend_keyframe in_keyframe = in_channel->keyframes.data[shape_ix];
				ufbx_blend_shape *in_shape = in_keyframe.shape;
				ufbxw_blend_shape out_shape = ufbxw_create_blend_shape(out_scene);
				element_ids[in_shape->element_id] = out_shape.id;

				ufbxw_int_buffer indices = to_ufbxw_uint_buffer(out_scene, in_shape->offset_vertices);
				ufbxw_vec3_buffer offsets = to_ufbxw_vec3_buffer(out_scene, in_shape->position_offsets);
				ufbxw_real target_weight = in_keyframe.target_weight * 100.0;

				ufbxw_blend_shape_set_offsets(out_scene, out_shape, indices, offsets);
				ufbxw_blend_channel_add_shape(out_scene, out_channel, out_shape, target_weight);
			}
		}
	}

	for (size_t mesh_ix = 0; mesh_ix < in_scene->meshes.count; mesh_ix++) {
		ufbx_mesh* in_mesh = in_scene->meshes.data[mesh_ix];
		ufbxw_mesh out_mesh = mesh_ids[in_mesh->typed_id];

		for (size_t skin_ix = 0; skin_ix < in_mesh->skin_deformers.count; skin_ix++) {
			ufbx_skin_deformer* in_skin = in_mesh->skin_deformers.data[skin_ix];
			ufbxw_skin_deformer out_skin = skin_deformer_ids[in_skin->typed_id];

			ufbxw_skin_deformer_add_mesh(out_scene, out_skin, out_mesh);
		}

		for (size_t blend_ix = 0; blend_ix < in_mesh->blend_deformers.count; blend_ix++) {
			ufbx_blend_deformer* in_blend = in_mesh->blend_deformers.data[blend_ix];
			ufbxw_blend_deformer out_blend = blend_deformer_ids[in_blend->typed_id];

			ufbxw_blend_deformer_add_mesh(out_scene, out_blend, out_mesh);
		}
	}

	for (size_t layer_ix = 0; layer_ix < in_scene->anim_layers.count; layer_ix++) {
		ufbx_anim_layer *in_layer = in_scene->anim_layers.data[layer_ix];
		ufbxw_anim_layer out_layer = ufbxw_create_anim_layer(out_scene, ufbxw_null_anim_stack);
		anim_layer_ids[layer_ix] = out_layer;
		element_ids[in_layer->element_id] = out_layer.id;

		for (size_t prop_ix = 0; prop_ix < in_layer->anim_props.count; prop_ix++) {
			ufbx_anim_prop *in_prop = &in_layer->anim_props.data[prop_ix];

			const ufbxw_id dst_id = element_ids[in_prop->element->element_id];
			if (!dst_id) {
				fprintf(stderr, "Ignoring animation on missing %s '%s'\n",
					ufbxwt_ufbx_element_type_names[in_prop->element->type],
					in_prop->element->name.data);
				continue;
			}

			ufbxw_id anim_id = dst_id;
			const char *anim_prop = in_prop->prop_name.data;

			// Retarget legacy blend channel properties into DeformPercent
			ufbx_element *deform_element = find_deform_percent_element(in_prop->element, anim_prop);
			if (deform_element) {
				anim_id = element_ids[deform_element->element_id];
				anim_prop = "DeformPercent";
			}

			ufbxw_anim_prop out_anim = ufbxw_animate_prop(out_scene, anim_id, anim_prop, out_layer);

			ufbx_anim_value *in_value = in_prop->anim_value;
			for (size_t curve_ix = 0; curve_ix < 3; curve_ix++) {
				ufbx_anim_curve *in_curve = in_prop->anim_value->curves[curve_ix];
				ufbxw_anim_curve out_curve = ufbxw_anim_get_curve(out_scene, out_anim, curve_ix);
				if (!out_curve.id) continue;

				ufbxw_anim_set_default_value(out_scene, out_anim, curve_ix, in_value->default_value.v[curve_ix]);

				if (in_curve) {
					ufbxw_anim_curve_set_pre_extrapolation(out_scene, out_curve, (ufbxw_extrapolation_type)in_curve->pre_extrapolation.mode);
					ufbxw_anim_curve_set_pre_extrapolation_repeat_count(out_scene, out_curve, in_curve->pre_extrapolation.repeat_count);

					ufbxw_anim_curve_set_post_extrapolation(out_scene, out_curve, (ufbxw_extrapolation_type)in_curve->post_extrapolation.mode);
					ufbxw_anim_curve_set_post_extrapolation_repeat_count(out_scene, out_curve, in_curve->post_extrapolation.repeat_count);

					for (size_t key_ix = 0; key_ix < in_curve->keyframes.count; key_ix++) {
						ufbx_keyframe in_key = in_curve->keyframes.data[key_ix];
						ufbxw_keyframe_real out_key = { 0 };

						out_key.flags = UFBXW_KEYFRAME_LINEAR;
						out_key.time = (ufbxw_ktime)round(in_key.time * UFBXW_KTIME_SECOND);
						out_key.value = in_key.value;

						ufbx_keyframe *prev_key = key_ix > 0 ? &in_curve->keyframes.data[key_ix - 1] : NULL;
						ufbx_keyframe *next_key = key_ix + 1 < in_curve->keyframes.count ? &in_curve->keyframes.data[key_ix + 1] : NULL;

						switch (in_key.interpolation) {
						case UFBX_INTERPOLATION_CONSTANT_PREV:
							out_key.flags = UFBXW_KEYFRAME_CONSTANT;
							break;
						case UFBX_INTERPOLATION_CONSTANT_NEXT:
							out_key.flags = UFBXW_KEYFRAME_CONSTANT_NEXT;
							break;
						case UFBX_INTERPOLATION_LINEAR:
							out_key.flags = UFBXW_KEYFRAME_LINEAR;
							break;
						case UFBX_INTERPOLATION_CUBIC:
							out_key.flags = UFBXW_KEYFRAME_CUBIC_USER_BROKEN;
							if (prev_key) {
								out_key.slope_left = in_key.left.dy / in_key.left.dx;
								out_key.weight_left = in_key.left.dx / (in_key.time - prev_key->time);
								out_key.flags |= UFBXW_KEYFRAME_WEIGHTED_LEFT;
							}
							if (next_key) {
								out_key.slope_right = in_key.right.dy / in_key.right.dx;
								out_key.weight_right = in_key.right.dx / (next_key->time - in_key.time);
								out_key.flags |= UFBXW_KEYFRAME_WEIGHTED_RIGHT;
							}
							break;
						}

						ufbxw_anim_curve_add_keyframe_key(out_scene, out_curve, out_key);
					}
				}
			}
		}
	}

	for (size_t stack_ix = 0; stack_ix < in_scene->anim_stacks.count; stack_ix++) {
		ufbx_anim_stack *in_stack = in_scene->anim_stacks.data[stack_ix];
		ufbxw_anim_stack out_stack = ufbxw_create_anim_stack(out_scene);
		anim_stack_ids[stack_ix] = out_stack;
		element_ids[in_stack->element_id] = out_stack.id;

		ufbxw_set_name(out_scene, out_stack.id, in_stack->name.data);

		ufbxw_ktime time_begin = (ufbxw_ktime)round(in_stack->time_begin * UFBXW_KTIME_SECOND);
		ufbxw_ktime time_end = (ufbxw_ktime)round(in_stack->time_end * UFBXW_KTIME_SECOND);
		ufbxw_anim_stack_set_time_range(out_scene, out_stack, time_begin, time_end);

		for (size_t layer_ix = 0; layer_ix < in_stack->layers.count; layer_ix++) {
			ufbxw_anim_layer out_layer = anim_layer_ids[in_stack->layers.data[layer_ix]->typed_id];
			ufbxw_anim_layer_set_stack(out_scene, out_layer, out_stack);
		}
	}

	free(mesh_ids);
	free(node_ids);
	free(anim_stack_ids);
	free(anim_layer_ids);
	free(element_ids);
	ufbx_free_scene(in_scene);

	ufbxw_prepare_scene(out_scene, NULL);

	ufbxw_save_opts save_opts = { 0 };
	if (!strcmp(format, "ascii")) {
		save_opts.format = UFBXW_SAVE_FORMAT_ASCII;
	} else if (!strcmp(format, "binary")) {
		save_opts.format = UFBXW_SAVE_FORMAT_BINARY;
	} else {
		ufbxwt_assert(0 && "specify -f ascii or -f binary");
	}
	save_opts.version = 7500;

	ufbxwt_assert(ufbxwt_deflate_setup(&save_opts.deflate, deflate_impl));
	ufbxwt_assert(ufbxwt_ascii_format_setup(&save_opts.ascii_formatter, ascii_impl));
	ufbxwt_assert(ufbxwt_thread_setup(&save_opts.thread_sync, &save_opts.thread_pool, thread_impl));

	ufbxw_error save_error;
	bool ok = ufbxw_save_file(out_scene, output_path, &save_opts, &save_error);
	if (!ok) {
		fprintf(stderr, "failed to save: %s\n", save_error.description);
		exit(3);
	}

	ufbxw_free_scene(out_scene);

	int result = 0;

	if (compare) {
		compare_fbx_opts compare_opts = { 0 };
		compare_opts.approx_epsilon = 1e-3;
		compare_opts.compare_anim = true;

		if (!compare_fbx(output_path, input_path, &compare_opts)) {
			result = 1;
		}
	}

	return result;
}
