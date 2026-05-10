#undef UFBXWT_TEST_GROUP
#define UFBXWT_TEST_GROUP "binary"

UFBXWT_TEST(binary_footer)
#if UFBXWT_IMPL
{
	static const uint32_t versions[] = { 7400, 7500 };

	size_t result_size = 1024 * 1024;
	void *result = malloc(result_size);
	ufbxwt_assert(result);

	for (int version_ix = 0; version_ix < ufbxwt_arraycount(versions); version_ix++) {
		for (size_t name_len = 1; name_len <= 32; name_len++) {
			ufbxw_scene *scene = ufbxw_create_scene(NULL);
			ufbxwt_assert(scene);

			char name_buf[64];
			ufbxwt_assert(name_len <= sizeof(name_buf));
			memset(name_buf, 'A', name_len);

			ufbxw_node node = ufbxw_create_node(scene);
			ufbxw_set_name_len(scene, node.id, name_buf, name_len);

			ufbxw_prepare_scene(scene, NULL);

			ufbxw_save_opts save_opts = { 0 };
			save_opts.format = UFBXW_SAVE_FORMAT_BINARY;
			save_opts.version = versions[version_ix];
			save_opts.no_default_timestamp = true;
			save_opts.local_timestamp.year = 1970;
			save_opts.local_timestamp.month = 1;
			save_opts.local_timestamp.day = 1;
			save_opts.local_timestamp.hour = 10;

			ufbxwt_memory_stream stream = { result, result_size };

			ufbxw_write_stream ws = { 0 };
			ws.write_fn = &ufbxwt_memory_stream_write;
			ws.user = &stream;

			ufbxw_error save_error;
			bool save_ok = false;

			if (name_len == 1) {
				char output_path[256];
				snprintf(output_path, sizeof(output_path), "%s/%s_%u_%s.fbx", output_root, "binary_footer", save_opts.version, "binary");
				save_ok = ufbxw_save_file(scene, output_path, &save_opts, &save_error);
				if (save_error.type != UFBXW_ERROR_NONE) {
					ufbxwt_log_error(&save_error);
				}
				ufbxwt_assert(save_ok);
			}

			save_ok = ufbxw_save_stream(scene, &ws, &save_opts, &save_error);
			if (save_error.type != UFBXW_ERROR_NONE) {
				ufbxwt_log_error(&save_error);
			}
			ufbxwt_assert(save_ok);

			ufbx_error load_error;
			ufbx_scene *loaded_scene = ufbx_load_memory(result, result_size, NULL, &load_error);
			if (load_error.type != UFBX_ERROR_NONE) {
				ufbxwt_log_uerror(&load_error);
			}
			ufbxwt_assert(loaded_scene);

			ufbxwt_assert(loaded_scene->nodes.count == 2);
			ufbx_node *loaded_node = loaded_scene->nodes.data[1];
			ufbxwt_assert(loaded_node->name.length == name_len);

			ufbx_free_scene(loaded_scene);


			ufbxwt_assert(stream.written_size % 16 == 0);

			{
				char *data = (char*)stream.data;
				size_t file_size = stream.written_size;

				ufbxwt_assert(stream.written_size > 256);

				size_t footer_start = stream.written_size - 16;

				const char footer_magic[] = "\xfa\xbc\xab\x09\xd0\xc8\xd4\x66\xb1\x76\xfb\x83\x1c\xf7\x26\x7e";
				while (footer_start > 0 && memcmp((char*)stream.data + footer_start, footer_magic, 16))
					footer_start--;
				ufbxwt_assert(footer_start >= stream.written_size - 256);

				size_t footer_align = footer_start + 16;
				footer_align = footer_align + (16 - footer_align % 16);
				ufbxwt_assert(footer_align <= file_size);

				uint8_t footer[128];

				ufbxwt_assert(footer_align + sizeof(footer) <= file_size);
				memcpy(footer, data + footer_align, sizeof(footer));

				ufbxwt_assert(footer[0] == 0);
				ufbxwt_assert(footer[1] == 0);
				ufbxwt_assert(footer[2] == 0);
				ufbxwt_assert(footer[3] == 0);
				ufbxwt_assert(footer[4] == (save_opts.version & 0xff));
				ufbxwt_assert(footer[5] == ((save_opts.version >> 8) & 0xff));
				ufbxwt_assert(footer[6] == 0);
				ufbxwt_assert(footer[7] == 0);
				for (size_t i = 8; i < sizeof(footer); i++) {
					ufbxwt_assert(footer[i] == 0);
				}

				const char fixed_footer[] = "\xf8\x5a\x8c\x6a\xde\xf5\xd9\x7e\xec\xe9\x0c\xe3\x75\x8f\x29\x0b";

				size_t footer_end = footer_align + sizeof(footer);
				ufbxwt_assert(footer_end + 16 == file_size);
				ufbxwt_assert(!memcmp(data + footer_end, fixed_footer, 16));
			}

			ufbxw_free_scene(scene);
		}
	}

	free(result);
}
#endif
