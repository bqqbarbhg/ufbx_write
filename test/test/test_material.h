#undef UFBXWT_TEST_GROUP
#define UFBXWT_TEST_GROUP "material"

UFBXWT_SCENE_TEST(material_implementation)
#if UFBXWT_IMPL
{
	ufbxw_binding_table table = ufbxw_create_binding_table(scene);
	ufbxw_binding_table_add_entry(scene, table, "ufbx|DiffuseColor", "DiffuseColor");
	ufbxw_binding_table_add_entry(scene, table, "ufbx|SpecularColor", "SpecularColor");

	ufbxw_implementation implementation = ufbxw_create_implementation(scene);
	ufbxw_implementation_set_binding_table(scene, implementation, table);

	ufbxw_material material = ufbxw_create_material(scene, UFBXW_MATERIAL_CUSTOM);
	ufbxw_set_name(scene, material.id, "UfbxMaterial");

	ufbxw_material_set_implementation(scene, material, implementation);

	ufbxw_vec3 diffuse = { 0.25, 0.5, 0.75 };
	ufbxw_add_vec3(scene, material.id, "ufbx|DiffuseColor", UFBXW_PROP_TYPE_COLOR, diffuse);

	ufbxw_vec3 specular = { 1.0, 0.0, 1.0 };
	ufbxw_add_vec3(scene, material.id, "ufbx|SpecularColor", UFBXW_PROP_TYPE_COLOR, specular);
}
#endif

UFBXWT_SCENE_CHECK(material_implementation)
#if UFBXWT_IMPL
{
	ufbx_material *material = ufbx_find_material(scene, "UfbxMaterial");
	ufbxwt_assert(material);

	ufbx_shader *shader = material->shader;
	ufbxwt_assert(shader);

	ufbx_string diffuse_prop = ufbx_find_shader_prop(shader, "DiffuseColor");
	ufbxwt_assert(!strcmp(diffuse_prop.data, "ufbx|DiffuseColor"));

	ufbx_string specular_prop = ufbx_find_shader_prop(shader, "SpecularColor");
	ufbxwt_assert(!strcmp(specular_prop.data, "ufbx|SpecularColor"));

	ufbx_vec3 diffuse_value = ufbx_find_vec3(&material->props, diffuse_prop.data, ufbx_zero_vec3);
	ufbx_vec3 specular_value = ufbx_find_vec3(&material->props, specular_prop.data, ufbx_zero_vec3);

	const ufbx_vec3 diffuse_ref = { 0.25, 0.5, 0.75 };
	const ufbx_vec3 specular_ref = { 1.0, 0.0, 1.0 };

	ufbxwt_assert_close_uvec3(err, diffuse_value, diffuse_ref);
	ufbxwt_assert_close_uvec3(err, specular_value, specular_ref);
}
#endif

UFBXWT_SCENE_TEST(mesh_material_single)
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
	ufbxw_mesh_set_single_material(scene, mesh, 0);

	ufbxw_material material = ufbxw_create_material(scene, UFBXW_MATERIAL_FBX_LAMBERT);
	ufbxw_set_name(scene, material.id, "Material");

	ufbxw_vec3 diffuse = { 1.0f, 0.5f, 0.25f };
	ufbxw_vec3 ambient = { 0.0f, 1.0f, 0.0f };
	ufbxw_set_vec3(scene, material.id, "DiffuseColor", diffuse);
	ufbxw_set_vec3(scene, material.id, "AmbientColor", ambient);

	ufbxw_node_set_material(scene, node, 0, material);
}
#endif

UFBXWT_SCENE_CHECK(mesh_material_single)
#if UFBXWT_IMPL
{
	ufbx_node *node = ufbx_find_node(scene, "Node");
	ufbxwt_assert(node);
	ufbxwt_assert(node->mesh);

	ufbxwt_assert(node->materials.count == 1);
	ufbx_material *material = node->materials.data[0];
	ufbxwt_assert(!strcmp(material->name.data, "Material"));

	ufbx_vec3 ref_diffuse = { 1.0f, 0.5f, 0.25f };
	ufbx_vec3 ref_ambient = { 0.0f, 1.0f, 0.0f };
	ufbxwt_assert_close_uvec3(err, material->fbx.diffuse_color.value_vec3, ref_diffuse);
	ufbxwt_assert_close_uvec3(err, material->fbx.ambient_color.value_vec3, ref_ambient);
}
#endif

