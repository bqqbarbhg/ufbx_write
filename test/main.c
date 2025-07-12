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
	ufbxw_node parent = ufbxw_create_node(scene);
	ufbxw_set_name(scene, parent.id, "Parent");

	ufbxw_node node = ufbxw_create_node(scene);
	ufbxw_set_name(scene, node.id, "Node");

	ufbxw_connect(scene, node.id, parent.id);

	ufbxw_vec3 pos = { 1.0f, 2.0f, 3.0f };

	ufbxw_node_set_translation(scene, node, pos);

	ufbxw_mesh mesh = ufbxw_create_mesh(scene);
	ufbxw_set_name(scene, mesh.id, "Cube");

	vector3_t vertices[] = {
		{ -1.0f, 0.0f, -1.0f, },
		{ +1.0f, 0.0f, -1.0f, },
		{ -1.0f, 0.0f, +1.0f, },
		{ +1.0f, 0.0f, +1.0f, },
	};
	int32_t indices[] = {
		0, 1, 3, 2,
	};
	int32_t face_sizes[] = {
		4,
	};

	ufbxw_mesh_set_vertices(scene, mesh,
		ufbxw_float3_array(&vertices[0].x, array_count(vertices)));

	ufbxw_mesh_set_polygons(scene, mesh,
		ufbxw_int32_array(indices, array_count(indices)),
		ufbxw_int32_array(face_sizes, array_count(face_sizes)));

	ufbxw_mesh_add_instance(scene, mesh, node);

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
