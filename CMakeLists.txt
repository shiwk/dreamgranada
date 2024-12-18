cmake_minimum_required(VERSION 3.12)
project(plangranada LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${CMAKE_CURRENT_SOURCE_DIR}/common)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_subdirectory(3rd_party/glog glog)

add_subdirectory(asio granada_asio)
add_executable(plangranada ${HEADERS} ${SOURCES})
target_link_libraries(plangranada PRIVATE granada_asio glog::glog)