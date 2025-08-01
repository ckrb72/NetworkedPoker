cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

project(server)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/")
set(COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../common/")

file(GLOB_RECURSE SRC_CXX_FILES "${SOURCE_DIR}/*.cpp")

include_directories("${VENDOR_DIR}/asio" "${COMMON_DIR}")

add_executable(server ${SRC_CXX_FILES})
target_link_libraries(server ws2_32)