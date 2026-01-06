#undef UFBXWT_TEST_GROUP
#define UFBXWT_TEST_GROUP "cache"

UFBXWT_SCENE_TEST(cache_simple)
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

	ufbxw_cache_file cache = ufbxw_create_cache_file(scene);
	ufbxw_set_name(scene, cache.id, "TestCache");

	ufbxw_cache_file_set_format(scene, cache, UFBXW_CACHE_FILE_FORMAT_MC);
	ufbxw_cache_file_set_filename(scene, cache, "C:/TestCache.xml");
	ufbxw_cache_file_set_relative_filename(scene, cache, "TestCache.xml");

	ufbxw_cache_deformer deformer = ufbxw_create_cache_deformer(scene, mesh);
	ufbxw_cache_deformer_set_channel_name(scene, deformer, "position");
	ufbxw_cache_deformer_set_cache_file(scene, deformer, cache);
}
#endif

UFBXWT_SCENE_CHECK(cache_simple)
#if UFBXWT_IMPL
{
	ufbx_node *node = ufbx_find_node(scene, "Node");
	ufbxwt_assert(node);

	ufbx_mesh *mesh = node->mesh;
	ufbxwt_assert(mesh);

	ufbxwt_assert(mesh->cache_deformers.count == 1);

	ufbx_cache_deformer *deformer = mesh->cache_deformers.data[0];
	ufbxwt_assert(!strcmp(deformer->channel.data, "position"));

	ufbx_cache_file *file = deformer->file;

	ufbxwt_assert(file);
	ufbxwt_assert(!strcmp(file->name.data, "TestCache"));
	ufbxwt_assert(!strcmp(file->absolute_filename.data, "C:/TestCache.xml"));
	ufbxwt_assert(!strcmp(file->relative_filename.data, "TestCache.xml"));
}
#endif
