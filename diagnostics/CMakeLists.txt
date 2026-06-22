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
    add_subdirectory(tests)
endif()
