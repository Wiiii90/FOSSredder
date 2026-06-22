find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

set(PERSISTENCE_TEST_SRCS
    unit/TestSqliteDb.cpp
    unit/TestSqliteSchema.cpp
    unit/TestSqliteTransaction.cpp
    unit/TestSqliteRegistry.cpp
    unit/TestFactory.cpp
    repositories/TestPropertyRepository.cpp
    repositories/TestContractRepository.cpp
    repositories/TestStatementRepository.cpp
    repositories/TestTransactionRepository.cpp
    repositories/TestAnalysisRepository.cpp
    repositories/TestAnnualRepository.cpp
    repositories/TestStatementDraftRepository.cpp
    repositories/TestTransactionDraftRepository.cpp
    repositories/TestImportLogRepository.cpp
    repositories/TestExportLogRepository.cpp
    repositories/TestActorRepository.cpp
    state/TestWorkspaceStateStore.cpp
)
add_executable(persistence_unit_tests ${PERSISTENCE_TEST_SRCS})
target_include_directories(persistence_unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
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
