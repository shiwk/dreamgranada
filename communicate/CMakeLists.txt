set(BOOST_ROOT_DIR
        ""
        CACHE STRING "boost.asio include dir.")

if(BOOST_ROOT_DIR STREQUAL "")
    message(SEND_ERROR "boost.asio include dir cannot be empty")
endif()

find_package(OpenSSL REQUIRED)

file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp )
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(granada_comm
        ${SOURCES}
        ${HEADERS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(granada_comm PRIVATE DEBUG_BUILD)
endif()

target_link_libraries(granada_comm PUBLIC glog::glog fmt::fmt OpenSSL::SSL)
target_include_directories(granada_comm PUBLIC ${BOOST_ROOT_DIR})
target_include_directories(granada_comm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)