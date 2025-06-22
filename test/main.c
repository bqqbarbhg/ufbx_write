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

#if 0
	ufbxw_writer *w = ufbxw_writer_create();

	ufbxw_node parent = ufbxw_create_node(w);
	ufbxw_node_set_name(w, parent, "Parent");

	ufbxw_node child = ufbxw_create_node(w);
	ufbxw_node_set_name(w, child, "Child");
	ufbxw_node_set_parent(w, child, parent);

	ufbxw_node_set_translation(w, child, vec3(1.0f, 2.0f, 3.0f));
	ufbxw_node_set_rotation_euler(w, child, vec3(90.0f, 0.0f, 0.0f));
	ufbxw_node_set_scale(w, child, vec3(0.25f, 0.5f, 1.0f));

	ufbxw_mesh mesh = ufbxw_create_mesh(w);
	ufbxw_mesh_add(w, mesh, child);

	const int32_t indices[] = { 0, 1, 2, 3 };
	const int32_t face_sizes[] = { 4 };

	ufbxw_mesh_set_indices(w, mesh,
		ufbxw_int_array(indices, 4),
		ufbxw_int_array(face_sizes, 1));

	ufbxw_free_writer(w);
#endif
}
