file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp )
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(granada_utils
        ${SOURCES}
        ${HEADERS})

target_include_directories(granada_utils PUBLIC  ${GRANADA_3RD_PARTY}/rapidjson/include)
target_include_directories(granada_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(granada_utils PRIVATE glog::glog fmt::fmt)
target_link_libraries(granada_utils PUBLIC gumbo)
