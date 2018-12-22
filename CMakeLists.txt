cmake_minimum_required(VERSION 3.12)
project(coursework2)

include(cmake/system.cmake)
include(cmake/definitions.cmake)
include(cmake/include_directories.cmake)
include(cmake/libraries.cmake)
include(cmake/sources.cmake)
include(cmake/util.cmake)

add_definitions(${${CMAKE_PROJECT_NAME}_DEFINITIONS})
include_directories(${${CMAKE_PROJECT_NAME}_INCLUDE_DIRECTORIES})
link_directories(${${CMAKE_PROJECT_NAME}_LINK_DIRECTORIES})

REQUIRE_CXX_17()

if(${SYSTEM} STREQUAL "darwin")
    add_macos_bundle(${CMAKE_PROJECT_NAME})
else()
    add_executable(${CMAKE_PROJECT_NAME} ${${CMAKE_PROJECT_NAME}_SOURCES})
    target_link_libraries(${CMAKE_PROJECT_NAME} ${${CMAKE_PROJECT_NAME}_LIBRARIES})
    set_executable_directory(${CMAKE_PROJECT_NAME})
endif()

compile_shaders(${CMAKE_PROJECT_NAME})
copy_directory(${CMAKE_PROJECT_NAME} ${CMAKE_SOURCE_DIR}/resources ${${CMAKE_PROJECT_NAME}_EXECUTABLE_DIRECTORY}/resources)

if(${SYSTEM} STREQUAL "windows")
    copy_target_dlls(${CMAKE_PROJECT_NAME})
endif()