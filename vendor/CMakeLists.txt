set(protobuf_BUILD_TESTS FALSE CACHE BOOL "")
set(protobuf_BUILD_EXAMPLES FALSE CACHE BOOL "")
set(protobuf_SHARED_OR_STATIC STATIC CACHE STRING "")
add_subdirectory(protobuf/cmake)
