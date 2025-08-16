cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

project(client)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(GLOBAL_VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/")
set(LOCAL_VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
set(COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../common/")

set(GLFW_BUILD_DOCS OFF BOOL "GLFW lib only")
set(GLFW_INSTALL OFF CACHE BOOL "GLFW lib only")

add_subdirectory(${LOCAL_VENDOR_DIR}/glfw)
add_subdirectory(${LOCAL_VENDOR_DIR}/freetype-2.13.3)

file(GLOB_RECURSE SRC_CXX_FILES "${SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE SRC_C_FILES "${SOURCE_DIR}/*.c")

include_directories("${GLOBAL_VENDOR_DIR}/asio" "${COMMON_DIR}" "${LOCAL_VENDOR_DIR}")

add_executable(client ${SRC_CXX_FILES} ${SRC_C_FILES})
target_link_libraries(client glfw freetype)