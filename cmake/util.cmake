include(cmake/system.cmake)

MACRO(REQUIRE_CXX_17)
    SET(CMAKE_CXX_STANDARD 17)

    IF (MSVC_VERSION GREATER_EQUAL "1900")
        INCLUDE(CheckCXXCompilerFlag)
        CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
        IF (_cpp_latest_flag_supported)
            ADD_COMPILE_OPTIONS("/std:c++latest")
        ELSE()
            MESSAGE(FATAL_ERROR "C++17 is required")
        ENDIF()
    ENDIF()
ENDMACRO()

MACRO(LINK_MACOS_FRAMEWORKS target frameworks frameworks_directories)
    FOREACH(FRAMEWORK ${${frameworks}})
        SET(___frameworks_link_flags___ "${___frameworks_link_flags___} -framework ${FRAMEWORK}")

        IF(DEFINED ${target}_${FRAMEWORK}_FRAMEWORK_DIRECTORY)
            SET(___frameworks_link_flags___ "${___frameworks_link_flags___} -F${${target}_${FRAMEWORK}_FRAMEWORK_DIRECTORY}")

            ADD_CUSTOM_COMMAND(
                TARGET ${target}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${${target}_${FRAMEWORK}_FRAMEWORK_DIRECTORY}/${FRAMEWORK}.framework ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${target}.app/Contents/Frameworks/${FRAMEWORK}.framework
                COMMENT "Copying ${FRAMEWORK}"
            )
        ENDIF()
    ENDFOREACH()

    IF(DEFINED ___frameworks_link_flags___)
        SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${___frameworks_link_flags___}")
    ENDIF()

ENDMACRO()

MACRO(add_macos_bundle project_name)
    ADD_EXECUTABLE(
        ${project_name}
        MACOSX_BUNDLE
        ${${project_name}_SOURCES}
        ${VULKAN_FRAMEWORK}
    )

    LINK_MACOS_FRAMEWORKS(${project_name} ${project_name}_FRAMEWORKS ${project_name}_FRAMEWORKS_DIRECTORIES)
    TARGET_LINK_LIBRARIES(${project_name} ${${project_name}_LIBRARIES})

    file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${target}.app)

    ADD_CUSTOM_COMMAND(
        TARGET ${project_name}
        POST_BUILD COMMAND
        ${CMAKE_INSTALL_NAME_TOOL} -add_rpath "@executable_path/../Frameworks/" $<TARGET_FILE:${project_name}>
    )

    SET(MACOS_BUNDLE_PLIST ${CMAKE_SOURCE_DIR}/platform/darwin/plist.in)
    SET_TARGET_PROPERTIES(
        ${project_name}
        PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST
        ${MACOS_BUNDLE_PLIST}
        LINKER_LANGUAGE CXX
    )

    add_custom_command(
        TARGET ${project_name} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}
        COMMENT "Erase old executable version"
    )

    set_executable_directory(${project_name})
ENDMACRO()

macro(set_executable_directory target)
    if(${SYSTEM} STREQUAL "darwin")
        set(${target}_EXECUTABLE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${target}.app/Contents/MacOS)
    else()
        set(${target}_EXECUTABLE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    endif()
endmacro()

macro(compile_shaders target)
    file(GLOB ${target}_SHADERS
        "${CMAKE_SOURCE_DIR}/src/shaders/*.vert"
        "${CMAKE_SOURCE_DIR}/src/shaders/*.frag"
    )

    FOREACH(file ${${target}_SHADERS})
        get_filename_component(filename ${file} NAME)
        ADD_CUSTOM_COMMAND(
            TARGET ${target}
            COMMAND $ENV{VULKAN_SDK}/bin/glslangValidator ARGS -V ${file} -o ${CMAKE_SOURCE_DIR}/resources/shaders/${filename}.spv
            COMMENT "Compiling shader ${file}"
        )
    ENDFOREACH()
endmacro()

macro(copy_directory target from_dir to_dir)
    ADD_CUSTOM_COMMAND(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${from_dir} ${to_dir}
        COMMENT "Copying directory ${from_dir} -> ${to_dir}"
    )
endmacro()

macro(copy_target_dlls target_name)
    file(GLOB dlls ${CMAKE_SOURCE_DIR}/lib/${SYSTEM}/bin/*${CMAKE_SHARED_LIBRARY_SUFFIX})

    foreach(dll ${dlls})
        file(COPY ${dll} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    endforeach(dll ${dlls})
endmacro()