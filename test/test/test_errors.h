#undef UFBXWT_TEST_GROUP
#define UFBXWT_TEST_GROUP "errors"

#if UFBXWT_IMPL
static void ufbxwt_capture_error(void *user, const ufbxw_error *error)
{
	ufbxw_error *dst = (ufbxw_error*)user;
	ufbxwt_assert(dst->type == UFBXW_ERROR_NONE);
	*dst = *error;
}

static void ufbxwt_reset_error(ufbxw_error *error)
{
	memset(error, 0, sizeof(ufbxw_error));
}
#endif

UFBXWT_TEST(node_not_found)
#if UFBXWT_IMPL
{
	ufbxw_scene *scene = ufbxw_create_scene(NULL);
	ufbxwt_assert(scene);

	ufbxw_error error = { UFBXW_ERROR_NONE };
	ufbxw_set_error_callback(scene, &ufbxwt_capture_error, &error);

	ufbxw_node node = ufbxwt_create_node(scene, "Node");

	ufbxw_string name = ufbxw_get_name(scene, node.id);
	ufbxwt_assert_string(name, "Node");

	ufbxw_delete_element(scene, node.id);

	ufbxwt_assert(error.type == UFBXW_ERROR_NONE);

	ufbxw_string del_name = ufbxw_get_name(scene, node.id);
	ufbxwt_assert_string(del_name, "");

	ufbxwt_assert_error(&error, UFBXW_ERROR_ELEMENT_NOT_FOUND, "ufbxw_get_name", "element not found");

	ufbxw_free_scene(scene);
}
#endif

UFBXWT_TEST(node_wrong_type)
#if UFBXWT_IMPL
{
	ufbxw_scene *scene = ufbxw_create_scene(NULL);
	ufbxwt_assert(scene);

	ufbxw_error error = { UFBXW_ERROR_NONE };
	ufbxw_set_error_callback(scene, &ufbxwt_capture_error, &error);

	ufbxw_mesh mesh = ufbxw_create_mesh(scene);

	ufbxw_node fake_node = { mesh.id };

	ufbxwt_assert(error.type == UFBXW_ERROR_NONE);

	ufbxw_vec3 translation = { 1.0f, 2.0f, 3.0f };
	ufbxw_node_set_translation(scene, fake_node, translation);

	ufbxwt_assert_error(&error, UFBXW_ERROR_ELEMENT_WRONG_TYPE, "ufbxw_node_set_translation", "wrong type: mesh");

	ufbxw_free_scene(scene);
}
#endif

UFBXWT_TEST(error_out_of_bounds)
#if UFBXWT_IMPL
{
	ufbxw_scene *scene = ufbxw_create_scene(NULL);
	ufbxwt_assert(scene);

	ufbxw_error error = { UFBXW_ERROR_NONE };
	ufbxw_set_error_callback(scene, &ufbxwt_capture_error, &error);

	ufbxw_node parent = ufbxwt_create_node(scene, "Parent");
	ufbxw_node child = ufbxwt_create_node(scene, "Child");
	ufbxw_node_set_parent(scene, child, parent);

	ufbxwt_assert(ufbxw_node_get_num_children(scene, parent) == 1);
	ufbxwt_assert(ufbxw_node_get_num_children(scene, child) == 0);

	ufbxw_node ref_child = ufbxw_node_get_child(scene, parent, 0);
	ufbxwt_assert(ref_child.id == child.id);

	ufbxwt_assert(error.type == UFBXW_ERROR_NONE);

	ufbxw_node bad_child = ufbxw_node_get_child(scene, parent, 1);
	ufbxwt_assert(bad_child.id == 0);

	ufbxwt_assert_error(&error, UFBXW_ERROR_INDEX_OUT_OF_BOUNDS, "ufbxw_node_get_child", "index (1) out of bounds (1)");

	ufbxw_free_scene(scene);
}
#endif

UFBXWT_TEST(error_buffer_edit)
#if UFBXWT_IMPL
{
	ufbxw_scene *scene = ufbxw_create_scene(NULL);
	ufbxwt_assert(scene);

	ufbxw_error error = { UFBXW_ERROR_NONE };
	ufbxw_set_error_callback(scene, &ufbxwt_capture_error, &error);

	ufbxw_int_buffer buffer = ufbxw_create_int_buffer(scene, 16);

	{
		ufbxw_int_list list = ufbxw_edit_int_buffer(scene, buffer);
		ufbxwt_assert(list.count == 16);
	}

	{
		ufbxw_long_buffer long_buffer = { buffer.id };
		ufbxw_long_list long_list = ufbxw_edit_long_buffer(scene, long_buffer);
		ufbxwt_assert(long_list.count == 0);
		ufbxwt_assert_error(&error, UFBXW_ERROR_BUFFER_WRONG_TYPE, "ufbxwi_edit_buffer", "wrong buffer type: int (expected long)");
	}

	ufbxw_free_buffer(scene, buffer.id);

	ufbxwt_reset_error(&error);

	{
		ufbxw_int_list list = ufbxw_edit_int_buffer(scene, buffer);
		ufbxwt_assert(list.count == 0);
		ufbxwt_assert_error(&error, UFBXW_ERROR_BUFFER_NOT_FOUND, "ufbxwi_edit_buffer", "buffer not found");
	}

	ufbxw_real data[4] = { 0 };
	ufbxw_real_buffer ext_buffer = ufbxw_external_real_array(scene, data, 4);

	ufbxwt_reset_error(&error);

	{
		ufbxw_real_list list = ufbxw_edit_real_buffer(scene, ext_buffer);
		ufbxwt_assert(list.count == 0);
		ufbxwt_assert_error(&error, UFBXW_ERROR_BUFFER_NOT_EDITABLE, "ufbxwi_edit_buffer", "buffer not editable");
	}

	ufbxwt_reset_error(&error);

	{
		ufbxw_int_buffer bad_id_buffer = { UINT64_MAX };
		ufbxw_int_list list = ufbxw_edit_int_buffer(scene, bad_id_buffer);
		ufbxwt_assert(list.count == 0);
		ufbxwt_assert_error(&error, UFBXW_ERROR_BUFFER_NOT_FOUND, "ufbxwi_edit_buffer", "buffer not found");
	}

	ufbxw_free_scene(scene);
}
#endif
