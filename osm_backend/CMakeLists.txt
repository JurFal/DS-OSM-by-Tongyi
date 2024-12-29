cmake_minimum_required(VERSION 3.10)
project(osm_pugixml)
# set(CMAKE_TOOLCHAIN_FILE "C:/Users/Administrator/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
# set(CMAKE_PREFIX_PATH "C:/Users/Administrator/vcpkg/installed/x64-windows" ${CMAKE_PREFIX_PATH})
set(SOURCES 
    xml_convert_pugi.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/headers)
add_library(pugixml STATIC ${PROJECT_SOURCE_DIR}/pugixml.cpp)
add_library(graph STATIC ${PROJECT_SOURCE_DIR}/graph.cpp)
# find_package(tinyxml2 REQUIRED)
add_executable(${PROJECT_NAME} ${SOURCES})
# target_link_libraries(${PROJECT_NAME} PRIVATE tinyxml2::tinyxml2)
target_link_libraries(${PROJECT_NAME} PRIVATE pugixml)
target_link_libraries(${PROJECT_NAME} PRIVATE graph)
target_link_libraries(${PROJECT_NAME} PRIVATE Ws2_32)