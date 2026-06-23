if(NOT DEFINED TARGET_DIR AND DEFINED ENV{TARGET_DIR})
    set(TARGET_DIR "$ENV{TARGET_DIR}")
endif()
if(NOT DEFINED VCPKG_INSTALLED_DIR AND DEFINED ENV{VCPKG_INSTALLED_DIR})
    set(VCPKG_INSTALLED_DIR "$ENV{VCPKG_INSTALLED_DIR}")
endif()
if(NOT DEFINED VCPKG_TARGET_TRIPLET AND DEFINED ENV{VCPKG_TARGET_TRIPLET})
    set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_TARGET_TRIPLET}")
endif()

if(DEFINED VCPKG_INSTALLED_DIR)
    string(REPLACE "\"" "" VCPKG_INSTALLED_DIR "${VCPKG_INSTALLED_DIR}")
endif()
if(DEFINED VCPKG_TARGET_TRIPLET)
    string(REPLACE "\"" "" VCPKG_TARGET_TRIPLET "${VCPKG_TARGET_TRIPLET}")
endif()
if(DEFINED TARGET_DIR)
    string(REPLACE "\"" "" TARGET_DIR "${TARGET_DIR}")
endif()
if(DEFINED BUILD_CONFIG)
    string(REPLACE "\"" "" BUILD_CONFIG "${BUILD_CONFIG}")
endif()
if(DEFINED APP_QML_MODULE_DIR)
    string(REPLACE "\"" "" APP_QML_MODULE_DIR "${APP_QML_MODULE_DIR}")
endif()
if(DEFINED APP_QML_SOURCE_DIR)
    string(REPLACE "\"" "" APP_QML_SOURCE_DIR "${APP_QML_SOURCE_DIR}")
endif()
if(DEFINED APP_QML_TOOLING_DIR)
    string(REPLACE "\"" "" APP_QML_TOOLING_DIR "${APP_QML_TOOLING_DIR}")
endif()

if(NOT DEFINED TARGET_DIR OR TARGET_DIR STREQUAL "")
    message(FATAL_ERROR "qt_deploy: TARGET_DIR not provided")
endif()
if(NOT DEFINED VCPKG_INSTALLED_DIR OR VCPKG_INSTALLED_DIR STREQUAL "")
    message(FATAL_ERROR "qt_deploy: VCPKG_INSTALLED_DIR not provided")
endif()
if(NOT DEFINED VCPKG_TARGET_TRIPLET OR VCPKG_TARGET_TRIPLET STREQUAL "")
    message(FATAL_ERROR "qt_deploy: VCPKG_TARGET_TRIPLET not provided")
endif()

set(_cfg "${BUILD_CONFIG}")
if(_cfg STREQUAL "" AND DEFINED CMAKE_BUILD_TYPE)
    set(_cfg "${CMAKE_BUILD_TYPE}")
endif()
if(_cfg MATCHES "\\$<CONFIG>" OR _cfg STREQUAL "")
    string(TOUPPER "${TARGET_DIR}" _target_dir_upper)
    if(_target_dir_upper MATCHES "[/\\]DEBUG$")
        set(_cfg "DEBUG")
    elseif(_target_dir_upper MATCHES "[/\\]RELEASE$")
        set(_cfg "RELEASE")
    endif()
endif()
string(TOUPPER "${_cfg}" _cfg)

