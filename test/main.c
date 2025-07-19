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

static size_t vec3_count_stream(void *user, ufbxw_vec3 *dst, size_t dst_size, size_t offset)
{
	for (size_t i = 0; i < dst_size; i++) {
		ufbxw_real t = (ufbxw_real)(offset + i);
		dst[i].x = t + 0.25;
		dst[i].y = t + 0.5;
		dst[i].z = t + 0.75;
	}
	return dst_size;
}

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
		4, 8,
	};
	int32_t polygon_vertices[] = {
		0, 2, 3, ~1, 2, 4, 5, ~3,
	};
	int32_t triangle_indices[] = {
		0, 2, 1, 1, 2, 3, 2, 4, 3, 3, 4, 5,
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

#if 0

	ufbxw_mesh_attribute_desc desc = { 0 };
	desc.name = ufbxw_str("UV Map");
	desc.values = uvs_buffer.id;
	desc.mapping = UFBXW_ATTRIBUTE_MAPPING_POLYGON_VERTEX;
	desc.generate_indices = true;

	ufbxw_mesh_set_attribute(scene, mesh, UFBXW_MESH_ATTRIBUTE_UV, 0, &desc);
#endif

	ufbxw_mesh_add_instance(scene, mesh, node);

#if 0
	ufbxw_id material = ufbxw_create_element_ex(scene, UFBXW_ELEMENT_MATERIAL, "FbxSurfaceLambert");
	ufbxw_set_name(scene, material, "MyLambert");
	ufbxw_set_vec3(scene, material, "DiffuseColor", make_vec3(0.2, 0.4, 0.6));

	ufbxw_id texture = ufbxw_create_element_ex(scene, UFBXW_ELEMENT_TEXTURE, "FbxFileTexture");
	ufbxw_set_name(scene, texture, "MyTexture");
	ufbxw_set_string(scene, texture, "Path", "C:/Files/texture.png");
	ufbxw_set_string(scene, texture, "RelPath", "texture.png");

	ufbxw_node light_node = ufbxw_create_node(scene);
	ufbxw_set_name(scene, light_node.id, "Light");
	ufbxw_set_vec3(scene, light_node.id, "Lcl Translation", make_vec3(5.0, 5.0, 5.0));

	ufbxw_id light = ufbxw_create_element(scene, UFBXW_ELEMENT_LIGHT);
	ufbxw_set_name(scene, light, "lightAttribute1");
	ufbxw_set_vec3(scene, light, "Color", make_vec3(1.0f, 0.5f, 0.25f));

	ufbxw_connect(scene, light, light_node.id);
	ufbxw_connect(scene, material, node.id);
	ufbxw_connect_prop(scene, texture, "", material, "TransparentColor");

	ufbxw_anim_layer anim_layer = ufbxw_get_default_anim_layer(scene);

	typedef struct {
		double time;
		ufbxw_vec3 translation;
		ufbxw_vec3 rotation;
		ufbxw_vec3 scaling;
		ufbxw_real intensity;
	} keyframe_t;

	static const keyframe_t keys[] = {
		{ 0.0, { 0.0, 0.0, 0.0 }, { 0.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0 }, 100.0 },
		{ 0.5, { 1.2, -0.5, 0.0 }, { 0.0, 90.0, 0.0 }, { 2.0, 2.0, 2.0 }, 80.0 },
		{ 1.0, { 1.0, 1.0, 0.0 }, { 90.0, 90.0, 0.0 }, { 1.0, 1.0, 1.0 }, 60.0 },
	};

	ufbxw_anim_prop anim_t = ufbxw_node_animate_translation(scene, node, anim_layer);
	ufbxw_anim_prop anim_r = ufbxw_node_animate_rotation(scene, node, anim_layer);
	ufbxw_anim_prop anim_s = ufbxw_node_animate_scaling(scene, node, anim_layer);
	ufbxw_anim_prop anim_i = ufbxw_animate_prop(scene, light, "Intensity", anim_layer);

	for (size_t i = 0; i < array_count(keys); i++) {
		ufbxw_ktime time = (ufbxw_ktime)(keys[i].time * (double)UFBXW_KTIME_SECOND);
		ufbxw_vec3 value_t = keys[i].translation;
		ufbxw_vec3 value_r = keys[i].rotation;
		ufbxw_vec3 value_s = keys[i].scaling;
		ufbxw_real value_i = keys[i].intensity;

		ufbxw_anim_add_keyframe_vec3(scene, anim_t, time, value_t, UFBXW_KEYFRAME_CUBIC_AUTO);
		ufbxw_anim_add_keyframe_vec3(scene, anim_r, time, value_r, UFBXW_KEYFRAME_CUBIC_AUTO);
		ufbxw_anim_add_keyframe_vec3(scene, anim_s, time, value_s, UFBXW_KEYFRAME_CUBIC_AUTO);
		ufbxw_anim_add_keyframe_real(scene, anim_i, time, value_i, UFBXW_KEYFRAME_LINEAR);
	}
#endif

#if 0
	ufbxw_node parent = ufbxw_create_node(scene);
	ufbxw_set_name(scene, parent.id, "Parent");

	ufbxw_vec3 pos = { 1.0f, 2.0f, 3.0f };
	ufbxw_vec3 scale = { 2.0f, 2.0f, 2.0f };

	ufbxw_vec3 old_scaling = ufbxw_get_vec3(scene, parent.id, "ScalingMax");

	ufbxw_vec3 val_scaling = { 0.1, 0.2, 0.3 };
	ufbxw_set_vec3(scene, parent.id, "ScalingMax", val_scaling);

	ufbxw_add_int(scene, parent.id, "UserThingy", UFBXW_PROP_TYPE_INT, 123);

	ufbxw_vec3 new_scaling = ufbxw_get_vec3(scene, parent.id, "ScalingMax");

	ufbxw_node_set_translation(scene, parent, pos);
	ufbxw_set_vec3(scene, parent.id, "Lcl Scaling", scale);

	ufbxw_vec3 ref_pos = ufbxw_get_vec3(scene, parent.id, "Lcl Translation");
	ufbxw_vec3 ref_scale = ufbxw_node_get_scaling(scene, parent);

	ufbxw_node cube = ufbxw_create_node(scene);
	ufbxw_set_name(scene, cube.id, "Cube");
	ufbxw_node_set_parent(scene, cube, parent);

	ufbxw_mesh mesh = ufbxw_create_mesh(scene);
	ufbxw_set_name(scene, mesh.id, "Cube_Shape");
	ufbxw_mesh_add_instance(scene, mesh, cube);

	ufbxw_vec3 red = { 1.0f, 0.0f, 0.0f };
	ufbxw_set_vec3(scene, mesh.id, "Color", red);
	ufbxw_set_int(scene, mesh.id, "Receive Shadows", 0);

	ufbxw_id scene_info = ufbxw_get_scene_info_id(scene);
	ufbxw_add_string(scene, scene_info, "CustomProperty", UFBXW_PROP_TYPE_STRING, "Hello world");

	ufbxw_id global_settings = ufbxw_get_global_settings_id(scene);
	ufbxw_add_string(scene, global_settings, "Something", UFBXW_PROP_TYPE_STRING, "Good Stuff");

	ufbxw_id node_template = ufbxw_get_template_id(scene, UFBXW_ELEMENT_NODE);
	ufbxw_vec3 tmpl_pos = { 1.0f, 2.0f, 3.0f };
	ufbxw_set_vec3(scene, node_template, "Lcl Translation", tmpl_pos);

	ufbxw_id material = ufbxw_create_element_ex(scene, UFBXW_ELEMENT_MATERIAL, "FbxSurfaceLambert");
	ufbxw_set_name(scene, material, "lambert1");

	ufbxw_vec3 green = { 0.0f, 1.0f, 0.0f };
	ufbxw_set_vec3(scene, material, "DiffuseColor", green);

	ufbxw_connect(scene, material, mesh.id);
#endif

	ufbxw_save_opts opts = { 0 };
	opts.ascii = true;
	opts.debug_comments = true;

	ufbxw_error error;
	ufbxw_save_file(scene, "test.fbx", &opts, &error);

	ufbxw_free_scene(scene);
}
