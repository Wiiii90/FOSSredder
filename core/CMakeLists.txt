set(CORE_PUBLIC_HEADERS
    include/core/application/analysis/AnalysisService.h
    include/core/application/analysis/AnalysisWorkflowSupport.h
    include/core/application/annual/AnnualService.h
    include/core/application/export/CsvExporter.h
    include/core/application/export/ExportLog.h
    include/core/application/export/ExportService.h
    include/core/application/export/XlsxExporter.h
    include/core/application/import/IImportStatement.h
    include/core/application/import/ImportLog.h
    include/core/application/import/ImportRequest.h
    include/core/application/import/ImportResult.h
    include/core/application/import/StatementImportJobSystem.h
    include/core/application/import/StatementImportRunner.h
    include/core/application/import/draft/DraftFinalizer.h
    include/core/application/import/draft/DraftMatcher.h
    include/core/application/import/draft/StatementDraft.h
    include/core/application/import/draft/TransactionDraft.h
    include/core/application/import/transaction/AmountParser.h
    include/core/application/storage/DeletionImpact.h
    include/core/application/storage/RepositoryBundle.h
    include/core/application/storage/StorageManager.h
    include/core/application/workspace/WorkspaceFacade.h
    include/core/application/workspace/WorkspaceSnapshotCatalogMapper.h
    include/core/application/workspace/WorkspaceQueryService.h
    include/core/application/workspace/WorkspaceSession.h
    include/core/application/workspace/WorkspaceStateManager.h
    include/core/ports/infra/archive/IArchive.h
    include/core/ports/usecases/analysis/AnalysisRequest.h
    include/core/ports/usecases/analysis/AnalysisResult.h
    include/core/ports/usecases/analysis/IAnalysisRunner.h
    include/core/ports/infra/analysis-image-renderer/IAnalysisImageRenderer.h
    include/core/ports/usecases/annual/AnnualRequest.h
    include/core/ports/usecases/annual/AnnualResult.h
    include/core/ports/usecases/annual/IAnnualRunner.h
    include/core/ports/usecases/export/ExportRequest.h
    include/core/ports/usecases/export/ExportResult.h
    include/core/ports/usecases/export/IExportRunner.h
    include/core/ports/usecases/import/IImportRunner.h
    include/core/ports/usecases/import/ImportRequest.h
    include/core/ports/usecases/import/ImportResult.h
    include/core/ports/infra/xlsx-writer/IXlsxWriter.h
    include/core/constants/analysis.h
    include/core/constants/app.h
    include/core/constants/app_state.h
    include/core/constants/export.h
    include/core/constants/filters.h
    include/core/constants/ids.h
    include/core/constants/import.h
    include/core/constants/json.h
    include/core/constants/jobs.h
    include/core/constants/localization.h
    include/core/constants/parser.h
    include/core/constants/preferences.h
    include/core/constants/runtime.h
    include/core/domain/entities/Actor.h
    include/core/domain/entities/Analysis.h
    include/core/domain/entities/Annual.h
    include/core/domain/entities/Contract.h
    include/core/domain/entities/Property.h
    include/core/domain/entities/Statement.h
    include/core/domain/entities/Transaction.h
    include/core/domain/policies/AnalysisPolicy.h
    include/core/domain/policies/AnnualPolicy.h
    include/core/domain/policies/DraftMatchingPolicy.h
    include/core/domain/policies/TransactionPolicy.h
    include/core/domain/policies/StatementPolicy.h
    include/core/domain/values/Alias.h
    include/core/errors/ErrorCodes.h
    include/core/errors/ErrorEvent.h
    include/core/errors/ErrorEventFactory.h
    include/core/errors/ErrorReporting.h
    include/core/ports/diagnostics/IDiagnostics.h
    include/core/ports/diagnostics/IErrorReporter.h
    include/core/jobs/JobSystem.h
    include/core/jobs/JobTypes.h
    include/core/jobs/Scheduler.h
    include/core/pch.h
    include/core/ports/repositories/IActorRepository.h
    include/core/ports/repositories/IAnalysisRepository.h
    include/core/ports/repositories/IAnnualRepository.h
    include/core/ports/repositories/IContractRepository.h
    include/core/ports/repositories/IExportLogRepository.h
    include/core/ports/repositories/IImportLogRepository.h
    include/core/ports/repositories/IPropertyRepository.h
    include/core/ports/repositories/IStatementDraftRepository.h
    include/core/ports/repositories/IStatementRepository.h
    include/core/ports/repositories/ITransactionDraftRepository.h
    include/core/ports/repositories/ITransactionRepository.h
    include/core/ports/infra/image-processing/IImageProcessor.h
    include/core/ports/infra/image-processing/ImageProcessingRequest.h
    include/core/ports/infra/image-processing/ImageProcessingResult.h
    include/core/ports/infra/image-processing/ImageProcessingTypes.h
    include/core/ports/infra/pdf-rendering/IPdfRenderer.h
    include/core/ports/infra/pdf-rendering/PdfRenderingRequest.h
    include/core/ports/infra/pdf-rendering/PdfRenderingResult.h
    include/core/ports/infra/pdf-rendering/PdfRenderingTypes.h
    include/core/ports/infra/text-recognition/ITextRecognizer.h
    include/core/ports/infra/text-recognition/TextRecognitionRequest.h
    include/core/ports/infra/text-recognition/TextRecognitionResult.h
    include/core/ports/infra/text-recognition/TextRecognitionTypes.h
    include/core/ports/infra/storage/IRegistry.h
    include/core/ports/infra/storage/IStorageManager.h
    include/core/utils/Time.h
)

