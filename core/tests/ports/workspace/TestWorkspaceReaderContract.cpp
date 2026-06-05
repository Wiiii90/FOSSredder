/**
 * @file core/tests/ports/workspace/TestWorkspaceReaderContract.cpp
 * @brief Tests for the workspace reader boundary contract.
 */

#include <gtest/gtest.h>

#include "core/ports/workspace/IWorkspaceReader.h"

namespace core::ports::workspace {

namespace {

class ReaderSpy final : public IWorkspaceReader {
public:
    WorkspaceSnapshot snapshot;
    std::optional<StatementDraftSnapshot> draft;

    WorkspaceSnapshot workspaceSnapshot() const override {
        return snapshot;
    }

    std::string currentPath() const override {
        return snapshot.hasCurrentPath ? snapshot.currentPath : std::string{};
    }

    std::optional<StatementDraftSnapshot> statementDraftSnapshot(const std::string& = {}) const override {
        return draft;
    }

    WorkspaceIdentitySnapshot actorIdentityByName(const std::string&) const override {
        return {};
    }

    WorkspaceIdentitySnapshot propertyIdentityByName(const std::string&) const override {
        return {};
    }

    WorkspaceIdentitySnapshot contractIdentityBySignature(
        const std::string&,
        const std::string&,
        const std::vector<std::string>&,
        const std::vector<std::string>&) const override {
        return {};
    }

    TransactionCatalogSelection transactionCatalogSelection(
        const TransactionCatalogSelectionChange& change) const override {
        return change.current;
    }

    std::string nextContractName() const override {
        return "Contract 1";
    }

};

} // namespace

TEST(WorkspaceReaderContractTest, ReturnsSnapshotAndDraftState) {
    ReaderSpy reader;
    reader.snapshot.currentPath = "P:/workspace.db";
    reader.snapshot.hasCurrentPath = true;
    reader.draft = StatementDraftSnapshot{};
    reader.draft->id = "draft-1";

    const auto snapshot = reader.workspaceSnapshot();
    const auto path = reader.currentPath();
    const auto draft = reader.statementDraftSnapshot();

    EXPECT_TRUE(snapshot.hasCurrentPath);
    EXPECT_EQ(snapshot.currentPath, "P:/workspace.db");
    EXPECT_EQ(path, "P:/workspace.db");
    ASSERT_TRUE(draft.has_value());
    EXPECT_EQ(draft->id, "draft-1");
}

} // namespace core::ports::workspace
