#undef UFBXT_TEST_GROUP
#define UFBXT_TEST_GROUP "basic"

UFBXWT_TEST(create_scene)
#if UFBXWT_IMPL
{
	ufbxw_scene *scene = ufbxw_create_scene(NULL);
	ufbxwt_assert(scene);
	ufbxw_free_scene(scene);
}
#endif

