file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(event_ut ${SOURCES})

target_link_libraries(event_ut
    gtest
    gtest_main
    dreamgranada
)

add_test(NAME event_unitests COMMAND event_ut)