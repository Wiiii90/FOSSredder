find_package(GTest CONFIG REQUIRED)

set(CORE_TEST_SRCS
    application/analysis/TestAnalysisService.cpp
    application/annual/TestAnnualService.cpp
    application/export/TestExportService.cpp
    application/workspace/TestWorkspaceCommandService.cpp
    application/workspace/TestWorkspaceFacade.cpp
    application/import/draft/TestDraftFinalizer.cpp
    application/import/draft/TestDraftMatcher.cpp
    application/import/draft/TestDraftMatcherProjection.cpp
    application/workspace/TestWorkspaceQueryService.cpp
    application/workspace/TestWorkspaceSnapshotProjector.cpp
    application/workspace/TestWorkspaceStateManager.cpp
    application/workspace/TestWorkspaceWorkflowService.cpp
    domain/catalog/TestWorkspaceCatalog.cpp
    domain/entities/TestActor.cpp
    domain/entities/TestAnalysis.cpp
    domain/entities/TestAnnual.cpp
    domain/entities/TestContract.cpp
    domain/entities/TestProperty.cpp
    domain/entities/TestStatement.cpp
    domain/entities/TestTransaction.cpp
    domain/policies/TestAliasPolicy.cpp
    domain/policies/TestAnalysisPolicy.cpp
    domain/policies/TestAnnualPolicy.cpp
    domain/policies/TestDraftMatchingPolicy.cpp
    domain/policies/TestStatementPolicy.cpp
    domain/policies/TestTransactionPolicy.cpp
    domain/values/TestAlias.cpp
    domain/values/TestAnalysisType.cpp
    domain/values/TestBookingDate.cpp
    domain/values/TestContractType.cpp
    domain/values/TestEntityName.cpp
    domain/values/TestExportFormat.cpp
    domain/values/TestFilterSpec.cpp
    domain/values/TestMoneyAmount.cpp
    domain/values/TestYear.cpp
    ports/workspace/TestWorkspaceReaderContract.cpp
    ports/workspace/TestWorkspaceWriterContract.cpp
)

add_executable(core_tests ${CORE_TEST_SRCS})
target_link_libraries(core_tests PRIVATE fossredder_core GTest::gtest_main)
target_compile_features(core_tests PRIVATE cxx_std_20)
target_include_directories(core_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
fossredder_enable_target_quality(core_tests)
fossredder_set_runtime_output_dirs(core_tests)

include(GoogleTest)
gtest_discover_tests(core_tests PROPERTIES LABELS "CORE")
