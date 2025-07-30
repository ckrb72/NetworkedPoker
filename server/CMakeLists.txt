cmake_minimum_required(VERSION 3.20)

project(server)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB_RECURSE SRC_CXX_FILES "${SOURCE_DIR}/*.cpp")

add_executable(server ${SRC_CXX_FILES})