#undef UFBXWT_TEST_GROUP
#define UFBXWT_TEST_GROUP "animation"

#if UFBXWT_IMPL
typedef struct {
	double time;
	ufbxw_vec3 value;
	uint32_t type;
} ufbxwt_keyframe_vec3;

static void ufbxwt_add_keyframes_vec3(ufbxw_scene *scene, ufbxw_anim_prop anim, const ufbxwt_keyframe_vec3 *keys, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		ufbxw_ktime ktime = (ufbxw_ktime)(keys[i].time * UFBXW_KTIME_SECOND);
		ufbxw_anim_add_keyframe_vec3(scene, anim, ktime, keys[i].value, keys[i].type);
	}
}

static const ufbxwt_keyframe_vec3 ufbxwt_simple_keys[] = {
	{ 0.0, { 0.0f, 0.0f, 0.0f }, UFBXW_KEYFRAME_CUBIC_AUTO },
	{ 0.5, { 1.0f, 0.0f, 0.0f }, UFBXW_KEYFRAME_CUBIC_AUTO },
	{ 1.0, { -1.0f, 0.0f, 1.0f }, UFBXW_KEYFRAME_CUBIC_AUTO },
};

#endif

UFBXWT_SCENE_TEST(anim_simple)
#if UFBXWT_IMPL
{
	ufbxw_node node = ufbxwt_create_node(scene, "Node");

	ufbxw_anim_layer anim_layer = ufbxw_get_default_anim_layer(scene);
	ufbxw_anim_prop anim = ufbxw_node_animate_translation(scene, node, anim_layer);

	ufbxwt_add_keyframes_vec3(scene, anim, ufbxwt_simple_keys, ufbxwt_arraycount(ufbxwt_simple_keys));
}
#endif

UFBXWT_SCENE_CHECK(anim_simple)
#if UFBXWT_IMPL
{
	ufbx_node *node = ufbx_find_node(scene, "Node");
	ufbxwt_assert(node);

	for (size_t i = 0; i < ufbxwt_arraycount(ufbxwt_simple_keys); i++) {
		ufbxwt_keyframe_vec3 key = ufbxwt_simple_keys[i];
		ufbx_transform transform = ufbx_evaluate_transform(scene->anim, node, key.time);

		ufbx_vec3 value = { key.value.x, key.value.y, key.value.z };
		ufbxwt_assert_close_uvec3(err, transform.translation, value);
	}
}
#endif


