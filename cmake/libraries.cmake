include(${CMAKE_CURRENT_LIST_DIR}/system.cmake)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(crt_type d)
    set(cty_type_long _debug)
endif()

set(${CMAKE_PROJECT_NAME}_LIBRARIES
    glm_static
    assimp
    tbb${cty_type_long}
)

set(TEST_LIBRARIES
    gtest${crt_type}
    gmock${crt_type}
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