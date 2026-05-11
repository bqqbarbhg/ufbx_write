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
	ufbxw_video video = ufbxw_create_video(scene);
	ufbxw_set_name(scene, video.id, "EmbeddedVideo");

	const char content[] = "Hello, world!";
	ufbxwt_assert(sizeof(content) - 1 == 13);

	ufbxw_byte_buffer content_buffer = ufbxw_copy_byte_array(scene, content, sizeof(content) - 1);
	ufbxw_video_set_content(scene, video, content_buffer);
}
#endif

UFBXWT_SCENE_CHECK(video_content)
#if UFBXWT_IMPL
{
	ufbx_video *video = (ufbx_video*)ufbx_find_element(scene, UFBX_ELEMENT_VIDEO, "EmbeddedVideo");
	ufbxwt_assert(video);

	ufbxwt_assert(video->content.size == 13);
	ufbxwt_assert(!memcmp(video->content.data, "Hello, world!", 13));
}
#endif

UFBXWT_SCENE_TEST(video_large_content)
#if UFBXWT_IMPL
{
	ufbxw_video video = ufbxw_create_video(scene);
	ufbxw_set_name(scene, video.id, "EmbeddedVideo");

	const size_t content_size = 8 * 1024;
	ufbxw_byte_buffer buffer = ufbxw_create_byte_buffer(scene, content_size);
	ufbxw_byte_list list = ufbxw_edit_byte_buffer(scene, buffer);
	for (size_t i = 0; i < content_size; i++) {
		list.data[i] = (char)((i * 0x9e3779b9u) >> 24);
	}

	ufbxw_video_set_content(scene, video, buffer);
}
#endif

UFBXWT_SCENE_CHECK(video_large_content)
#if UFBXWT_IMPL
{
	ufbx_video *video = (ufbx_video*)ufbx_find_element(scene, UFBX_ELEMENT_VIDEO, "EmbeddedVideo");
	ufbxwt_assert(video);

	const size_t content_size = 8 * 1024;
	ufbxwt_assert(video->content.size == content_size);

	const char *content = video->content.data;
	for (size_t i = 0; i < content_size; i++) {
		ufbxwt_assert(content[i] == (char)((i * 0x9e3779b9) >> 24));
	}
}
#endif

#if UFBXWT_IMPL
static size_t ufbxwt_video_content_stream(void *user, void *dst, size_t dst_size, size_t offset)
{
	char *d = (char*)dst;
	for (size_t i = 0; i < dst_size; i++) {
		d[i] = (char)(((offset + i) * 0x9e3779b9u) >> 24);
	}
	return dst_size;
}
#endif

UFBXWT_SCENE_TEST(video_stream_content)
#if UFBXWT_IMPL
{
	ufbxw_video video = ufbxw_create_video(scene);
	ufbxw_set_name(scene, video.id, "EmbeddedVideo");

	const size_t content_size = 8 * 1024;
	ufbxw_byte_buffer buffer = ufbxw_external_byte_stream(scene, &ufbxwt_video_content_stream, NULL, content_size);

	ufbxw_video_set_content(scene, video, buffer);
}
#endif

UFBXWT_SCENE_CHECK(video_stream_content)
#if UFBXWT_IMPL
{
	ufbx_video *video = (ufbx_video*)ufbx_find_element(scene, UFBX_ELEMENT_VIDEO, "EmbeddedVideo");
	ufbxwt_assert(video);

	const size_t content_size = 8 * 1024;
	ufbxwt_assert(video->content.size == content_size);

	const char *content = video->content.data;
	for (size_t i = 0; i < content_size; i++) {
		ufbxwt_assert(content[i] == (char)((i * 0x9e3779b9) >> 24));
	}
}
#endif

UFBXWT_SCENE_TEST(texture_file_content)
#if UFBXWT_IMPL
{
	ufbxw_texture texture = ufbxw_create_texture(scene, UFBXW_TEXTURE_FILE);
	ufbxw_set_name(scene, texture.id, "Texture");
	ufbxw_texture_set_filename(scene, texture, "/data/texture.png");
	ufbxw_texture_set_relative_filename(scene, texture, "texture.png");

	const char content[] = "Hello, world!";
	ufbxwt_assert(sizeof(content) - 1 == 13);

	ufbxw_byte_buffer content_buffer = ufbxw_copy_byte_array(scene, content, sizeof(content) - 1);
	ufbxw_texture_set_content(scene, texture, content_buffer);
}
#endif

UFBXWT_SCENE_CHECK(texture_file_content)
#if UFBXWT_IMPL
{
	ufbx_texture *texture = (ufbx_texture*)ufbx_find_element(scene, UFBX_ELEMENT_TEXTURE, "Texture");
	ufbxwt_assert(texture);
	ufbxwt_assert(!strcmp(texture->absolute_filename.data, "/data/texture.png"));
	ufbxwt_assert(!strcmp(texture->relative_filename.data, "texture.png"));
	ufbxwt_assert(texture->content.size == 13);
	ufbxwt_assert(!memcmp(texture->content.data, "Hello, world!", 13));

	ufbx_video *video = texture->video;
	ufbxwt_assert(video);
	ufbxwt_assert(!strcmp(video->absolute_filename.data, "/data/texture.png"));
	ufbxwt_assert(!strcmp(video->relative_filename.data, "texture.png"));
	ufbxwt_assert(video->content.size == 13);
	ufbxwt_assert(!memcmp(video->content.data, "Hello, world!", 13));
}
#endif
