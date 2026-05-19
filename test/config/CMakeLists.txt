file(GLOB CONFIG_UT_SRCS *.cpp)

add_executable(config_ut ${CONFIG_UT_SRCS})

target_link_libraries(config_ut PUBLIC granada_utils gtest gtest_main)

add_test(NAME config_unitests COMMAND config_ut)