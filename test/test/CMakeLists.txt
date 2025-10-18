add_executable(ufbxw_test)

file(GLOB SRC_TEST *.c *.h)

target_sources(ufbxw_test PRIVATE ${SRC_TEST})
