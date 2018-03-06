add_executable(vprc-exe
    main.cpp
    cli/cli.cpp
)

target_link_libraries(vprc-exe
    Threads::Threads
    ${Boost_LIBRARIES}
    vprc
)

set_target_properties(vprc-exe
    PROPERTIES OUTPUT_NAME vprc
)
