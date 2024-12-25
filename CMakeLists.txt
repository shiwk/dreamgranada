cmake_minimum_required(VERSION 3.12)
project(dreamgranada LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/ ${CMAKE_CURRENT_SOURCE_DIR}/common)
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_subdirectory(3rd_party/glog glog)
add_subdirectory(3rd_party/fmt)
add_subdirectory(ioloop)
add_subdirectory(clients)

add_executable(dreamgranada 
                ${HEADERS} 
                ${SOURCES})


target_link_libraries(dreamgranada PRIVATE granada_ioloop granada_client glog::glog fmt::fmt)