/**
 * @file core/tests/ports/workspace/TestWorkspaceWriterContract.cpp
 * @brief Tests for the workspace writer boundary contract.
 */

#include <gtest/gtest.h>

#include "core/ports/workspace/IWorkspaceWriter.h"

namespace core::ports::workspace {

namespace {

class WriterSpy final : public IWorkspaceWriter {
public:
    SnapshotChanged snapshotChanged;
    std::string lastAddedActor;

    void setSnapshotChangedCallback(SnapshotChanged cb) override {
        snapshotChanged = std::move(cb);
    }

    void setErrorReporter(std::shared_ptr<core::errors::IErrorReporter>) override {}
    void setAtomicStoreSave(core::ports::storage::IStorageManager::AtomicStoreSave) override {}
    void setAtomicStoreLoad(core::ports::storage::IStorageManager::AtomicStoreLoad) override {}
    void setDeletionImpactCallback(core::ports::workspace::IWorkspaceWriter::DeletionImpactCallback) override {}
    void openLatest() override {}
    void newFile(const std::string&) override {}
    void openFile(const std::string&) override {}
    void saveFile() override {}
    void saveFileAs(const std::string&) override {}
    void commit() override {}
    void notifySnapshot() override {}

    ValidationResult validateActor(const ActorCommand&) const override { return {}; }
    ValidationResult validateProperty(const PropertyCommand&) const override { return {}; }
    ValidationResult validateContract(const ContractCommand&) const override { return {}; }
    ValidationResult validateStatement(const StatementCommand&) const override { return {}; }
    ValidationResult validateTransaction(const TransactionCommand&) const override { return {}; }
    ValidationResult validateAnalysis(const AnalysisCommand&) const override { return {}; }
    ValidationResult validateAnnual(const AnnualCommand&) const override { return {}; }

    std::string addActor(const ActorCommand& command) override {
        lastAddedActor = command.name;
        return command.id.empty() ? "actor-1" : command.id;
    }
    void updateActor(const ActorCommand&) override {}
    void deleteActor(const std::string&) override {}
    std::string addProperty(const PropertyCommand&) override { return {}; }
    void updateProperty(const PropertyCommand&) override {}
    void deleteProperty(const std::string&) override {}
    std::string addContract(const ContractCommand&) override { return {}; }
    void updateContract(const ContractCommand&) override {}
    void deleteContract(const std::string&) override {}
    std::string addStatement(const StatementCommand&) override { return {}; }
    void updateStatement(const StatementCommand&) override {}
    void deleteStatement(const std::string&) override {}
    std::string addTransaction(const TransactionCommand&) override { return {}; }
    void updateTransaction(const TransactionCommand&) override {}
    void deleteTransaction(const std::string&) override {}
    std::string addAnalysis(const AnalysisCommand&) override { return {}; }
    void updateAnalysis(const AnalysisCommand&) override {}
    void deleteAnalysis(const std::string&) override {}
    std::string addAnnual(const AnnualCommand&) override { return {}; }
    void updateAnnual(const AnnualCommand&) override {}
    void deleteAnnual(const std::string&) override {}
    std::string finalizeStatementDraft(const FinalizeStatementDraftCommand&) override { return {}; }
    void saveStatementDraft(const StatementDraftCommand&) override {}
    void clearStatementDraft(const std::string&) override {}
    void setImportLogs(const ImportLogsCommand&) override {}
    void saveImportLog(const ImportLogCommand&) override {}
    void deleteImportLog(const std::string&) override {}
    void clearImportLogs() override {}
    void setExportLogs(const ExportLogsCommand&) override {}
    void saveExportLog(const ExportLogCommand&) override {}
    void deleteExportLog(const std::string&) override {}
    void clearExportLogs() override {}
};

} // namespace

TEST(WorkspaceWriterContractTest, StoresCallbacksAndRoutesCommands) {
    WriterSpy writer;
    bool called = false;
    writer.setSnapshotChangedCallback([&](const WorkspaceSnapshot&) {
        called = true;
    });

    const auto id = writer.addActor(ActorCommand{.id = {}, .name = "Alpha"});

    EXPECT_EQ(id, "actor-1");
    EXPECT_EQ(writer.lastAddedActor, "Alpha");
    ASSERT_TRUE(writer.snapshotChanged);
    writer.snapshotChanged(WorkspaceSnapshot{});
    EXPECT_TRUE(called);
}

} // namespace core::ports::workspace
