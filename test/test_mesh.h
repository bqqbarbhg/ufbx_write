#undef UFBXWT_TEST_GROUP
#define UFBXWT_TEST_GROUP "mesh"

UFBXWT_SCENE_TEST(mesh_plane)
#if UFBXWT_IMPL
{
	ufbxw_node node = ufbxwt_create_node(scene, "Node");
	ufbxw_mesh mesh = ufbxw_create_mesh(scene);

	ufbxw_mesh_add_instance(scene, mesh, node);

	ufbxw_vec3 vertices[] = {
		{ -1.0f, 0.0f, -1.0f },
		{ +1.0f, 0.0f, -1.0f },
		{ -1.0f, 0.0f, +1.0f },
		{ +1.0f, 0.0f, +1.0f },
	};
	int32_t indices[] = {
		0, 2, 3, 1,
	};
	int32_t face_offsets[] = {
		0, 4,
	};

	ufbxw_vec3_buffer vertex_buffer = ufbxw_view_vec3_array(scene, vertices, ufbxwt_arraycount(vertices));
	ufbxw_int_buffer index_buffer = ufbxw_view_int_array(scene, indices, ufbxwt_arraycount(indices));
	ufbxw_int_buffer face_buffer = ufbxw_view_int_array(scene, face_offsets, ufbxwt_arraycount(face_offsets));

	ufbxw_mesh_set_vertices(scene, mesh, vertex_buffer);
	ufbxw_mesh_set_polygons(scene, mesh, index_buffer, face_buffer);
}
#endif

UFBXWT_SCENE_CHECK(mesh_plane)
#if UFBXWT_IMPL
{
	ufbx_node *node = ufbx_find_node(scene, "Node");
	ufbxwt_assert(node);

	ufbx_mesh *mesh = node->mesh;
	ufbxwt_assert(mesh);

	ufbxwt_assert(mesh->faces.count == 1);

	ufbx_face face = mesh->faces.data[0];
	ufbxwt_assert(face.index_begin == 0);
	ufbxwt_assert(face.num_indices == 4);

	ufbx_vec3 vertices[] = {
		{ -1.0f, 0.0f, -1.0f },
		{ -1.0f, 0.0f, +1.0f },
		{ +1.0f, 0.0f, +1.0f },
		{ +1.0f, 0.0f, -1.0f },
	};

	for (size_t i = 0; i < 4; i++) {
		ufbx_vec3 v = ufbx_get_vertex_vec3(&mesh->vertex_position, i);
		ufbxwt_assert_close_uvec3(&err, v, vertices[i]);
	}
}
#endif
