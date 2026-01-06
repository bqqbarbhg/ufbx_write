#undef UFBXWT_TEST_GROUP
#define UFBXWT_TEST_GROUP "collection"

UFBXWT_SCENE_TEST(selection_set_simple)
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
	int32_t edges[] = {
		0, 1, 2, 3,
	};

	ufbxw_vec3_buffer vertex_buffer = ufbxw_view_vec3_array(scene, vertices, ufbxwt_arraycount(vertices));
	ufbxw_int_buffer index_buffer = ufbxw_view_int_array(scene, indices, ufbxwt_arraycount(indices));
	ufbxw_int_buffer face_buffer = ufbxw_view_int_array(scene, face_offsets, ufbxwt_arraycount(face_offsets));
	ufbxw_int_buffer edge_buffer = ufbxw_view_int_array(scene, edges, ufbxwt_arraycount(edges));

	ufbxw_mesh_set_vertices(scene, mesh, vertex_buffer);
	ufbxw_mesh_set_polygons(scene, mesh, index_buffer, face_buffer);
	ufbxw_mesh_set_fbx_edges(scene, mesh, edge_buffer);

	{
		ufbxw_selection_set set = ufbxw_create_selection_set(scene);

		ufbxw_selection_node sel = ufbxw_create_selection_node(scene);
		ufbxw_selection_set_add_node(scene, set, sel);

		ufbxw_set_name(scene, set.id, "Node");
		ufbxw_set_name(scene, sel.id, "Node");

		ufbxw_selection_node_set_node(scene, sel, node);
		ufbxw_selection_node_set_include_node(scene, sel, true);
	}

	{
		ufbxw_selection_set set = ufbxw_create_selection_set(scene);

		ufbxw_selection_node sel = ufbxw_create_selection_node(scene);
		ufbxw_selection_set_add_node(scene, set, sel);

		ufbxw_set_name(scene, set.id, "Vertices");
		ufbxw_set_name(scene, sel.id, "Vertices");

		ufbxw_selection_node_set_node(scene, sel, node);

		const int32_t vertices[] = { 0, 1 };
		ufbxw_int_buffer buf = ufbxw_view_int_array(scene, vertices, ufbxwt_arraycount(vertices));
		ufbxw_selection_node_set_vertices(scene, sel, buf);
	}

	{
		ufbxw_selection_set set = ufbxw_create_selection_set(scene);

		ufbxw_selection_node sel = ufbxw_create_selection_node(scene);
		ufbxw_selection_set_add_node(scene, set, sel);

		ufbxw_set_name(scene, set.id, "Edges");
		ufbxw_set_name(scene, sel.id, "Edges");

		ufbxw_selection_node_set_node(scene, sel, node);

		const int32_t edges[] = { 0, 2 };
		ufbxw_int_buffer buf = ufbxw_view_int_array(scene, edges, ufbxwt_arraycount(edges));
		ufbxw_selection_node_set_edges(scene, sel, buf);
	}

	{
		ufbxw_selection_set set = ufbxw_create_selection_set(scene);

		ufbxw_selection_node sel = ufbxw_create_selection_node(scene);
		ufbxw_selection_set_add_node(scene, set, sel);

		ufbxw_set_name(scene, set.id, "Polygons");
		ufbxw_set_name(scene, sel.id, "Polygons");

		ufbxw_selection_node_set_node(scene, sel, node);

		const int32_t polygons[] = { 0 };
		ufbxw_int_buffer buf = ufbxw_view_int_array(scene, polygons, ufbxwt_arraycount(polygons));
		ufbxw_selection_node_set_polygons(scene, sel, buf);
	}
}
#endif

UFBXWT_SCENE_CHECK(selection_set_simple)
#if UFBXWT_IMPL
{
	ufbx_node *node = ufbx_find_node(scene, "Node");
	ufbxwt_assert(node);

	{
		ufbx_selection_set *set = ufbx_find_element(scene, UFBX_ELEMENT_SELECTION_SET, "Node");
		ufbxwt_assert(set);
		ufbxwt_assert(set->nodes.count == 1);

		ufbx_selection_node *sel = set->nodes.data[0];
		ufbxwt_assert(sel->target_node == node);
		ufbxwt_assert(sel->include_node);
	}

	{
		ufbx_selection_set *set = ufbx_find_element(scene, UFBX_ELEMENT_SELECTION_SET, "Vertices");
		ufbxwt_assert(set);
		ufbxwt_assert(set->nodes.count == 1);

		ufbx_selection_node *sel = set->nodes.data[0];
		ufbxwt_assert(sel->target_node == node);
		ufbxwt_assert(!sel->include_node);
		ufbxwt_assert(sel->vertices.count == 2);
		ufbxwt_assert(sel->vertices.data[0] == 0);
		ufbxwt_assert(sel->vertices.data[1] == 1);
		ufbxwt_assert(sel->edges.count == 0);
		ufbxwt_assert(sel->faces.count == 0);
	}

	{
		ufbx_selection_set *set = ufbx_find_element(scene, UFBX_ELEMENT_SELECTION_SET, "Edges");
		ufbxwt_assert(set);
		ufbxwt_assert(set->nodes.count == 1);

		ufbx_selection_node *sel = set->nodes.data[0];
		ufbxwt_assert(sel->target_node == node);
		ufbxwt_assert(!sel->include_node);
		ufbxwt_assert(sel->edges.count == 2);
		ufbxwt_assert(sel->edges.data[0] == 0);
		ufbxwt_assert(sel->edges.data[1] == 2);
		ufbxwt_assert(sel->vertices.count == 0);
		ufbxwt_assert(sel->faces.count == 0);
	}

	{
		ufbx_selection_set *set = ufbx_find_element(scene, UFBX_ELEMENT_SELECTION_SET, "Polygons");
		ufbxwt_assert(set);
		ufbxwt_assert(set->nodes.count == 1);

		ufbx_selection_node *sel = set->nodes.data[0];
		ufbxwt_assert(sel->target_node == node);
		ufbxwt_assert(!sel->include_node);
		ufbxwt_assert(sel->faces.count == 1);
		ufbxwt_assert(sel->faces.data[0] == 0);
		ufbxwt_assert(sel->vertices.count == 0);
		ufbxwt_assert(sel->edges.count == 0);
	}
}
#endif
