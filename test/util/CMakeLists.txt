file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(util_ut ${SOURCES})

target_link_libraries(util_ut
    gtest
    gtest_main
    granada_utils
)

add_test(NAME util_unitests COMMAND util_ut)