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
endforeach()

add_custom_target(proto
    DEPENDS ${proto_sources}
)

set_source_files_properties(${proto_sources}
    PROPERTIES GENERATED TRUE
)

include_directories("${CMAKE_BINARY_DIR}/proto")

add_library(vprc SHARED
    ${sources}
    ${proto_sources}
)

target_link_libraries(vprc
    $<TARGET_FILE:libprotobuf>
)
