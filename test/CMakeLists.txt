cmake_minimum_required(VERSION 3.15)

project(ufbxw_tests)

include(FetchContent)

add_executable(ufbxw_test)

option(USE_LIBDEFLATE "Enable libdeflate" ON)
option(USE_ZLIB "Enable Zlib" ON)

set(SRC_EXTRA "")

if(USE_LIBDEFLATE)
    FetchContent_Declare(
        libdeflate
        GIT_REPOSITORY "https://github.com/ebiggers/libdeflate.git"
        EXCLUDE_FROM_ALL
    )

    set(LIBDEFLATE_BUILD_STATIC_LIB ON CACHE BOOL "" FORCE)
    set(LIBDEFLATE_BUILD_SHARED_LIB OFF CACHE BOOL "" FORCE)
    set(LIBDEFLATE_BUILD_GZIP OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(libdeflate)

    target_link_libraries(ufbxw_test PRIVATE libdeflate_static)
    target_compile_definitions(ufbxw_test PRIVATE UFBXWT_HAS_LIBDEFLATE=1)

    list(APPEND SRC_EXTRA "../extra/ufbxw_libdeflate.h")
endif()

if(USE_ZLIB)
    FetchContent_Declare(
        zlib
        GIT_REPOSITORY "https://github.com/madler/zlib.git"
        EXCLUDE_FROM_ALL
    )

    set(ZLIB_BUILD_STATIC ON CACHE BOOL "" FORCE)
    set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(ZLIB_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(ZLIB_BUILD_MINIZIP OFF CACHE BOOL "" FORCE)
    set(ZLIB_INSTALL OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(zlib)

    target_link_libraries(ufbxw_test PRIVATE zlibstatic)
    target_compile_definitions(ufbxw_test PRIVATE UFBXWT_HAS_ZLIB=1)

    list(APPEND SRC_EXTRA "../extra/ufbxw_zlib.h")
endif()

set(SRC_UFBX_WRITE
    ../ufbx_write.c
    ../ufbx_write.h
)

set(SRC_UFBX
    ufbx/ufbx.c
    ufbx/ufbx.h
)

file(GLOB SRC_TEST *.c *.h)

target_sources(ufbxw_test PRIVATE ${SRC_UFBX_WRITE})
target_sources(ufbxw_test PRIVATE ${SRC_UFBX})
target_sources(ufbxw_test PRIVATE ${SRC_TEST})
target_sources(ufbxw_test PRIVATE ${SRC_EXTRA})

source_group("ufbx_write" FILES ${SRC_UFBX_WRITE})
source_group("ufbx" FILES ${SRC_UFBX})
source_group("test" FILES ${SRC_TEST})
source_group("extra" FILES ${SRC_EXTRA})

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -Wno-format-truncation)
    target_link_libraries(ufbxw_test PRIVATE m)
endif()

target_compile_definitions(ufbxw_test PRIVATE UFBXW_DEV=1 UFBX_DEV=1)

get_filename_component(ROOT_PATH
                       "${CMAKE_CURRENT_LIST_DIR}/.."
                       ABSOLUTE)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ufbxw_test)
set_target_properties(ufbxw_test PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${ROOT_PATH}
)

file(MAKE_DIRECTORY "${ROOT_PATH}/output")
