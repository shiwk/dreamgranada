cmake_minimum_required(VERSION 3.12)
project(dreamgranada LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common)

set(GRANADA_3RD_PARTY ${CMAKE_CURRENT_SOURCE_DIR}/3rd_party)

add_subdirectory(3rd_party/glog glog)
add_subdirectory(3rd_party/fmt)
add_subdirectory(3rd_party/gumbo)

add_subdirectory(utils)
add_subdirectory(communicate)


file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
add_library(dreamgranada
                ${HEADERS} 
                ${SOURCES})

find_package(Threads REQUIRED)
target_include_directories(dreamgranada PUBLIC ${BOOST_ROOT_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include/ ${CMAKE_CURRENT_SOURCE_DIR}/common)

target_link_libraries(dreamgranada PUBLIC granada_comm granada_utils glog::glog fmt::fmt Threads::Threads)