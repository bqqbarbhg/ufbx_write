#undef UFBXWT_TEST_GROUP
#define UFBXWT_TEST_GROUP "basic"

UFBXWT_TEST(create_scene)
#if UFBXWT_IMPL
{
	ufbxw_scene *scene = ufbxw_create_scene(NULL);
	ufbxwt_assert(scene);

	ufbxw_free_scene(scene);
}
#endif

UFBXWT_SCENE_TEST(simple_node)
#if UFBXWT_IMPL
{
	ufbxw_node node = ufbxw_create_node(scene);
	ufbxw_set_name(scene, node.id, "Test_Node");

	ufbxw_vec3 translation = { 1.0f, 2.0f, 3.0f };
	ufbxw_node_set_translation(scene, node, translation);

	ufbxw_vec3 field_translation = ufbxw_node_get_translation(scene, node);
	ufbxw_vec3 prop_translation = ufbxw_get_vec3(scene, node.id, "Lcl Translation");

	ufbxwt_assert(ufbxwt_equal_vec3(field_translation, translation));
	ufbxwt_assert(ufbxwt_equal_vec3(prop_translation, translation));
}
#endif

UFBXWT_SCENE_CHECK(simple_node)
#if UFBXWT_IMPL
{
	ufbx_node *node = ufbx_find_node(scene, "Test_Node");
	ufbxwt_assert(node);

	ufbx_vec3 translation = { 1.0f, 2.0f, 3.0f };
	ufbxwt_assert_close_uvec3(err, node->local_transform.translation, translation);
}
#endif