set(_vroot "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
if(_cfg STREQUAL "DEBUG")
    set(_src_bin "${_vroot}/debug/bin")
    set(_qt_dll_suffix "d")
    set(_qt_plugin_suffix "d")

    set(_src_qml "${_vroot}/debug/qml")
    if(NOT EXISTS "${_src_qml}")
        set(_src_qml "${_vroot}/debug/Qt6/qml")
    endif()

    set(_src_platforms "${_vroot}/debug/plugins/platforms")
    if(NOT EXISTS "${_src_platforms}")
        set(_src_platforms "${_vroot}/debug/Qt6/plugins/platforms")
    endif()

    set(_src_imageformats "${_vroot}/debug/plugins/imageformats")
    if(NOT EXISTS "${_src_imageformats}")
        set(_src_imageformats "${_vroot}/debug/Qt6/plugins/imageformats")
    endif()
else()
    set(_src_bin "${_vroot}/bin")
    set(_qt_dll_suffix "")
    set(_qt_plugin_suffix "")

    set(_src_qml "${_vroot}/qml")
    if(NOT EXISTS "${_src_qml}")
        set(_src_qml "${_vroot}/Qt6/qml")
    endif()

    set(_src_platforms "${_vroot}/plugins/platforms")
    if(NOT EXISTS "${_src_platforms}")
        set(_src_platforms "${_vroot}/Qt6/plugins/platforms")
    endif()

    set(_src_imageformats "${_vroot}/plugins/imageformats")
    if(NOT EXISTS "${_src_imageformats}")
        set(_src_imageformats "${_vroot}/Qt6/plugins/imageformats")
    endif()
endif()

if(NOT EXISTS "${_src_bin}")
    message(STATUS "qt_deploy: source bin ${_src_bin} not found; skipping")
    return()
endif()

file(MAKE_DIRECTORY "${TARGET_DIR}")
file(LOCK "${TARGET_DIR}/.qtdeploy.lock" GUARD PROCESS TIMEOUT 120)
file(REMOVE_RECURSE "${TARGET_DIR}/platforms")
file(REMOVE_RECURSE "${TARGET_DIR}/imageformats")
file(REMOVE_RECURSE "${TARGET_DIR}/qml")
file(MAKE_DIRECTORY "${TARGET_DIR}/platforms")
file(MAKE_DIRECTORY "${TARGET_DIR}/qml")
file(MAKE_DIRECTORY "${TARGET_DIR}/imageformats")
file(WRITE "${TARGET_DIR}/qt.conf" "[Paths]\nPlugins=./\nQml2Imports=./qml\n")

set(_dlls
    Qt6Core${_qt_dll_suffix}.dll
    Qt6Gui${_qt_dll_suffix}.dll
    Qt6OpenGL${_qt_dll_suffix}.dll
    Qt6Qml${_qt_dll_suffix}.dll
    Qt6QmlMeta${_qt_dll_suffix}.dll
    Qt6QmlModels${_qt_dll_suffix}.dll
    Qt6Quick${_qt_dll_suffix}.dll
    Qt6QuickEffects${_qt_dll_suffix}.dll
    Qt6QuickControls2${_qt_dll_suffix}.dll
    Qt6QuickControls2Basic${_qt_dll_suffix}.dll
    Qt6QuickControls2Fusion${_qt_dll_suffix}.dll
    Qt6QuickControls2FusionStyleImpl${_qt_dll_suffix}.dll
    Qt6QuickControls2Impl${_qt_dll_suffix}.dll
    Qt6QuickLayouts${_qt_dll_suffix}.dll
    Qt6QuickTemplates2${_qt_dll_suffix}.dll
    Qt6Svg${_qt_dll_suffix}.dll
    Qt6Widgets${_qt_dll_suffix}.dll
    MSVCP140.dll
)
foreach(_dll IN LISTS _dlls)
    if(EXISTS "${_src_bin}/${_dll}")
        file(COPY "${_src_bin}/${_dll}" DESTINATION "${TARGET_DIR}")
    endif()
endforeach()

if(EXISTS "${_src_platforms}/qwindows${_qt_plugin_suffix}.dll")
    file(COPY "${_src_platforms}/qwindows${_qt_plugin_suffix}.dll" DESTINATION "${TARGET_DIR}/platforms")
else()
    message(WARNING "qt_deploy: qwindows${_qt_plugin_suffix}.dll not found under ${_src_platforms}")
endif()

if(EXISTS "${_src_imageformats}/qsvg${_qt_plugin_suffix}.dll")
    file(COPY "${_src_imageformats}/qsvg${_qt_plugin_suffix}.dll" DESTINATION "${TARGET_DIR}/imageformats")
endif()

if(EXISTS "${_src_imageformats}/qjpeg${_qt_plugin_suffix}.dll")
    file(COPY "${_src_imageformats}/qjpeg${_qt_plugin_suffix}.dll" DESTINATION "${TARGET_DIR}/imageformats")
endif()

if(EXISTS "${_src_qml}")
    file(GLOB_RECURSE _qml "${_src_qml}/*")
    foreach(_q IN LISTS _qml)
        file(RELATIVE_PATH _rrel "${_src_qml}" "${_q}")
        get_filename_component(_dstdir "${TARGET_DIR}/qml/${_rrel}" DIRECTORY)
        if(NOT EXISTS "${_dstdir}")
            file(MAKE_DIRECTORY "${_dstdir}")
        endif()

        string(REGEX MATCH "\\.pdb$" _ispdb "${_q}")
        if(NOT _ispdb)
            file(COPY "${_q}" DESTINATION "${_dstdir}")
        endif()
    endforeach()
endif()

if(DEFINED APP_QML_SOURCE_DIR AND EXISTS "${APP_QML_SOURCE_DIR}")
    file(COPY "${APP_QML_SOURCE_DIR}" DESTINATION "${TARGET_DIR}/qml")
endif()

if(DEFINED APP_QML_TOOLING_DIR AND EXISTS "${APP_QML_TOOLING_DIR}")
    file(GLOB_RECURSE _app_qml_metadata "${APP_QML_TOOLING_DIR}/*.qmltypes")
    foreach(_meta IN LISTS _app_qml_metadata)
        file(RELATIVE_PATH _meta_rel "${APP_QML_TOOLING_DIR}" "${_meta}")
        get_filename_component(_meta_dest_dir "${TARGET_DIR}/qml/FossRedder/${_meta_rel}" DIRECTORY)
        file(MAKE_DIRECTORY "${_meta_dest_dir}")
        file(COPY "${_meta}" DESTINATION "${_meta_dest_dir}")
    endforeach()
endif()

set(_target_folderlist_dir "${TARGET_DIR}/qml/Qt/labs/folderlistmodel")
file(MAKE_DIRECTORY "${_target_folderlist_dir}")

set(_found_folderlist 0)
set(_folderlist_candidates
    "${_src_qml}/Qt/labs/folderlistmodel/qmlfolderlistmodelplugin${_qt_plugin_suffix}.dll"
    "${_src_bin}/qmlfolderlistmodelplugin${_qt_plugin_suffix}.dll"
    "${_vroot}/plugins/labs/qmlfolderlistmodelplugin${_qt_plugin_suffix}.dll"
)
foreach(_ld IN LISTS _folderlist_candidates)
    if(EXISTS "${_ld}")
        # Atomic copy: copy to temporary filename then rename into place with retries.
        get_filename_component(_ldname "${_ld}" NAME)
        set(_temp_dest "${_target_folderlist_dir}/${_ldname}.tmp")
        set(_final_dest "${_target_folderlist_dir}/${_ldname}")

        set(_copy_rc 1)
        set(_attempt 0)
        while(_attempt LESS 3 AND _copy_rc)
            if(EXISTS "${_temp_dest}")
                file(REMOVE "${_temp_dest}")
            endif()
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${_ld}" "${_temp_dest}"
                RESULT_VARIABLE _copy_rc
                OUTPUT_VARIABLE _copy_out
                ERROR_VARIABLE _copy_err
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_STRIP_TRAILING_WHITESPACE)
            if(_copy_rc EQUAL 0)
                # If final exists try remove it first to avoid rename conflict
                if(EXISTS "${_final_dest}")
                    file(REMOVE "${_final_dest}")
                endif()
                execute_process(COMMAND ${CMAKE_COMMAND} -E rename "${_temp_dest}" "${_final_dest}"
                    RESULT_VARIABLE _rename_rc
                    OUTPUT_VARIABLE _rename_out
                    ERROR_VARIABLE _rename_err
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_STRIP_TRAILING_WHITESPACE)
                if(_rename_rc EQUAL 0)
                    set(_found_folderlist 1)
                    break()
                else()
                    # remove temp if rename failed, then retry
                    if(EXISTS "${_temp_dest}")
                        file(REMOVE "${_temp_dest}")
                    endif()
                endif()
            endif()
            execute_process(COMMAND ${CMAKE_COMMAND} -E sleep 1)
            math(EXPR _attempt "${_attempt} + 1")
        endwhile()

        if(_found_folderlist)
            message(STATUS "qt_deploy: copied ${_ldname} to ${_target_folderlist_dir}")
            # list contents for diagnostics
            file(GLOB _contents RELATIVE "${_target_folderlist_dir}" "${_target_folderlist_dir}/*")
            foreach(_c IN LISTS _contents)
                message(STATUS "qt_deploy: content: ${_c}")
            endforeach()
        else()
            message(WARNING "qt_deploy: failed to copy ${_ldname} to ${_target_folderlist_dir} after ${_attempt} attempts. error: ${_copy_err} ${_rename_err}")
            # list contents for diagnostics
            if(EXISTS "${_target_folderlist_dir}")
                file(GLOB _contents RELATIVE "${_target_folderlist_dir}" "${_target_folderlist_dir}/*")
                foreach(_c IN LISTS _contents)
                    message(STATUS "qt_deploy: content: ${_c}")
                endforeach()
            endif()
        endif()
        break()
    endif()
