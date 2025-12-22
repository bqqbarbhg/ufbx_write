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

