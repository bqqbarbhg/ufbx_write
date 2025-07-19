#include "../ufbx_write.h"

#define array_count(arr) (sizeof(arr) / sizeof(*(arr)))

static ufbxw_vec3 make_vec3(ufbxw_real x, ufbxw_real y, ufbxw_real z)
{
	ufbxw_vec3 v = { x, y, z };
	return v;
}

typedef struct {
	float x, y, z;
} vector3_t;

int main(int argc, char **argv)
{
	ufbxw_scene *scene = ufbxw_create_scene(NULL);

	ufbxw_node node = ufbxw_create_node(scene);
	ufbxw_set_name(scene, node.id, "Node");

	ufbxw_node parent = ufbxw_create_node(scene);
	ufbxw_set_name(scene, parent.id, "Parent");
	ufbxw_node_set_parent(scene, node, parent);

	ufbxw_vec3 pos = { 1.0f, 2.0f, 3.0f };
	ufbxw_node_set_translation(scene, node, pos);

	ufbxw_mesh mesh = ufbxw_create_mesh(scene);
	ufbxw_set_name(scene, mesh.id, "Cube");

	vector3_t vertices[] = {
		{ -1.0f, 0.0f, 0.0f, },
		{ +1.0f, 0.0f, 0.0f, },
		{ -1.0f, 0.0f, 1.0f, },
		{ +1.0f, 0.0f, 1.0f, },
		{ -1.0f, 0.0f, 2.0f, },
		{ +1.0f, 0.0f, 2.0f, },
	};
	int32_t indices[] = {
		0, 2, 3, 1, 2, 4, 5, 3,
	};
	int32_t face_offsets[] = {
		0, 4, 8,
	};
	ufbxw_vec3 normals[] = {
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
	};
	ufbxw_vec2 uvs[] = {
		{ 0.0f, 0.0f, },
		{ 0.0f, 0.5f, },
		{ 1.0f, 0.5f, },
		{ 1.0f, 0.0f, },
		{ 0.0f, 0.5f, },
		{ 0.0f, 1.0f, },
		{ 1.0f, 1.0f, },
		{ 1.0f, 0.5f, },
	};
	int32_t materials[] = {
		0, 1,
	};

	ufbxw_vec3_buffer vertex_buffer = ufbxw_create_vec3_buffer(scene, array_count(vertices));

	ufbxw_vec3_list dst_data = ufbxw_edit_vec3_buffer(scene, vertex_buffer);
	for (size_t i = 0; i < array_count(vertices); i++) {
		dst_data.data[i].x = vertices[i].x;
		dst_data.data[i].y = vertices[i].y;
		dst_data.data[i].z = vertices[i].z;
	}

	ufbxw_mesh_set_vertices(scene, mesh, vertex_buffer);

	ufbxw_int_buffer indices_buf = ufbxw_view_int_array(scene, indices, array_count(indices));
	ufbxw_int_buffer offsets_buf = ufbxw_view_int_array(scene, face_offsets, array_count(face_offsets));
	ufbxw_mesh_set_polygons(scene, mesh, indices_buf, offsets_buf);

	ufbxw_vec3_buffer normals_buffer = ufbxw_view_vec3_array(scene, normals, array_count(normals));
	ufbxw_mesh_set_normals(scene, mesh, normals_buffer, UFBXW_ATTRIBUTE_MAPPING_VERTEX);

	ufbxw_vec2_buffer uvs_buffer = ufbxw_external_vec2_array(scene, uvs, array_count(uvs));
	ufbxw_mesh_set_uvs(scene, mesh, 0, uvs_buffer, UFBXW_ATTRIBUTE_MAPPING_POLYGON_VERTEX);

	ufbxw_int_buffer mat_buffer = ufbxw_external_int_array(scene, materials, array_count(materials));
	ufbxw_mesh_set_face_material(scene, mesh, mat_buffer);

	ufbxw_anim_layer anim_layer = ufbxw_get_default_anim_layer(scene);
	ufbxw_anim_prop anim_t = ufbxw_node_animate_translation(scene, node, anim_layer);

	ufbxw_anim_add_keyframe_vec3(scene, anim_t, (ufbxw_ktime)(0.0 * UFBXW_KTIME_SECOND), make_vec3(0.0f, 0.0f, 0.0f), UFBXW_KEYFRAME_CUBIC_AUTO);
	ufbxw_anim_add_keyframe_vec3(scene, anim_t, (ufbxw_ktime)(1.0 * UFBXW_KTIME_SECOND), make_vec3(1.0f, 0.3f, 0.0f), UFBXW_KEYFRAME_CUBIC_AUTO);
	ufbxw_anim_add_keyframe_vec3(scene, anim_t, (ufbxw_ktime)(2.0 * UFBXW_KTIME_SECOND), make_vec3(0.0f, 0.6f, 0.0f), UFBXW_KEYFRAME_CUBIC_AUTO);

	ufbxw_id mat_a = ufbxw_create_element_ex(scene, UFBXW_ELEMENT_MATERIAL, "FbxSurfaceLambert");
	ufbxw_id mat_b = ufbxw_create_element_ex(scene, UFBXW_ELEMENT_MATERIAL, "FbxSurfaceLambert");

	ufbxw_set_name(scene, mat_a, "Material_Red");
	ufbxw_set_name(scene, mat_b, "Material_Green");

	ufbxw_set_vec3(scene, mat_a, "DiffuseColor", make_vec3(1.0f, 0.2f, 0.2f));
	ufbxw_set_vec3(scene, mat_b, "DiffuseColor", make_vec3(0.2f, 1.0f, 0.2f));

	// TODO: Make ufbxw_connect() take the connection type
	ufbxw_connect(scene, mat_a, node.id);
	ufbxw_connect(scene, mat_b, node.id);

	ufbxw_save_opts opts = { 0 };
	opts.ascii = true;
	opts.debug_comments = true;

	ufbxw_error error;
	ufbxw_save_file(scene, "test.fbx", &opts, &error);

	ufbxw_free_scene(scene);
}