option(BUILD_CORE_TESTS "Build core tests" OFF)

set(CORE_PRIVATE_HEADERS
    src/application/analysis/internal/AnalysisFilter.h
    src/application/analysis/presentation/PlotAnalysis.h
    src/application/analysis/presentation/TableAnalysis.h
    src/application/analysis/adjustment/AdjustmentCalculation.h
    src/application/export/PropertyContractMatrix.h
    src/application/import/ImportPageRequests.h
    src/application/import/ImportPipelineHelpers.h
    src/application/import/ImportStatementStrategy.h
    src/application/import/ImportStrategySupport.h
    src/jobs/JobManager.h
    src/utils/StableId.h
    src/utils/UniqId.h
    src/utils/Util.h
)

# Core implementation sources.
set(CORE_SRCS
    src/application/analysis/AnalysisRequest.cpp
    src/application/analysis/AnalysisWorkflowSupport.cpp
    src/application/analysis/AnalysisService.cpp
    src/application/annual/AnnualService.cpp
    src/application/annual/AnnualWorkflowSupport.cpp
    src/application/analysis/internal/AnalysisFilter.cpp
    src/application/analysis/presentation/PlotAnalysis.cpp
    src/application/analysis/presentation/TableAnalysis.cpp
    src/application/analysis/adjustment/AdjustmentCalculation.cpp
    src/application/export/ObjectExportExecutor.cpp
    src/application/export/CsvExporter.cpp
    src/application/export/ExportService.cpp
    src/application/export/XlsxExporter.cpp
    src/application/import/DefaultImportStatementStrategy.cpp
    src/application/import/draft/DraftFinalizer.cpp
    src/application/import/draft/DraftMatcherProjection.cpp
    src/application/import/draft/DraftMatcherResolution.cpp
    src/application/import/draft/DraftMatcherSignals.cpp
    src/application/import/ImportPipelineHelpers.cpp
    src/application/import/ImportStatement.cpp
    src/application/import/StatementImportJobSystem.cpp
    src/application/import/StatementImportRunner.cpp
    src/application/import/ImportStrategySupport.cpp
    src/application/import/internal/ParserDateUtils.cpp
    src/application/import/internal/ParserHelpers.cpp
    src/application/import/internal/ParserHeuristics.cpp
    src/application/import/statement/DefaultStatementParser.cpp
    src/application/import/statement/StatementParseHelpers.cpp
    src/application/import/transaction/AmountParser.cpp
    src/application/import/transaction/DefaultTransactionParser.cpp
    src/application/storage/StorageManager.cpp
    src/application/workspace/WorkspaceCommandService.cpp
    src/application/workspace/WorkspaceFacade.cpp
    src/application/workspace/WorkspaceWorkflowService.cpp
    src/application/workspace/WorkspaceSnapshotProjector.cpp
    src/application/workspace/WorkspaceSnapshotCatalogMapper.cpp
    src/application/workspace/WorkspaceQueryService.cpp
    src/application/workspace/WorkspaceSession.cpp
    src/application/workspace/WorkspaceStateManager.cpp
    src/jobs/JobManager.cpp
    src/jobs/JobSystem.cpp
    src/jobs/Scheduler.cpp
    src/domain/entities/Actor.cpp
    src/domain/entities/Analysis.cpp
    src/domain/entities/Annual.cpp
    src/domain/entities/Contract.cpp
    src/domain/entities/Property.cpp
    src/domain/entities/Statement.cpp
    src/domain/entities/Transaction.cpp
    src/domain/catalog/WorkspaceCatalog.cpp
    src/domain/policies/AliasPolicy.cpp
    src/domain/policies/AnalysisPolicy.cpp
    src/domain/policies/AnnualPolicy.cpp
    src/domain/policies/StatementPolicy.cpp
    src/domain/policies/TransactionPolicy.cpp
    src/domain/values/Alias.cpp
    src/domain/values/AnalysisType.cpp
    src/domain/values/BookingDate.cpp
    src/domain/values/ContractType.cpp
    src/domain/values/EntityName.cpp
    src/domain/values/ExportFormat.cpp
    src/domain/values/FilterSpec.cpp
    src/domain/values/MoneyAmount.cpp
    src/domain/values/Year.cpp
    src/domain/policies/DraftMatchingPolicy.cpp
    src/utils/Util.cpp
    ${CORE_PUBLIC_HEADERS}
    ${CORE_PRIVATE_HEADERS}
)

add_library(fossredder_core STATIC ${CORE_SRCS})
add_library(FossRedder::Core ALIAS fossredder_core)
fossredder_enable_target_quality(fossredder_core)

set_target_properties(fossredder_core PROPERTIES EXPORT_NAME Core)

target_compile_features(fossredder_core PUBLIC cxx_std_20)

target_include_directories(fossredder_core PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Internal dependencies used by core implementation code.
find_package(nlohmann_json CONFIG REQUIRED)

target_link_libraries(fossredder_core PRIVATE nlohmann_json::nlohmann_json)

fossredder_set_library_output_dirs(fossredder_core)
fossredder_enable_target_pch(fossredder_core "${CMAKE_CURRENT_SOURCE_DIR}/include/core/pch.h")

fossredder_install_library_target(fossredder_core)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")

if(BUILD_CORE_TESTS)
    add_subdirectory(tests)
endif()
