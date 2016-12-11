############################################
#
# libdyntree uses cmake because it's simple
#   Build instructions can be found in the README
#
############################################

# Set some flags
cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c11")
set(CMAKE_BUILD_TYPE Debug)

# Create our project for further reference
project(libdyntree)
set(DYN_TREE_SRC
        lib/dtree.c
        lib/eztree.c
        lib/dtree_utils.c

        # External files compiled in
        lib/jsmn.c)

# Define our library in cmake
add_library(libdyntree SHARED ${DYN_TREE_SRC})

# Include the subdirectories to search for headers
target_include_directories(libdyntree PUBLIC "include")
target_include_directories(libdyntree PRIVATE "lib")

# since the name starts with 'lib' dont add it again
set_target_properties(libdyntree PROPERTIES PREFIX "")

################### TESTING CODE BELOW ###################

set(TEST_SRC test/main.c)
add_executable(dyntree_test ${TEST_SRC})

# Library dependencies for the http extention
target_link_libraries(dyntree_test libdyntree)