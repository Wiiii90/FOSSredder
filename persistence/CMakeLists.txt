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
    add_subdirectory(tests)
endif()
