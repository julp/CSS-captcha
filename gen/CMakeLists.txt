cmake_minimum_required(VERSION 2.8.3)

project(CSS-captcha C)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
find_package(ICU 4.4 REQUIRED)

include_directories(
    ${ICU_INCLUDE_DIRS}
)

set(LIBRARIES )
set(SOURCES tables.c darray.c)

add_definitions(-DWITHOUT_ITERATOR) # darray

if(${CMAKE_SYSTEM_NAME} MATCHES "BSD$")
    list(APPEND LIBRARIES "util")
endif(${CMAKE_SYSTEM_NAME} MATCHES "BSD$")

# include(CheckFunctionExists)
# check_function_exists("getopt_long")
include(CheckIncludeFile)
check_include_file(getopt.h HAVE_GETOPT_H)
if(HAVE_GETOPT_H)
    include(CheckSymbolExists)
    check_symbol_exists("getopt_long" "getopt.h" HAVE_GETOPT_LONG)
    if(NOT HAVE_GETOPT_LONG)
        message(FATAL "your system offers getopt but not getopt_long")
    endif(NOT HAVE_GETOPT_LONG)
else(HAVE_GETOPT_H)
    list(APPEND SOURCES getopt_long.c)
endif(HAVE_GETOPT_H)

add_executable(tables ${SOURCES})
target_link_libraries(tables ${ICU_LIBRARIES} ${LIBRARIES})
