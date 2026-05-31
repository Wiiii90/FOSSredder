find_package(Qt6 COMPONENTS Quick Qml QuickControls2 Widgets Svg Concurrent QUIET)

# Disable QML cache generation for faster local iteration when requested.
# This suppresses the per-file _qml.cpp cachegen step for qt_add_qml_module().
option(FOSSREDDER_FAST_QML_BUILD "Disable QML cache generation for faster local builds" OFF)
if(FOSSREDDER_FAST_QML_BUILD)
    if(DEFINED Qt6_VERSION AND Qt6_VERSION VERSION_GREATER_EQUAL "6.8.1")
        set(QT_QML_NO_CACHEGEN TRUE)
        message(STATUS "Fast QML build enabled: cache generation disabled for ui QML modules")
    else()
        message(WARNING "FOSSREDDER_FAST_QML_BUILD requires Qt 6.8.1 or newer; QML cache generation remains enabled")
    endif()
endif()

# UI library sources and public headers.
set(UI_SOURCES
    src/MainWindow.cpp
    src/shell/AppActions.cpp
    src/adapters/core/AnalysisRequestMapper.cpp
    src/adapters/core/AnalysisResultMapper.cpp
    src/adapters/core/AnnualRequestMapper.cpp
    src/adapters/core/AnnualResultMapper.cpp
    src/shell/AppContext.cpp
    src/shell/QmlRuntime.cpp
    src/workflows/analysis/AnalysisWorkflow.cpp
    src/workflows/annual/AnnualWorkflow.cpp
    src/workflows/export/ExportWorkflow.cpp
    src/platform/filesystem/FileSystemBrowser.cpp
    src/workflows/import/ImportWorkflow.cpp
    src/platform/localization/LanguageService.cpp
    src/viewmodels/system/SettingsViewModel.cpp
    src/platform/dialogs/FileDialogs.cpp
    src/workflows/export/WorkspaceSnapshot.cpp
    src/workflows/export/ExportRunner.cpp
    src/adapters/core/DraftViewMapper.cpp
    src/adapters/core/ImportDraftMapper.cpp
    src/workflows/import/ImportJobBridge.cpp
    src/workflows/import/ImportRunStore.cpp
    src/adapters/core/ImportSuggestionMapper.cpp
    src/workflows/import/ImportWorkflowState.cpp
    src/viewmodels/catalog/ActorListModel.cpp
    src/viewmodels/reporting/AnalysisListModel.cpp
    src/viewmodels/reporting/AnnualListModel.cpp
    src/viewmodels/catalog/ContractListModel.cpp
    src/viewmodels/export/ExportRunListModel.cpp
    src/viewmodels/import/ImportRunListModel.cpp
    src/viewmodels/catalog/PropertyListModel.cpp
    src/viewmodels/import/StatementDraftViewModel.cpp
    src/viewmodels/booking/StatementListModel.cpp
    src/viewmodels/import/TransactionDraftListModel.cpp
    src/viewmodels/booking/TransactionFilterModel.cpp
    src/viewmodels/booking/TransactionListModel.cpp
    src/state/filters/FilterState.cpp
    src/state/export/ExportState.cpp
    src/state/import/ImportState.cpp
    src/state/import/StatementDraftState.cpp
    src/state/import/TransactionDraftState.cpp
    src/state/navigation/NavigationState.cpp
    src/state/settings/SettingsState.cpp
    src/state/session/AnalysisState.cpp
    src/state/session/AnnualState.cpp
    src/state/session/ActorState.cpp
    src/state/session/BookingState.cpp
    src/state/session/ContractState.cpp
    src/state/session/PropertyState.cpp
    src/state/session/WorkspaceSessionSelection.cpp
    src/state/selection/SelectionState.cpp
    src/adapters/core/WorkspaceRowProjector.cpp
    src/state/session/WorkspaceSessionModels.cpp
    src/state/session/WorkspaceSessionState.cpp
    src/state/mutation/SessionMutationState.cpp
    src/workspace/WorkspaceFacade.cpp
    src/shell/window/CloseWorkflow.cpp
    src/shell/window/DropHandler.cpp
    src/shell/window/MainWindowContext.cpp
)

