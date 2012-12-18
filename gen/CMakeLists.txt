cmake_minimum_required(VERSION 2.8.3)

project(CSS-captcha C)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
find_package(ICU 4.4 REQUIRED)

include_directories(
    ${ICU_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

add_executable(tables tables.c)
target_link_libraries(tables ${ICU_LIBRARIES})
