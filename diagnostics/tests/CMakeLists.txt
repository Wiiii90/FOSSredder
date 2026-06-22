find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

set(DIAGNOSTICS_TEST_SRCS
    unit/TestDiagnosticsErrorReporter.cpp
)
add_executable(diagnostics_unit_tests ${DIAGNOSTICS_TEST_SRCS})
target_include_directories(diagnostics_unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}/core/include
)
target_link_libraries(diagnostics_unit_tests PRIVATE
    GTest::gtest_main
    FossRedder::Diagnostics
)
target_compile_features(diagnostics_unit_tests PRIVATE cxx_std_20)

fossredder_set_runtime_output_dirs(diagnostics_unit_tests)
fossredder_enable_target_quality(diagnostics_unit_tests)

gtest_discover_tests(diagnostics_unit_tests PROPERTIES LABELS "DIAGNOSTICS_UNIT")
