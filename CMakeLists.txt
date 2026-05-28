cmake_minimum_required(VERSION 3.20)
project(concurrency_patterns_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Pick a sanitizer with -DSAN=tsan or -DSAN=asan. Default: none.
set(SAN "" CACHE STRING "Sanitizer to enable (tsan, asan, ubsan, or empty)")
if(SAN STREQUAL "tsan")
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer -g -O1)
    add_link_options(-fsanitize=thread)
elseif(SAN STREQUAL "asan")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g -O1)
    add_link_options(-fsanitize=address)
elseif(SAN STREQUAL "ubsan")
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer -g -O1)
    add_link_options(-fsanitize=undefined)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

# Modules added one at a time as they land.

add_subdirectory(race-conditions)
add_subdirectory(producer-consumer)
add_subdirectory(dining-philosophers)
