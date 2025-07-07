#include "../ufbx_write.h"

static ufbxw_vec3 make_vec3(ufbxw_real x, ufbxw_real y, ufbxw_real z)
{
	ufbxw_vec3 v = { x, y, z };
	return v;
}

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

	ufbxw_mesh_add_instance(scene, mesh, node);

	ufbxw_id material = ufbxw_create_element_ex(scene, UFBXW_ELEMENT_MATERIAL, "FbxSurfaceLambert");
	ufbxw_set_name(scene, material, "lambert1");
	ufbxw_set_vec3(scene, material, "DiffuseColor", make_vec3(0.2, 0.4, 0.6));

	ufbxw_id texture = ufbxw_create_element(scene, UFBXW_ELEMENT_FILE_TEXTURE);
	ufbxw_set_name(scene, texture, "texture");
	ufbxw_set_string(scene, texture, "Path", "C:/Files/texture.png");
	ufbxw_set_string(scene, texture, "RelPath", "texture.png");

	ufbxw_connect_prop(scene, texture, "", material, "DiffuseColor");

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
