cmake_minimum_required(VERSION 3.5)

project(Vapor CXX)

find_package(Threads REQUIRED)
find_package(Boost 1.64 REQUIRED
    COMPONENTS system program_options filesystem
)
include_directories(${Boost_INCLUDE_DIRS})

include(FindPkgConfig)

pkg_check_modules(SODIUM libsodium>=1.0)
include_directories(${SODIUM_INCLUDEDIR})
link_directories(${SODIUM_LIBRARY_DIRS})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC")

add_subdirectory(vendor EXCLUDE_FROM_ALL)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wpedantic -Weffc++ -Werror -Wno-unused-parameter")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ferror-limit=1 -ftemplate-backtrace-limit=0 -fno-limit-debug-info")
endif()

include_directories(include/reaver)
include_directories(vendor/protobuf/src SYSTEM)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(tests EXCLUDE_FROM_ALL)
