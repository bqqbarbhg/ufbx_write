#include "../ufbx_write.h"

int main(int argc, char **argv)
{
	ufbxw_scene *scene = ufbxw_create_scene(NULL);

	ufbxw_node cube = ufbxw_create_node(scene);
	ufbxw_set_name(scene, cube.id, "Cube");

	ufbxw_string name = ufbxw_get_name(scene, cube.id);

	ufbxw_save_opts opts = { 0 };
	ufbxw_error error;
	ufbxw_save_file(scene, "test.fbx", &opts, &error);

	ufbxw_free_scene(scene);
}