UFBXWT_SCENE_TEST(mesh_material_dual)
#if UFBXWT_IMPL
{
	ufbxw_node node = ufbxwt_create_node(scene, "Node");
	ufbxw_mesh mesh = ufbxw_create_mesh(scene);

	ufbxw_mesh_add_instance(scene, mesh, node);

	ufbxw_vec3 vertices[] = {
		{ -1.0f, 0.0f, 0.0f },
		{ +1.0f, 0.0f, 0.0f },
		{ -1.0f, 0.0f, 2.0f },
		{ +1.0f, 0.0f, 2.0f },
		{ -1.0f, 0.0f, 4.0f },
		{ +1.0f, 0.0f, 4.0f },
	};
	int32_t indices[] = {
		0, 2, 3, 1, 2, 4, 5, 3,
	};
	int32_t face_offsets[] = {
		0, 4, 8,
	};
	int32_t material_indices[] = {
		0, 1,
	};

	ufbxw_vec3_buffer vertex_buffer = ufbxw_view_vec3_array(scene, vertices, ufbxwt_arraycount(vertices));
	ufbxw_int_buffer index_buffer = ufbxw_view_int_array(scene, indices, ufbxwt_arraycount(indices));
	ufbxw_int_buffer face_buffer = ufbxw_view_int_array(scene, face_offsets, ufbxwt_arraycount(face_offsets));
	ufbxw_int_buffer material_buffer = ufbxw_view_int_array(scene, material_indices, ufbxwt_arraycount(material_indices));

	ufbxw_mesh_set_vertices(scene, mesh, vertex_buffer);
	ufbxw_mesh_set_polygons(scene, mesh, index_buffer, face_buffer);
	ufbxw_mesh_set_face_material(scene, mesh, material_buffer);

	ufbxw_material material_red = ufbxw_create_material(scene, UFBXW_MATERIAL_FBX_LAMBERT);
	ufbxw_set_name(scene, material_red.id, "Red");

	ufbxw_vec3 diffuse_red = { 1.0f, 0.0f, 0.0f };
	ufbxw_set_vec3(scene, material_red.id, "DiffuseColor", diffuse_red);

	ufbxw_material material_blue = ufbxw_create_material(scene, UFBXW_MATERIAL_FBX_LAMBERT);
	ufbxw_set_name(scene, material_blue.id, "Blue");

	ufbxw_vec3 diffuse_blue = { 0.0f, 0.0f, 1.0f };
	ufbxw_set_vec3(scene, material_blue.id, "DiffuseColor", diffuse_blue);

	ufbxw_node_set_material(scene, node, 0, material_red);
	ufbxw_node_set_material(scene, node, 1, material_blue);
}
#endif

UFBXWT_SCENE_CHECK(mesh_material_dual)
#if UFBXWT_IMPL
{
	ufbx_node *node = ufbx_find_node(scene, "Node");
	ufbxwt_assert(node);

	ufbx_mesh *mesh = node->mesh;
	ufbxwt_assert(mesh);

	ufbxwt_assert(mesh->face_material.count == 2);
	ufbxwt_assert(mesh->face_material.data[0] == 0);
	ufbxwt_assert(mesh->face_material.data[1] == 1);

	ufbxwt_assert(node->materials.count == 2);
	ufbx_material *material_red = node->materials.data[0];
	ufbx_material *material_blue = node->materials.data[1];

	ufbxwt_assert(!strcmp(material_red->name.data, "Red"));
	ufbxwt_assert(!strcmp(material_blue->name.data, "Blue"));

	ufbx_vec3 diffuse_red = { 1.0f, 0.0f, 0.0f };
	ufbx_vec3 diffuse_blue = { 0.0f, 0.0f, 1.0f };
	ufbxwt_assert_close_uvec3(err, material_red->fbx.diffuse_color.value_vec3, diffuse_red);
	ufbxwt_assert_close_uvec3(err, material_blue->fbx.diffuse_color.value_vec3, diffuse_blue);

}
#endif
