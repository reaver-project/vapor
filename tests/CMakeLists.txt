file(GLOB_RECURSE sources "*.cpp")

add_executable(tests
    ${sources}
)

target_link_libraries(tests
    Threads::Threads
    ${Boost_LIBRARIES}
    vprc-lib
)

include(ProcessorCount)
ProcessorCount(N)
if(N EQUAL 0)
    set(N 1)
endif()

add_custom_target(run-tests
    COMMAND $<TARGET_FILE:tests> -j ${N} -l 3 -e
    DEPENDS tests
)

add_custom_target(ci-tests
    COMMAND $<TARGET_FILE:tests> -j ${N} -l 3 -r teamcity
    DEPENDS tests
)
