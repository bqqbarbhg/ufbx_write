#include "../ufbx_write.h"

int main(int argc, char **argv)
{
	ufbxw_scene *scene = ufbxw_create_scene(NULL);

	ufbxw_id a = ufbxw_create_element(scene, UFBXW_ELEMENT_NODE);
	ufbxw_id b = ufbxw_create_element(scene, UFBXW_ELEMENT_NODE);

	ufbxw_delete_element(scene, a);

	ufbxw_id c = ufbxw_create_element(scene, UFBXW_ELEMENT_NODE);

	ufbxw_id elements[64];
	size_t count = ufbxw_get_elements(scene, elements, 64);

	ufbxw_save_opts opts = { 0 };
	ufbxw_error error;
	ufbxw_save_file(scene, "test.fbx", &opts, &error);

	ufbxw_free_scene(scene);
}
