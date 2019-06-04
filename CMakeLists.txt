cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c11")
set(CMAKE_BUILD_TYPE Debug)

project(bowl)
add_library(bowl SHARED bowl.c)

target_include_directories(bowl PUBLIC ".")

################### TESTING CODE BELOW ###################

add_executable(bowl_test main.c)
target_link_libraries(bowl_test bowl)