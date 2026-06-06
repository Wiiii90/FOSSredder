include_guard(GLOBAL)

set(FOSSREDDER_EXPORT_SET "FossRedderTargets")
set(FOSSREDDER_INSTALL_CMAKEDIR "lib/cmake/FossRedder")

function(fossredder_set_runtime_output_dirs _target)
    set_target_properties(${_target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
    )
endfunction()

function(fossredder_set_library_output_dirs _target)
    set_target_properties(${_target} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/lib/Debug"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/Release"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
    )
endfunction()

function(fossredder_enable_target_pch _target _header)
    if(EXISTS "${_header}")
        target_precompile_headers(${_target} PRIVATE "${_header}")
    endif()
endfunction()

function(fossredder_install_library_target _target)
    install(TARGETS ${_target}
        EXPORT ${FOSSREDDER_EXPORT_SET}
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
    )
endfunction()

function(fossredder_install_headers _include_dir)
    install(DIRECTORY "${_include_dir}/" DESTINATION include)
endfunction()

function(fossredder_configure_qt_runtime _target)
    set(options)
    set(oneValueArgs QML_SOURCE_DIR QML_TOOLING_DIR)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "" ${ARGN})

    if(WIN32 AND DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        if(MSVC)
            set(_target_dir_macro "$(TargetDir)")
            set(_dbg_env "$<$<CONFIG:Debug>:PATH=${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin$<SEMICOLON>VCPKG_INSTALLED_DIR=${VCPKG_INSTALLED_DIR}$<SEMICOLON>QT_QPA_PLATFORM_PLUGIN_PATH=${_target_dir_macro}platforms$<SEMICOLON>QT_PLUGIN_PATH=${_target_dir_macro}$<SEMICOLON>QML2_IMPORT_PATH=${_target_dir_macro}qml$<SEMICOLON>QML_IMPORT_PATH=${_target_dir_macro}qml>")
            set(_rel_env "$<$<CONFIG:Release>:PATH=${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin$<SEMICOLON>QT_QPA_PLATFORM_PLUGIN_PATH=${_target_dir_macro}platforms$<SEMICOLON>QT_PLUGIN_PATH=${_target_dir_macro}$<SEMICOLON>QML2_IMPORT_PATH=${_target_dir_macro}qml$<SEMICOLON>QML_IMPORT_PATH=${_target_dir_macro}qml>")

            set_target_properties(${_target} PROPERTIES
                VS_DEBUGGER_ENVIRONMENT "${_dbg_env}${_rel_env}"
            )
        endif()

        set(_deploy_args
            -DTARGET_DIR="$<TARGET_FILE_DIR:${_target}>"
            -DVCPKG_INSTALLED_DIR="${VCPKG_INSTALLED_DIR}"
            -DVCPKG_TARGET_TRIPLET="${VCPKG_TARGET_TRIPLET}"
            -DBUILD_CONFIG="$<CONFIG>"
        )

        if(ARG_QML_SOURCE_DIR)
            list(APPEND _deploy_args -DAPP_QML_SOURCE_DIR="${ARG_QML_SOURCE_DIR}")
        endif()
        if(ARG_QML_TOOLING_DIR)
            list(APPEND _deploy_args -DAPP_QML_TOOLING_DIR="${ARG_QML_TOOLING_DIR}")
        endif()

        add_custom_command(TARGET ${_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                ${_deploy_args}
                -P "${CMAKE_SOURCE_DIR}/cmake/modules/FossredderQtDeploy.cmake"
            VERBATIM
        )
    endif()
endfunction()
