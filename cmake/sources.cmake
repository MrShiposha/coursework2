include(${CMAKE_CURRENT_LIST_DIR}/system.cmake)

set(${CMAKE_PROJECT_NAME}_${SYSTEM}_SOURCES
    ${CMAKE_SOURCE_DIR}/src/${SYSTEM}/*.h
    ${CMAKE_SOURCE_DIR}/src/${SYSTEM}/*.cpp
)

if(${SYSTEM} STREQUAL "darwin")
    set(${CMAKE_PROJECT_NAME}_${SYSTEM}_SOURCES 
        ${${CMAKE_PROJECT_NAME}_${SYSTEM}_SOURCES}
        ${CMAKE_SOURCE_DIR}/src/${SYSTEM}/*.m
        ${CMAKE_SOURCE_DIR}/src/${SYSTEM}/*.mm
    )
endif()

file(GLOB ${CMAKE_PROJECT_NAME}_SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${${CMAKE_PROJECT_NAME}_${SYSTEM}_SOURCES}
)

file(GLOB ${CMAKE_PROJECT_NAME}_SHADERS
    ${CMAKE_SOURCE_DIR}/src/shaders/*.vert
    ${CMAKE_SOURCE_DIR}/src/shaders/*.frag
)