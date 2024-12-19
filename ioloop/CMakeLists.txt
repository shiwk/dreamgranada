set(BOOST_ROOT_DIR
        ""
        CACHE STRING "boost.asio include dir.")

if(BOOST_ROOT_DIR STREQUAL "")
    message(SEND_ERROR "boost.asio include dir cannot be empty")
endif()

file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp )
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(granada_ioloop
        ${SOURCES}
        ${HEADERS})

target_include_directories(granada_ioloop PUBLIC ${BOOST_ROOT_DIR} )
target_include_directories(granada_ioloop PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)