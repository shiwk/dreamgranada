cmake_minimum_required(VERSION 3.14)

project(granada_test)

enable_testing()

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(granda_ut ${SOURCES})

# 链接到 gtest 和 gtest_main
target_link_libraries(granda_ut
    gtest
    gtest_main
    dreamgranada
)

# 将测试可执行文件添加到 CTest 测试套件中
add_test(NAME granada_unitests COMMAND granda_ut)