endforeach()

if(NOT _found_folderlist)
    message(WARNING "qt_deploy: qmlfolderlistmodelplugin${_qt_plugin_suffix}.dll not found; Qt.labs.folderlistmodel may fail to load at runtime")
else()
    message(STATUS "qt_deploy: copied qmlfolderlistmodelplugin${_qt_plugin_suffix}.dll to ${_target_folderlist_dir}")
endif()

set(_target_effects_dir "${TARGET_DIR}/qml/QtQuick/Effects")
file(MAKE_DIRECTORY "${_target_effects_dir}")

set(_effects_candidates
    "${_src_qml}/QtQuick/Effects/effectsplugin${_qt_plugin_suffix}.dll"
    "${_vroot}/plugins/QtQuick/Effects/effectsplugin${_qt_plugin_suffix}.dll"
    "${_src_bin}/qml/QtQuick/Effects/effectsplugin${_qt_plugin_suffix}.dll"
)

set(_found_effects 0)
foreach(_ed IN LISTS _effects_candidates)
    if(EXISTS "${_ed}")
        file(COPY "${_ed}" DESTINATION "${_target_effects_dir}")
        set(_found_effects 1)
        break()
    endif()
endforeach()

if(NOT _found_effects)
    message(WARNING "qt_deploy: effectsplugin${_qt_plugin_suffix}.dll not found; QtQuick.Effects may fail to load at runtime")
endif()
