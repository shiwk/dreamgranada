cmake_minimum_required(VERSION 3.14)

project(granada_test)

enable_testing()

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(granda_ut ${SOURCES})

target_link_libraries(granda_ut
    gtest
    gtest_main
    dreamgranada
)

add_test(NAME granada_unitests COMMAND granda_ut)