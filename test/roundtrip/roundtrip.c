#include "../../ufbx_write.h"
#include "../ufbx/ufbx.h"
#include "../util/ufbxwt_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../compare_fbx/compare_fbx.h"

static void ufbxwt_assert_fail(const char *file, uint32_t line, const char *expr) {
	fprintf(stderr, "assert fail: %s (%s:%u)\n", expr, file, line);
	exit(1);
}

#define ufbxwt_assert(cond) do { \
		if (!(cond)) ufbxwt_assert_fail(__FILE__, __LINE__, #cond); \
	} while (0)

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

static ufbxw_vec3 to_ufbxw_euler(ufbx_quat v)
{
	ufbx_vec3 euler = ufbx_quat_to_euler(v, UFBX_ROTATION_ORDER_XYZ);
	return to_ufbxw_vec3(euler);
}

static const ufbx_vec3 one_vec3 = { 1.0f, 1.0f, 1.0f };

int main(int argc, char **argv)
{
	const char *output_path = NULL;
	const char *input_path = NULL;
	const char *format = "";
	bool compare = false;

	ufbxwt_deflate_impl deflate_impl = UFBXWT_DEFLATE_IMPL_NONE;
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
						ascii_impl = (ufbxwt_deflate_impl)i;
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

	ufbxw_mesh *mesh_ids = (ufbxw_mesh*)calloc(in_scene->meshes.count, sizeof(ufbxw_mesh));
	ufbxw_node *node_ids = (ufbxw_node*)calloc(in_scene->nodes.count, sizeof(ufbxw_node));
	ufbxw_anim_stack *anim_stack_ids = (ufbxw_anim_stack*)calloc(in_scene->anim_stacks.count, sizeof(ufbxw_anim_stack));
	ufbxw_anim_layer *anim_layer_ids = (ufbxw_anim_layer*)calloc(in_scene->anim_layers.count, sizeof(ufbxw_anim_layer));
	ufbxw_id *element_ids = (ufbxw_id*)calloc(in_scene->elements.count, sizeof(ufbxw_id));

	for (size_t mesh_ix = 0; mesh_ix < in_scene->meshes.count; mesh_ix++) {
		ufbx_mesh *in_mesh = in_scene->meshes.data[mesh_ix];
		ufbxw_mesh out_mesh = ufbxw_create_mesh(out_scene);
		mesh_ids[mesh_ix] = out_mesh;
		element_ids[in_mesh->element_id] = out_mesh.id;

		ufbxw_vec3_buffer vertices = ufbxw_create_vec3_buffer(out_scene, in_mesh->vertices.count);
		ufbxw_int_buffer vertex_indices = ufbxw_create_int_buffer(out_scene, in_mesh->vertex_indices.count);
		ufbxw_int_buffer face_offsets = ufbxw_create_int_buffer(out_scene, in_mesh->faces.count);

		{
			ufbxw_vec3_list vertex_data = ufbxw_edit_vec3_buffer(out_scene, vertices);
			for (size_t i = 0; i < in_mesh->vertices.count; i++) {
				vertex_data.data[i] = to_ufbxw_vec3(in_mesh->vertices.data[i]);
			}
		}

		{
			ufbxw_int_list index_data = ufbxw_edit_int_buffer(out_scene, vertex_indices);
			for (size_t i = 0; i < in_mesh->vertex_indices.count; i++) {
				index_data.data[i] = (int32_t)in_mesh->vertex_indices.data[i];
			}
		}

		{
			ufbxw_int_list face_data = ufbxw_edit_int_buffer(out_scene, face_offsets);
			for (size_t i = 0; i < in_mesh->faces.count; i++) {
				face_data.data[i] = (int32_t)in_mesh->faces.data[i].index_begin;
			}
		}

		ufbxw_mesh_set_vertices(out_scene, out_mesh, vertices);
		ufbxw_mesh_set_polygons(out_scene, out_mesh, vertex_indices, face_offsets);

		if (in_mesh->vertex_normal.exists) {
			ufbxw_vec3_buffer normals = ufbxw_create_vec3_buffer(out_scene, in_mesh->num_indices);

			{
				ufbxw_vec3_list normals_data = ufbxw_edit_vec3_buffer(out_scene, normals);
				for (size_t i = 0; i < in_mesh->num_indices; i++) {
					normals_data.data[i] = to_ufbxw_vec3(ufbx_get_vertex_vec3(&in_mesh->vertex_normal, i));
				}
			}

			ufbxw_mesh_set_normals(out_scene, out_mesh, normals, UFBXW_ATTRIBUTE_MAPPING_POLYGON_VERTEX);
		}

		for (size_t uv_set = 0; uv_set < in_mesh->uv_sets.count; uv_set++) {
			ufbx_uv_set set = in_mesh->uv_sets.data[uv_set];

			ufbxw_vec2_buffer uv_values = ufbxw_create_vec2_buffer(out_scene, set.vertex_uv.values.count);
			ufbxw_int_buffer uv_indices = ufbxw_create_int_buffer(out_scene, set.vertex_uv.indices.count);

			{
				ufbxw_vec2_list values_data = ufbxw_edit_vec2_buffer(out_scene, uv_values);
				for (size_t i = 0; i < set.vertex_uv.values.count; i++) {
					values_data.data[i] = to_ufbxw_vec2(set.vertex_uv.values.data[i]);
				}
			}

			{
				ufbxw_int_list indices_data = ufbxw_edit_int_buffer(out_scene, uv_indices);
				for (size_t i = 0; i < set.vertex_uv.indices.count; i++) {
					indices_data.data[i] = (int32_t)set.vertex_uv.indices.data[i];
				}
			}

			ufbxw_mesh_set_uvs_indexed(out_scene, out_mesh, (int32_t)uv_set, uv_values, uv_indices, UFBXW_ATTRIBUTE_MAPPING_POLYGON_VERTEX);
			ufbxw_mesh_set_attribute_name(out_scene, out_mesh, UFBXW_MESH_ATTRIBUTE_UV, (int32_t)uv_set, set.name.data);
		}
	}

	for (size_t node_ix = 0; node_ix < in_scene->nodes.count; node_ix++) {
		ufbx_node *in_node = in_scene->nodes.data[node_ix];
		if (in_node->is_root) {
			continue;
		}

		ufbxw_node out_node = ufbxw_create_node(out_scene);
		node_ids[node_ix] = out_node;
		element_ids[in_node->element_id] = out_node.id;

		ufbxw_set_name(out_scene, out_node.id, in_node->name.data);

		ufbxw_node_set_inherit_type(out_scene, out_node, (ufbxw_inherit_type)in_node->inherit_mode);

		if (in_node->parent && !in_node->parent->is_root) {
			ufbxw_node_set_parent(out_scene, out_node, node_ids[in_node->parent->typed_id]);
		}

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
		}
	}

	for (size_t layer_ix = 0; layer_ix < in_scene->anim_layers.count; layer_ix++) {
		ufbx_anim_layer *in_layer = in_scene->anim_layers.data[layer_ix];
		ufbxw_anim_layer out_layer = ufbxw_create_anim_layer(out_scene, ufbxw_null_anim_stack);
		anim_layer_ids[layer_ix] = out_layer;
		element_ids[in_layer->element_id] = out_layer.id;

		for (size_t prop_ix = 0; prop_ix < in_layer->anim_props.count; prop_ix++) {
			ufbx_anim_prop *in_prop = &in_layer->anim_props.data[prop_ix];

			ufbxw_id dst_id = element_ids[in_prop->element->element_id];
			ufbxwt_assert(dst_id != 0);
			ufbxw_anim_prop out_anim = ufbxw_animate_prop(out_scene, dst_id, in_prop->prop_name.data, out_layer);

			ufbx_anim_value *in_value = in_prop->anim_value;
			for (size_t curve_ix = 0; curve_ix < 3; curve_ix++) {
				ufbx_anim_curve *in_curve = in_prop->anim_value->curves[curve_ix];
				ufbxw_anim_curve out_curve = ufbxw_anim_get_curve(out_scene, out_anim, curve_ix);
				if (!out_curve.id) continue;

				ufbxw_anim_set_default_value(out_scene, out_anim, curve_ix, in_value->default_value.v[curve_ix]);

				if (in_curve) {
					for (size_t key_ix = 0; key_ix < in_curve->keyframes.count; key_ix++) {
						ufbx_keyframe in_key = in_curve->keyframes.data[key_ix];
						ufbxw_keyframe_real out_key = { 0 };

						out_key.flags = UFBXW_KEYFRAME_LINEAR;
						out_key.time = (ufbxw_ktime)round(in_key.time * UFBXW_KTIME_SECOND);
						out_key.value = in_key.value;

						ufbx_keyframe *prev_key = key_ix > 0 ? &in_curve->keyframes.data[key_ix - 1] : NULL;
						ufbx_keyframe *next_key = key_ix > 0 ? &in_curve->keyframes.data[key_ix + 1] : NULL;

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
								out_key.slope_left = in_key.left.dy;
								out_key.weight_left = in_key.left.dx / (in_key.time - prev_key->time);
							}
							if (next_key) {
								out_key.slope_right = in_key.left.dy;
								out_key.weight_right = in_key.right.dx / (next_key->time - in_key.time);
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
		if (!compare_fbx(output_path, input_path)) {
			result = 1;
		}
	}

	return result;
}