set(UI_HEADERS
    include/MainWindow.h
    include/ui/pch.h
    include/ui/shell/AppActions.h
    include/ui/adapters/core/AnalysisRequestMapper.h
    include/ui/adapters/core/AnalysisResultMapper.h
    include/ui/adapters/core/AnnualRequestMapper.h
    include/ui/adapters/core/AnnualResultMapper.h
    include/ui/shell/AppContext.h
    include/ui/shell/QmlContracts.h
    include/ui/shell/QmlRuntime.h
    include/ui/shared/config/Defaults.h
    include/ui/workflows/analysis/AnalysisWorkflow.h
    include/ui/workflows/annual/AnnualWorkflow.h
    include/ui/workflows/export/ExportWorkflow.h
    include/ui/platform/filesystem/FileSystemBrowser.h
    include/ui/workflows/import/ImportWorkflow.h
    include/ui/platform/localization/LanguageService.h
    include/ui/viewmodels/system/SettingsViewModel.h
    include/ui/platform/dialogs/FileDialogs.h
    include/ui/workflows/export/WorkspaceSnapshot.h
    include/ui/workflows/export/ExportRunner.h
    include/ui/adapters/core/DraftViewMapper.h
    include/ui/adapters/core/ImportDraftMapper.h
    include/ui/workflows/import/ImportJobBridge.h
    include/ui/workflows/import/ImportRunStore.h
    include/ui/adapters/core/ImportSuggestionMapper.h
    include/ui/workflows/import/ImportWorkflowState.h
    include/ui/viewmodels/catalog/ActorListModel.h
    include/ui/viewmodels/reporting/AnalysisListModel.h
    include/ui/viewmodels/reporting/AnnualListModel.h
    include/ui/viewmodels/catalog/ContractListModel.h
    include/ui/viewmodels/export/ExportRunListModel.h
    include/ui/viewmodels/import/ImportRunListModel.h
    include/ui/viewmodels/import/ImportSuggestionViewModel.h
    include/ui/viewmodels/base/IndexedListModel.h
    include/ui/viewmodels/catalog/PropertyListModel.h
    include/ui/viewmodels/base/RowListModel.h
    include/ui/viewmodels/import/StatementDraftViewModel.h
    include/ui/viewmodels/booking/StatementListModel.h
    include/ui/viewmodels/import/TransactionDraftViewModel.h
    include/ui/viewmodels/import/TransactionDraftListModel.h
    include/ui/viewmodels/booking/TransactionFilterModel.h
    include/ui/viewmodels/booking/TransactionListModel.h
    include/ui/shared/observability/Origins.h
    include/ui/shared/observability/Trace.h
    include/ui/shared/payload/PayloadKeys.h
    include/ui/shared/payload/PayloadMapper.h
    include/ui/state/filters/FilterState.h
    include/ui/state/export/ExportState.h
    include/ui/state/import/ImportState.h
    include/ui/state/import/StatementDraftState.h
    include/ui/state/import/TransactionDraftState.h
    include/ui/state/navigation/NavigationState.h
    include/ui/state/settings/SettingsState.h
    include/ui/state/session/AnalysisState.h
    include/ui/state/session/AnnualState.h
    include/ui/state/session/ActorState.h
    include/ui/state/session/BookingState.h
    include/ui/state/session/ContractState.h
    include/ui/state/session/PropertyState.h
    include/ui/state/session/WorkspaceSessionSelection.h
    include/ui/state/selection/SelectionState.h
    include/ui/state/session/WorkspaceSessionModels.h
    include/ui/state/session/WorkspaceSessionState.h
    include/ui/state/mutation/SessionMutationState.h
    include/ui/adapters/core/WorkspaceRowProjector.h
    include/ui/workspace/WorkspaceFacade.h
    include/ui/state/status/StatusState.h
    include/ui/shared/text/Text.h
    include/ui/shell/window/CloseWorkflow.h
    include/ui/shell/window/DropHandler.h
    include/ui/shell/window/MainWindowContext.h
    include/ui/shell/window/MainWindowTrace.h
    include/ui/shared/util/CoreFacadeGuard.h
    include/ui/shared/util/StringConversions.h
)

add_library(fossredder_ui STATIC ${UI_SOURCES} ${UI_HEADERS})
add_library(FossRedder::Ui ALIAS fossredder_ui)
fossredder_enable_target_quality(fossredder_ui)
if(MSVC)
    target_compile_options(fossredder_ui PRIVATE /FS)
endif()

set_target_properties(fossredder_ui PROPERTIES EXPORT_NAME Ui)

