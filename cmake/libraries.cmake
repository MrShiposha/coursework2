include(${CMAKE_CURRENT_LIST_DIR}/system.cmake)

set(${CMAKE_PROJECT_NAME}_LIBRARIES
    glm_static
    assimp
)

if(${SYSTEM} STREQUAL "darwin")
    set(${CMAKE_PROJECT_NAME}_FRAMEWORKS
        Cocoa
        AppKit
        QuartzCore
        vulkan
    )

    set(${CMAKE_PROJECT_NAME}_vulkan_FRAMEWORK_DIRECTORY $ENV{VULKAN_SDK}/Frameworks)
endif()

set(${CMAKE_PROJECT_NAME}_LINK_DIRECTORIES ${CMAKE_SOURCE_DIR}/lib/${SYSTEM}/bin)