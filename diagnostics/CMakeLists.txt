# Diagnostics infrastructure sources.
set(DIAGNOSTICS_SRCS
    src/ErrorReporter.cpp
    src/FileDiagnostics.cpp
    src/RunManager.cpp
    src/SpdlogDiagnostics.cpp
)

add_library(fossredder_diagnostics STATIC ${DIAGNOSTICS_SRCS})
add_library(FossRedder::Diagnostics ALIAS fossredder_diagnostics)
fossredder_enable_target_quality(fossredder_diagnostics)

set_target_properties(fossredder_diagnostics PROPERTIES EXPORT_NAME Diagnostics)

target_compile_features(fossredder_diagnostics PUBLIC cxx_std_20)

target_include_directories(fossredder_diagnostics PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Public diagnostics headers depend on core interfaces.
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(fossredder_diagnostics
    PUBLIC
        FossRedder::Core
    PRIVATE
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)

fossredder_set_library_output_dirs(fossredder_diagnostics)

fossredder_install_library_target(fossredder_diagnostics)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")

# Optional module-local tests.
option(BUILD_DIAGNOSTICS_TESTS "Build tests for diagnostics module" OFF)
if (BUILD_DIAGNOSTICS_TESTS)
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    set(DIAGNOSTICS_TEST_SRCS
        tests/unit/TestDiagnosticsErrorReporter.cpp
    )
    add_executable(diagnostics_unit_tests ${DIAGNOSTICS_TEST_SRCS})
    target_include_directories(diagnostics_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
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
endif()
