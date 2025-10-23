enable_testing()

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(http_ut ${SOURCES})

target_link_libraries(http_ut
    gtest
    gtest_main
    granada_comm
)

add_test(NAME http_unitests COMMAND http_ut)