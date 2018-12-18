cmake_minimum_required(VERSION 3.12)
project(coursework2)

include(cmake/system.cmake)
include(cmake/include_directories.cmake)
include(cmake/libraries.cmake)
include(cmake/sources.cmake)
include(cmake/util.cmake)

include_directories(${${CMAKE_PROJECT_NAME}_INCLUDE_DIRECTORIES})
link_directories(${${CMAKE_PROJECT_NAME}_LINK_DIRECTORIES})

REQUIRE_CXX_17()

if(${SYSTEM} STREQUAL "darwin")
    add_macos_bundle(${CMAKE_PROJECT_NAME})
endif()