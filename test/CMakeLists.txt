cmake_minimum_required(VERSION 3.14)

project(granada_test)

enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common)

add_subdirectory(config)
add_subdirectory(event)
add_subdirectory(http)
add_subdirectory(util)