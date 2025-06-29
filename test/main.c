#include "../ufbx_write.h"

int main(int argc, char **argv)
{
	ufbxw_scene *scene = ufbxw_create_scene(NULL);

	ufbxw_node parent = ufbxw_create_node(scene);
	ufbxw_set_name(scene, parent.id, "Parent");

	ufbxw_vec3 pos = { 1.0f, 2.0f, 3.0f };
	ufbxw_vec3 scale = { 2.0f, 2.0f, 2.0f };

	ufbxw_node_set_translation(scene, parent, pos);
	ufbxw_set_vec3(scene, parent.id, UFBXW_P_Lcl_Scaling, scale);

	ufbxw_vec3 ref_pos = ufbxw_get_vec3(scene, parent.id, UFBXW_P_Lcl_Translation);
	ufbxw_vec3 ref_scale = ufbxw_node_get_scaling(scene, parent);

	ufbxw_node cube = ufbxw_create_node(scene);
	ufbxw_set_name(scene, cube.id, "Cube");
	ufbxw_node_set_parent(scene, cube, parent);

	ufbxw_mesh mesh = ufbxw_create_mesh(scene);
	ufbxw_set_name(scene, mesh.id, "Cube_Shape");
	ufbxw_mesh_add_instance(scene, mesh, cube);

	ufbxw_id scene_info = ufbxw_get_scene_info_id(scene);
	ufbxw_prop custom_prop = ufbxw_get_custom_prop_c(scene, "CustomProperty", "KString", "");
	ufbxw_set_string(scene, scene_info, custom_prop, "Hello world");

	ufbxw_save_opts opts = { 0 };
	opts.ascii = true;

	ufbxw_error error;
	ufbxw_save_file(scene, "test.fbx", &opts, &error);

	ufbxw_free_scene(scene);
}
