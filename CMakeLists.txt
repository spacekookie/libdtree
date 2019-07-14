cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c11")
set(CMAKE_BUILD_TYPE Debug)
set(BUILD_EXAMPLES 0 CACHE BOOL "Build the included examples projects")

project(bowl)
add_library(bowl SHARED array.c
                        bowl.c
                        data.c
                        hash.c
                        utils.c)

target_include_directories(bowl PUBLIC ".")

################### EXAMPLES ###################

if(BUILD_EXAMPLES)
    add_executable(example_tree examples/tree.c)
    target_link_libraries(example_tree bowl)

    add_executable(example_list examples/list.c)
    target_link_libraries(example_list bowl)

    add_executable(example_hash examples/hash.c)
    target_link_libraries(example_hash bowl)
endif()
