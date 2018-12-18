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
ENDMACRO() 

macro(compile_shaders target)
    FOREACH(file ${SHADERS})
        ADD_CUSTOM_COMMAND(
            TARGET ${target}
            COMMAND $ENV{VULKAN_SDK}/bin/glslangValidator ARGS -V ${file} -o ${file}.spv
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