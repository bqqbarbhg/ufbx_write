add_executable(ufbxwt_roundtrip)

file(GLOB SRC_ROUNDTRIP *.c *.h)

target_link_libraries(ufbxwt_roundtrip PRIVATE ufbx_write ufbx ufbxw_extra ufbxwt_util)
target_sources(ufbxwt_roundtrip PRIVATE ${SRC_ROUNDTRIP})