set_target_properties(fossredder_ui PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_include_directories(fossredder_ui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(fossredder_ui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/adapters
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/adapters/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/platform/dialogs
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/platform/filesystem
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/platform/localization
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/shared/config
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/shared/observability
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/shared/payload
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/shared/text
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/shared/util
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/shell
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/shell/window
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/state
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/state/export
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/state/filters
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/state/import
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/state/mutation
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/state/navigation
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/state/selection
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/state/settings
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/state/session
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/state/status
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/viewmodels
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/viewmodels/base
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/viewmodels/booking
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/viewmodels/catalog
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/viewmodels/export
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/viewmodels/import
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/viewmodels/reporting
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/viewmodels/system
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/workflows
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/workflows/analysis
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/workflows/annual
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/workflows/export
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/workflows/import
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/workspace
)

target_compile_features(fossredder_ui PUBLIC cxx_std_20)
fossredder_set_library_output_dirs(fossredder_ui)

# Optional Qt UI feature blocks.
if(TARGET Qt6::Quick AND TARGET Qt6::Qml)
    target_link_libraries(fossredder_ui PUBLIC Qt6::Quick Qt6::Qml)
    target_compile_definitions(fossredder_ui PUBLIC USE_QML=1)

    if(COMMAND qt_policy)
        qt_policy(SET QTP0004 NEW)
    endif()

    set(UI_QML_IMPORT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/qml")
    set(UI_QML_MODULE_ROOT "${UI_QML_IMPORT_ROOT}/FossRedder")
    set(UI_QML_ROOT_URI "FossRedder")
    set(UI_QML_MODULE_VERSION "1.0")
    set(UI_QML_TOOLING_QUICK_LIBRARIES Qt6::Quick Qt6::Qml Qt6::QuickControls2)

    function(fossredder_add_ui_qml_tooling_submodule target_name module_uri output_directory)
        cmake_parse_arguments(ARG "" "" "DEPENDENCIES;LINK_LIBRARIES;QML_FILES" ${ARGN})

        set(_qml_tooling_anchor "${CMAKE_CURRENT_BINARY_DIR}/${target_name}_qml_tooling_anchor.cpp")
        file(WRITE "${_qml_tooling_anchor}" "namespace { [[maybe_unused]] constexpr int qml_tooling_anchor = 0; }\n")

        add_library(${target_name} STATIC EXCLUDE_FROM_ALL "${_qml_tooling_anchor}")
        set_target_properties(${target_name} PROPERTIES AUTOMOC ON)
        if(MSVC)
            target_compile_options(${target_name} PRIVATE /FS)
        endif()
        if(ARG_LINK_LIBRARIES)
            target_link_libraries(${target_name} PRIVATE ${ARG_LINK_LIBRARIES})
        endif()

        qt_add_qml_module(${target_name}
            URI ${module_uri}
            VERSION ${UI_QML_MODULE_VERSION}
            OUTPUT_DIRECTORY "${output_directory}"
            RESOURCE_PREFIX /qml
            IMPORT_PATH
                "${UI_QML_TOOLING_OUTPUT_ROOT}"
                "${UI_QML_IMPORT_ROOT}"
            IMPORTS ${UI_QML_ROOT_URI}
            DEPENDENCIES ${ARG_DEPENDENCIES}
            NO_PLUGIN
            QML_FILES ${ARG_QML_FILES}
        )
    endfunction()

    file(GLOB UI_QML_ROOT_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS
        "${UI_QML_MODULE_ROOT}/Main.qml"
        "${UI_QML_MODULE_ROOT}/Theme/*.qml"
        "${UI_QML_MODULE_ROOT}/Constants/*.js"
    )
    file(GLOB UI_QML_ROOT_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS
        "${UI_QML_MODULE_ROOT}/Assets/*"
    )
    file(GLOB UI_QML_CONTROLS_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS
        "${UI_QML_MODULE_ROOT}/Controls/*.qml"
    )
    file(GLOB UI_QML_COMPONENTS_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS
        "${UI_QML_MODULE_ROOT}/Components/**/*.qml"
    )
    file(GLOB UI_QML_VIEWS_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS
        "${UI_QML_MODULE_ROOT}/Views/*.qml"
        "${UI_QML_MODULE_ROOT}/Views/**/*.qml"
    )

    set(UI_QML_TOOLING_OUTPUT_ROOT "${CMAKE_CURRENT_BINARY_DIR}/qmltooling")

    set_source_files_properties(
        qml/FossRedder/Theme/Theme.qml
        PROPERTIES QT_QML_SINGLETON_TYPE TRUE
    )

    qt_add_qml_module(fossredder_ui
        URI ${UI_QML_ROOT_URI}
        VERSION ${UI_QML_MODULE_VERSION}
        OUTPUT_DIRECTORY "${UI_QML_TOOLING_OUTPUT_ROOT}/FossRedder"
        RESOURCE_PREFIX /qml
        IMPORT_PATH "${UI_QML_IMPORT_ROOT}"
        QML_FILES ${UI_QML_ROOT_FILES}
        RESOURCES ${UI_QML_ROOT_RESOURCES}
    )

    fossredder_add_ui_qml_tooling_submodule(
        fossredder_ui_qml_controls
        "${UI_QML_ROOT_URI}.Controls"
        "${UI_QML_TOOLING_OUTPUT_ROOT}/FossRedder/Controls"
        DEPENDENCIES QtQuick
        LINK_LIBRARIES ${UI_QML_TOOLING_QUICK_LIBRARIES}
        QML_FILES ${UI_QML_CONTROLS_FILES}
    )

    fossredder_add_ui_qml_tooling_submodule(
        fossredder_ui_qml_components
        "${UI_QML_ROOT_URI}.Components"
        "${UI_QML_TOOLING_OUTPUT_ROOT}/FossRedder/Components"
        DEPENDENCIES QtQuick QtQuick.Controls
        LINK_LIBRARIES ${UI_QML_TOOLING_QUICK_LIBRARIES}
        QML_FILES ${UI_QML_COMPONENTS_FILES}
    )

    fossredder_add_ui_qml_tooling_submodule(
        fossredder_ui_qml_views
        "${UI_QML_ROOT_URI}.Views"
        "${UI_QML_TOOLING_OUTPUT_ROOT}/FossRedder/Views"
        DEPENDENCIES QtQuick QtQuick.Controls QtQuick.Layouts
        LINK_LIBRARIES ${UI_QML_TOOLING_QUICK_LIBRARIES}
        QML_FILES ${UI_QML_VIEWS_FILES}
    )

    target_link_libraries(fossredder_ui PRIVATE
        fossredder_ui_qml_controls
        fossredder_ui_qml_components
        fossredder_ui_qml_views
    )

    set(UI_QML_TOOLING_DEPS)
    foreach(_qml_tooling_target
            fossredder_ui_generate_qmlls_ini_file
            fossredder_ui_qml_controls_generate_qmlls_ini_file
            fossredder_ui_qml_components_generate_qmlls_ini_file
            fossredder_ui_qml_views_generate_qmlls_ini_file)
        if(TARGET ${_qml_tooling_target})
            list(APPEND UI_QML_TOOLING_DEPS ${_qml_tooling_target})
        endif()
    endforeach()

    add_custom_target(fossredder_ui_qml_tooling ALL
        DEPENDS ${UI_QML_TOOLING_DEPS}
    )
else()
    message(STATUS "Qt6 Quick/Qml not found; QML features disabled in fossredder_ui")
endif()

if(TARGET Qt6::QuickControls2)
    target_link_libraries(fossredder_ui PUBLIC Qt6::QuickControls2)
else()
    message(WARNING "Qt6 QuickControls2 not found; QML Controls may be missing at runtime")
endif()

if(TARGET Qt6::Widgets)
    target_link_libraries(fossredder_ui PUBLIC Qt6::Widgets)
endif()

if(TARGET Qt6::Concurrent)
    target_link_libraries(fossredder_ui PUBLIC Qt6::Concurrent)
else()
    message(WARNING "Qt6 Concurrent not found; QtConcurrent features may be unavailable")
endif()

if(TARGET Qt6::Svg)
    target_link_libraries(fossredder_ui PUBLIC Qt6::Svg)
else()
    message(WARNING "Qt6 Svg not found; QML QtQuick.Svg module may be unavailable at runtime")
endif()

target_link_libraries(fossredder_ui
    PUBLIC
        FossRedder::Core
        FossRedder::Debug
)

fossredder_enable_target_pch(fossredder_ui "${CMAKE_CURRENT_SOURCE_DIR}/include/ui/pch.h")

# Optional module-local UI test blocks.
# Enable this to build QML-near Quick Test suites against the real UI module.
option(BUILD_UI_TESTS "Build tests for ui module" OFF)
if(BUILD_UI_TESTS)
    find_package(Qt6 COMPONENTS QuickTest Test REQUIRED)
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/qml" UI_QML_IMPORT_DIR)
    file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tests/qml" UI_QML_TEST_SOURCE_DIR)

    set(UI_UNIT_TEST_SRCS
        tests/unit/TestAnalysisPayloadMapper.cpp
        tests/unit/TestAnalysisState.cpp
        tests/unit/TestAnalysisWorkflow.cpp
        tests/unit/TestAnnualState.cpp
        tests/unit/TestAnnualWorkflow.cpp
        tests/unit/TestBookingState.cpp
        tests/unit/TestExportState.cpp
        tests/unit/TestExportWorkflow.cpp
        tests/unit/TestImportWorkflow.cpp
        tests/unit/TestImportSuggestionService.cpp
        tests/unit/TestNavigationState.cpp
        tests/unit/TestSelectionState.cpp
        tests/unit/TestSettingsState.cpp
        tests/unit/TestSessionModels.cpp
        tests/unit/TestSessionMutationState.cpp
        tests/unit/TestWorkspaceStartupRehydration.cpp
        tests/unit/TestWorkspaceFacade.cpp
        tests/unit/TestWorkspaceRowProjector.cpp
        tests/unit/TestWorkspaceSessionSelection.cpp
        tests/unit/TestWorkspaceSessionState.cpp
    )

    add_executable(ui_unit_tests ${UI_UNIT_TEST_SRCS})
    target_link_libraries(ui_unit_tests
        PRIVATE
            GTest::gtest_main
            FossRedder::Ui
    )
    target_include_directories(ui_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    target_compile_features(ui_unit_tests PRIVATE cxx_std_20)
    fossredder_set_runtime_output_dirs(ui_unit_tests)
    fossredder_enable_target_quality(ui_unit_tests)
    gtest_discover_tests(ui_unit_tests PROPERTIES LABELS "UNIT")

    set(UI_INTERACTION_TEST_SRCS
        tests/interaction/TestImportState.cpp
    )

    add_executable(ui_interaction_tests ${UI_INTERACTION_TEST_SRCS})
    target_link_libraries(ui_interaction_tests
        PRIVATE
            GTest::gtest_main
            FossRedder::Ui
    )
    target_compile_features(ui_interaction_tests PRIVATE cxx_std_20)
    fossredder_set_runtime_output_dirs(ui_interaction_tests)
    fossredder_enable_target_quality(ui_interaction_tests)
    gtest_discover_tests(ui_interaction_tests PROPERTIES LABELS "INTERACTION")

    add_executable(ui_qml_tests
        tests/qml/qmltests.cpp
    )

    set_target_properties(ui_qml_tests PROPERTIES
        AUTOMOC ON
    )

    target_compile_features(ui_qml_tests PRIVATE cxx_std_20)
    target_compile_definitions(ui_qml_tests PRIVATE
        FOSSREDDER_UI_QML_IMPORT_DIR="${UI_QML_IMPORT_DIR}"
        QUICK_TEST_SOURCE_DIR="${UI_QML_TEST_SOURCE_DIR}"
    )

    target_link_libraries(ui_qml_tests
        PRIVATE
            Qt6::QuickTest
            Qt6::Test
            FossRedder::Ui
    )

    fossredder_set_runtime_output_dirs(ui_qml_tests)
    fossredder_enable_target_quality(ui_qml_tests)
    fossredder_configure_qt_runtime(ui_qml_tests
        QML_SOURCE_DIR "${CMAKE_SOURCE_DIR}/ui/qml/FossRedder"
        QML_TOOLING_DIR "${CMAKE_BINARY_DIR}/ui/qmltooling/FossRedder"
    )

    add_test(NAME ui_qml_tests COMMAND ui_qml_tests -platform windows)
    set_tests_properties(ui_qml_tests PROPERTIES
        LABELS "UI;QML"
        ENVIRONMENT "QML_DISABLE_DISK_CACHE=1;QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:ui_qml_tests>/platforms;QT_PLUGIN_PATH=$<TARGET_FILE_DIR:ui_qml_tests>;QML2_IMPORT_PATH=$<TARGET_FILE_DIR:ui_qml_tests>/qml;QML_IMPORT_PATH=$<TARGET_FILE_DIR:ui_qml_tests>/qml"
    )
endif()

if(TARGET Qt6::Quick AND TARGET Qt6::Qml)
    foreach(_qml_target IN ITEMS
        fossredder_ui_qml_controls
        fossredder_ui_qml_components
        fossredder_ui_qml_views
        fossredder_ui_qml_utils
    )
        if(TARGET ${_qml_target})
            fossredder_install_library_target(${_qml_target})
        endif()
    endforeach()

    foreach(_qml_resource_base IN ITEMS
        fossredder_ui
        fossredder_ui_qml_controls
        fossredder_ui_qml_components
        fossredder_ui_qml_views
        fossredder_ui_qml_utils
    )
        foreach(_resource_index RANGE 1 8)
            if(TARGET ${_qml_resource_base}_resources_${_resource_index})
                fossredder_install_library_target(${_qml_resource_base}_resources_${_resource_index})
            endif()
        endforeach()
    endforeach()

endif()

fossredder_install_library_target(fossredder_ui)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")
