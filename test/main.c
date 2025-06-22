#include "../ufbx_write.h"

int main(int argc, char **argv)
{
	ufbxw_scene *scene = ufbxw_create_scene(NULL);

	ufbxw_node parent = ufbxw_create_node(scene);
	ufbxw_set_name(scene, parent.id, "Parent");

	ufbxw_node cube = ufbxw_create_node(scene);
	ufbxw_set_name(scene, cube.id, "Cube");
	ufbxw_node_set_parent(scene, cube, parent);

	ufbxw_mesh mesh = ufbxw_create_mesh(scene);
	ufbxw_set_name(scene, mesh.id, "Cube_Shape");
	ufbxw_mesh_add_instance(scene, mesh, cube);

	ufbxw_save_opts opts = { 0 };
	ufbxw_error error;
	ufbxw_save_file(scene, "test.fbx", &opts, &error);

	ufbxw_free_scene(scene);
}
