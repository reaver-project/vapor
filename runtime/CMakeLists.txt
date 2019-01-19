file(GLOB_RECURSE sources *.ll)

foreach(abs_source ${sources})
    file(RELATIVE_PATH source "${CMAKE_CURRENT_SOURCE_DIR}" "${abs_source}")

    set(optimized "${CMAKE_BINARY_DIR}/runtime/${source}")
    set(object "${optimized}.o")

    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/runtime/${source}.o"
        COMMENT "Compiling ${source}..."
        COMMAND ${OPT} -O3 ${source} -o ${optimized} -S
        COMMAND ${LLC} -filetype=obj -relocation-model=pic ${optimized} -o ${object}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    list(APPEND objects ${object})
endforeach()

add_library(vpr-rt STATIC
    ${objects}
)
set_target_properties(vpr-rt
    PROPERTIES
    LINKER_LANGUAGE C
    OUTPUT_NAME vprrt
)
