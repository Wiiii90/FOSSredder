# Persistence implementation sources.
set(PERSISTENCE_SRCS
    src/WorkspaceStateStore.cpp
    src/Factory.cpp
    src/SqliteDb.cpp
    src/SqliteRegistry.cpp
    src/SqliteSchema.cpp
    src/SqliteTransaction.cpp
    src/repositories/SqliteActorRepository.cpp
    src/repositories/SqliteAnalysisRepository.cpp
    src/repositories/SqliteAnnualRepository.cpp
    src/repositories/SqliteContractRepository.cpp
    src/repositories/SqliteImportLogRepository.cpp
    src/repositories/SqliteExportLogRepository.cpp
    src/repositories/SqliteStatementDraftRepository.cpp
    src/repositories/SqlitePropertyRepository.cpp
    src/repositories/SqliteStatementRepository.cpp
    src/repositories/SqliteTransactionRepository.cpp
    src/repositories/SqliteTransactionDraftRepository.cpp
)

add_library(fossredder_persistence STATIC ${PERSISTENCE_SRCS})
add_library(FossRedder::Persistence ALIAS fossredder_persistence)
fossredder_enable_target_quality(fossredder_persistence)

set_target_properties(fossredder_persistence PROPERTIES EXPORT_NAME Persistence)

target_compile_features(fossredder_persistence PUBLIC cxx_std_20)

target_include_directories(fossredder_persistence PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Persistence implementation depends on SQLite and JSON serialization.
find_package(nlohmann_json CONFIG REQUIRED)
find_package(SQLite3 REQUIRED)

target_link_libraries(fossredder_persistence
    PUBLIC
        FossRedder::Core
    PRIVATE
        nlohmann_json::nlohmann_json
        SQLite::SQLite3
)

fossredder_set_library_output_dirs(fossredder_persistence)
fossredder_enable_target_pch(fossredder_persistence "${CMAKE_CURRENT_SOURCE_DIR}/include/persistence/pch.h")

fossredder_install_library_target(fossredder_persistence)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")

# Optional module-local tests.
option(BUILD_PERSISTENCE_TESTS "Build tests for persistence module" OFF)
if (BUILD_PERSISTENCE_TESTS)
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    set(PERSISTENCE_TEST_SRCS
        tests/unit/TestSqliteDb.cpp
        tests/unit/TestSqliteSchema.cpp
        tests/unit/TestSqliteTransaction.cpp
        tests/unit/TestSqliteRegistry.cpp
        tests/unit/TestFactory.cpp
        tests/repositories/TestPropertyRepository.cpp
        tests/repositories/TestContractRepository.cpp
        tests/repositories/TestStatementRepository.cpp
        tests/repositories/TestTransactionRepository.cpp
        tests/repositories/TestAnalysisRepository.cpp
        tests/repositories/TestAnnualRepository.cpp
        tests/repositories/TestStatementDraftRepository.cpp
        tests/repositories/TestTransactionDraftRepository.cpp
        tests/repositories/TestImportLogRepository.cpp
        tests/repositories/TestExportLogRepository.cpp
        tests/repositories/TestActorRepository.cpp
        tests/state/TestWorkspaceStateStore.cpp
    )
    add_executable(persistence_unit_tests ${PERSISTENCE_TEST_SRCS})
    target_include_directories(persistence_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/core/include
    )
    target_link_libraries(persistence_unit_tests PRIVATE
        GTest::gtest_main
        FossRedder::Persistence
    )
    target_compile_features(persistence_unit_tests PRIVATE cxx_std_20)

    fossredder_set_runtime_output_dirs(persistence_unit_tests)
    fossredder_enable_target_quality(persistence_unit_tests)

    gtest_discover_tests(persistence_unit_tests PROPERTIES LABELS "PERSISTENCE_UNIT")
endif()
