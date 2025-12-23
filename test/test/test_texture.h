#undef UFBXWT_TEST_GROUP
#define UFBXWT_TEST_GROUP "texture"

UFBXWT_SCENE_TEST(texture_file_simple)
#if UFBXWT_IMPL
{
	ufbxw_texture texture = ufbxw_create_texture(scene, UFBXW_TEXTURE_FILE);
	ufbxw_set_name(scene, texture.id, "Texture");
	ufbxw_texture_set_filename(scene, texture, "/data/texture.png");
	ufbxw_texture_set_relative_filename(scene, texture, "texture.png");
}
#endif

UFBXWT_SCENE_CHECK(texture_file_simple)
#if UFBXWT_IMPL
{
	ufbx_texture *texture = (ufbx_texture*)ufbx_find_element(scene, UFBX_ELEMENT_TEXTURE, "Texture");
	ufbxwt_assert(texture);
	ufbxwt_assert(!strcmp(texture->absolute_filename.data, "/data/texture.png"));
	ufbxwt_assert(!strcmp(texture->relative_filename.data, "texture.png"));

	ufbx_video *video = texture->video;
	ufbxwt_assert(video);
	ufbxwt_assert(!strcmp(video->absolute_filename.data, "/data/texture.png"));
	ufbxwt_assert(!strcmp(video->relative_filename.data, "texture.png"));
}
#endif

UFBXWT_SCENE_TEST(video_content)
#if UFBXWT_IMPL
{
}
#endif

UFBXWT_SCENE_CHECK(video_content)
#if UFBXWT_IMPL
{
}
#endif

