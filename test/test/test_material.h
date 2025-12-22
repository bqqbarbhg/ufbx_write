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

	ufbxw_material material = ufbxw_create_material(scene);
}
#endif

UFBXWT_SCENE_CHECK(material_implementation)
#if UFBXWT_IMPL
{
}
#endif

