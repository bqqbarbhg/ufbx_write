#undef UFBXWT_TEST_GROUP
#define UFBXWT_TEST_GROUP "bone"

UFBXWT_SCENE_TEST(simple_bone)
#if UFBXWT_IMPL
{
	ufbxw_node root = ufbxwt_create_node(scene, "Root");
	ufbxw_create_bone(scene, UFBXW_BONE_ROOT, root);

	ufbxw_node mid = ufbxwt_create_node(scene, "Mid");
	ufbxw_create_bone(scene, UFBXW_BONE_LIMB_NODE, mid);

	ufbxw_node top = ufbxwt_create_node(scene, "Top");
	ufbxw_create_bone(scene, UFBXW_BONE_LIMB_NODE, top);

	ufbxw_vec3 root_pos = { 0.0f, 0.0f, 0.0f };
	ufbxw_vec3 mid_pos = { 0.0f, 1.0f, 0.0f };
	ufbxw_vec3 top_pos = { 0.0f, 1.0f, 0.0f };

	ufbxw_node_set_translation(scene, root, root_pos);
	ufbxw_node_set_translation(scene, mid, mid_pos);
	ufbxw_node_set_translation(scene, top, top_pos);

	ufbxw_node_set_parent(scene, mid, root);
	ufbxw_node_set_parent(scene, top, mid);
}
#endif

UFBXWT_SCENE_CHECK(simple_bone)
#if UFBXWT_IMPL
{
	ufbx_node *root = ufbx_find_node(scene, "Root");
	ufbx_node *mid = ufbx_find_node(scene, "Mid");
	ufbx_node *top = ufbx_find_node(scene, "Top");

	ufbxwt_assert(root);
	ufbxwt_assert(mid);
	ufbxwt_assert(top);

	ufbxwt_assert(root->bone);
	ufbxwt_assert(mid->bone);
	ufbxwt_assert(top->bone);

	ufbxwt_assert(root->bone->is_root);
	ufbxwt_assert(!mid->bone->is_root);
	ufbxwt_assert(!top->bone->is_root);

	ufbxwt_assert(root->parent == scene->root_node);
	ufbxwt_assert(mid->parent == root);
	ufbxwt_assert(top->parent == mid);

	ufbx_vec3 root_pos = { 0.0f, 0.0f, 0.0f };
	ufbx_vec3 mid_pos = { 0.0f, 1.0f, 0.0f };
	ufbx_vec3 top_pos = { 0.0f, 1.0f, 0.0f };

	ufbxwt_assert_close_uvec3(err, root->local_transform.translation, root_pos);
	ufbxwt_assert_close_uvec3(err, mid->local_transform.translation, mid_pos);
	ufbxwt_assert_close_uvec3(err, top->local_transform.translation, top_pos);
}
#endif

