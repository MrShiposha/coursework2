include(${CMAKE_CURRENT_LIST_DIR}/system.cmake)

set(${CMAKE_PROJECT_NAME}_INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/lib/${SYSTEM}/include
    $ENV{VULKAN_SDK}/include
)