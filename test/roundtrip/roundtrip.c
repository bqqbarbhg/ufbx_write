#include "../../ufbx_write.h"
#include "../ufbx/ufbx.h"
#include "../util/ufbxwt_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(int argc, char **argv)
{
	const char *output_path = NULL;
	const char *input_path = NULL;
	const char *format = "";

	ufbxwt_deflate_impl deflate_impl = UFBXWT_DEFLATE_IMPL_NONE;
	ufbxwt_ascii_format_impl ascii_impl = UFBXWT_ASCII_FORMAT_IMPL_DEFAULT;
	ufbxwt_thread_impl thread_impl = UFBXWT_THREAD_IMPL_NONE;

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

	ufbxw_scene *out_scene = ufbxw_create_scene(NULL);

	ufbxw_mesh *mesh_ids = (ufbxw_mesh*)calloc(in_scene->meshes.count, sizeof(ufbxw_mesh));
	ufbxw_node *node_ids = (ufbxw_node*)calloc(in_scene->nodes.count, sizeof(ufbxw_node));

	for (size_t mesh_ix = 0; mesh_ix < in_scene->meshes.count; mesh_ix++) {
		ufbx_mesh *in_mesh = in_scene->meshes.data[mesh_ix];
		ufbxw_mesh out_mesh = ufbxw_create_mesh(out_scene);

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
		}

		mesh_ids[mesh_ix] = out_mesh;
	}

	for (size_t node_ix = 0; node_ix < in_scene->nodes.count; node_ix++) {
		ufbx_node *in_node = in_scene->nodes.data[node_ix];
		ufbxw_node out_node = ufbxw_create_node(out_scene);

		ufbxw_set_name(out_scene, out_node.id, in_node->name.data);

		ufbxw_node_set_translation(out_scene, out_node, to_ufbxw_vec3(in_node->local_transform.translation));
		ufbxw_node_set_rotation(out_scene, out_node, to_ufbxw_euler(in_node->local_transform.rotation));
		ufbxw_node_set_scaling(out_scene, out_node, to_ufbxw_vec3(in_node->local_transform.scale));

		if (in_node->mesh) {
			ufbxw_mesh_add_instance(out_scene, mesh_ids[in_node->mesh->typed_id], out_node);
		}

		node_ids[node_ix] = out_node;
	}

	free(mesh_ids);
	free(node_ids);
	ufbx_free_scene(in_scene);

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

	return 0;
}
