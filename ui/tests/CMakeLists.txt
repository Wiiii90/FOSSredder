find_package(Qt6 COMPONENTS QuickTest Test REQUIRED)
find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

get_filename_component(UI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
get_filename_component(UI_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/.." ABSOLUTE)
file(TO_CMAKE_PATH "${UI_SOURCE_DIR}/qml" UI_QML_IMPORT_DIR)
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/qml" UI_QML_TEST_SOURCE_DIR)

set(UI_UNIT_TEST_SRCS
    unit/adapters/TestAnalysisAdapter.cpp
    unit/adapters/TestAnnualAdapter.cpp
    unit/adapters/TestExportAdapter.cpp
    unit/adapters/TestImportAdapter.cpp
    unit/shell/TestNavigation.cpp
    unit/viewmodels/TestActorViewModel.cpp
    unit/viewmodels/TestAnalysisViewModel.cpp
    unit/viewmodels/TestAnnualViewModel.cpp
    unit/viewmodels/TestBookingViewModel.cpp
    unit/viewmodels/TestContractViewModel.cpp
    unit/viewmodels/TestExportViewModel.cpp
    unit/viewmodels/TestImportViewModel.cpp
    unit/viewmodels/TestPropertyViewModel.cpp
    unit/viewmodels/TestSettingsViewModel.cpp
    unit/workflows/TestAnalysisWorkflow.cpp
    unit/workflows/TestAnnualWorkflow.cpp
    unit/workflows/TestExportWorkflow.cpp
    unit/workflows/TestImportWorkflow.cpp
    unit/workspace/TestWorkspaceCommands.cpp
    unit/workspace/TestWorkspaceSelection.cpp
    unit/workspace/TestWorkspaceSelectors.cpp
    unit/workspace/TestWorkspaceStore.cpp
)

add_executable(ui_unit_tests ${UI_UNIT_TEST_SRCS})
target_link_libraries(ui_unit_tests
    PRIVATE
        GTest::gtest_main
        FossRedder::Ui
)
target_include_directories(ui_unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(ui_unit_tests PRIVATE
    FOSSREDDER_UI_SOURCE_DIR="${UI_SOURCE_DIR}"
)
target_compile_features(ui_unit_tests PRIVATE cxx_std_20)
fossredder_set_runtime_output_dirs(ui_unit_tests)
fossredder_enable_target_quality(ui_unit_tests)
gtest_discover_tests(ui_unit_tests PROPERTIES LABELS "UNIT")

set(UI_INTERACTION_TEST_SRCS
    interaction/TestFeatureWiring.cpp
    interaction/TestImportState.cpp
)

add_executable(ui_interaction_tests ${UI_INTERACTION_TEST_SRCS})
target_link_libraries(ui_interaction_tests
    PRIVATE
        GTest::gtest_main
        FossRedder::Ui
)
target_compile_features(ui_interaction_tests PRIVATE cxx_std_20)
target_include_directories(ui_interaction_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
fossredder_set_runtime_output_dirs(ui_interaction_tests)
fossredder_enable_target_quality(ui_interaction_tests)
gtest_discover_tests(ui_interaction_tests PROPERTIES LABELS "INTERACTION")

add_executable(ui_qml_tests
    qml/qmltests.cpp
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
    QML_SOURCE_DIR "${UI_SOURCE_DIR}/qml/FossRedder"
    QML_TOOLING_DIR "${UI_BINARY_DIR}/qmltooling/FossRedder"
)

add_test(NAME ui_qml_tests COMMAND ui_qml_tests -platform windows)
set_tests_properties(ui_qml_tests PROPERTIES
    LABELS "UI;QML"
    ENVIRONMENT "QML_DISABLE_DISK_CACHE=1;QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:ui_qml_tests>/platforms;QT_PLUGIN_PATH=$<TARGET_FILE_DIR:ui_qml_tests>;QML2_IMPORT_PATH=$<TARGET_FILE_DIR:ui_qml_tests>/qml;QML_IMPORT_PATH=$<TARGET_FILE_DIR:ui_qml_tests>/qml"
)
