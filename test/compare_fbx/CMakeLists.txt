add_executable(compare_fbx)

file(GLOB SRC_COMPARE_FBX *.c *.cpp *.h)

target_link_libraries(compare_fbx PRIVATE ufbx ufbxwt_util)
target_sources(compare_fbx PRIVATE ${SRC_COMPARE_FBX})

