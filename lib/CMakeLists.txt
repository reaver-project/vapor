file(GLOB_RECURSE sources "*.cpp")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/proto")

file(GLOB_RECURSE proto_files "proto/*.proto")
foreach(abs_proto_file ${proto_files})
    file(RELATIVE_PATH proto_file "${CMAKE_CURRENT_SOURCE_DIR}" "${abs_proto_file}")
    string(REGEX REPLACE "\\.[^.]*$" "" base "${proto_file}")
    add_custom_command(
        OUTPUT
            "${CMAKE_BINARY_DIR}/${base}.pb.cc"
            "${CMAKE_BINARY_DIR}/${base}.pb.h"
        COMMENT "Compiling ${base}.proto..."
        COMMAND $<TARGET_FILE:protoc> -I proto --cpp_out=${CMAKE_BINARY_DIR}/proto  "${proto_file}"
        DEPENDS protoc "${abs_proto_file}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    list(APPEND proto_sources "${CMAKE_BINARY_DIR}/${base}.pb.cc")
    list(APPEND proto_headers "${CMAKE_BINARY_DIR}/${base}.pb.h")
endforeach()

add_custom_target(proto
    DEPENDS ${proto_sources}
)

add_custom_target(clean-proto
    COMMAND rm -rf ${proto_sources} ${proto_headers}
)

set_source_files_properties(${proto_sources}
    PROPERTIES GENERATED TRUE
)

include_directories("${CMAKE_BINARY_DIR}/proto")

add_library(vprc-lib SHARED
    ${sources}
    ${proto_sources}
)

target_link_libraries(vprc-lib
    $<TARGET_FILE:libprotobuf>
    ${SODIUM_LIBRARIES}
)

set_target_properties(vprc-lib
    PROPERTIES OUTPUT_NAME vprc
)
