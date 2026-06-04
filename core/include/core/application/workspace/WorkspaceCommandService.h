/**
 * @file core/include/core/application/workspace/WorkspaceCommandService.h
 * @brief Applies catalog commands to workspace session state and persists commits.
 */

#pragma once

#include <string>
#include <vector>

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Transaction.h"
#include "core/domain/values/Alias.h"
#include "core/domain/values/AnalysisType.h"
#include "core/domain/values/BookingDate.h"
#include "core/domain/values/ExportFormat.h"
#include "core/domain/values/FilterSpec.h"
#include "core/ports/workspace/WorkspaceCommands.h"

namespace core::application {

class WorkspaceSession;

struct ActorInput {
    std::string name;
    std::vector<core::domain::Alias> aliases;
    std::vector<std::string> contractIds;
};

struct PropertyInput {
    std::string name;
    std::vector<core::domain::Alias> aliases;
    std::vector<std::string> contractIds;
};

struct ContractInput {
    std::string name;
    std::string type;
    std::string allocatableMode = "mixed";
    std::vector<std::string> actorIds;
    std::vector<std::string> propertyIds;
    std::vector<core::domain::Alias> aliases;
};

struct TransactionInput {
    std::string name;
    core::domain::BookingDate bookingDate;
    std::string valuta;
    double amount = 0.0;
    std::string statementId;
    std::string insertAfterTransactionId;
    core::domain::Transaction::Status status = core::domain::Transaction::Status::Neutral;
    std::string actorId;
    std::string contractId;
    bool allocatable = false;
    std::vector<std::string> propertyIds;
};

struct AnalysisInput {
    std::string name;
    core::domain::AnalysisType type;
    std::string configJson;
    core::domain::FilterSpec filterSpec;
    core::domain::ExportFormat exportFormat;
    bool includeCalculationAdjustments = true;
    std::string exportStateJson;
    std::string snapshotTransactionsJson;
    std::vector<std::pair<std::string, double>> adjustments;
};

class WorkspaceCommandService {
public:
    /** @brief Creates command service bound to one workspace session. */
    explicit WorkspaceCommandService(WorkspaceSession& session);
    ~WorkspaceCommandService();

    WorkspaceCommandService(const WorkspaceCommandService&) = delete;
    WorkspaceCommandService& operator=(const WorkspaceCommandService&) = delete;
    WorkspaceCommandService(WorkspaceCommandService&&) noexcept;
    WorkspaceCommandService& operator=(WorkspaceCommandService&&) noexcept;

    /** @brief Adds actor and returns created id. */
    [[nodiscard]] std::string addActor(const core::ports::workspace::ActorCommand& command);
    /** @brief Updates actor identified by command id. */
    void updateActor(const core::ports::workspace::ActorCommand& command);
    /** @brief Deletes actor by id. */
    void deleteActor(const std::string& id);

    /** @brief Adds property and returns created id. */
    [[nodiscard]] std::string addProperty(const core::ports::workspace::PropertyCommand& command);
    /** @brief Updates property identified by command id. */
    void updateProperty(const core::ports::workspace::PropertyCommand& command);
    /** @brief Deletes property by id. */
    void deleteProperty(const std::string& id);

    /** @brief Adds contract and returns created id. */
    [[nodiscard]] std::string addContract(const core::ports::workspace::ContractCommand& command);
    /** @brief Updates contract identified by command id. */
    void updateContract(const core::ports::workspace::ContractCommand& command);
    /** @brief Deletes contract by id. */
    void deleteContract(const std::string& id);

    /** @brief Adds statement and returns created id. */
    [[nodiscard]] std::string addStatement(const core::ports::workspace::StatementCommand& command);
    /** @brief Updates statement identified by command id. */
    void updateStatement(const core::ports::workspace::StatementCommand& command);
    /** @brief Deletes statement by id. */
    void deleteStatement(const std::string& id);

    /** @brief Adds transaction and returns created id. */
    [[nodiscard]] std::string addTransaction(const core::ports::workspace::TransactionCommand& command);
    /** @brief Updates transaction identified by command id. */
    void updateTransaction(const core::ports::workspace::TransactionCommand& command);
    /** @brief Deletes transaction by id. */
    void deleteTransaction(const std::string& id);

    /** @brief Adds analysis and returns created id. */
    [[nodiscard]] std::string addAnalysis(const core::ports::workspace::AnalysisCommand& command);
    /** @brief Updates analysis identified by command id. */
    void updateAnalysis(const core::ports::workspace::AnalysisCommand& command);
    /** @brief Deletes analysis by id. */
    void deleteAnalysis(const std::string& id);

    /** @brief Adds annual aggregate and returns created id. */
    [[nodiscard]] std::string addAnnual(const core::ports::workspace::AnnualCommand& command);
    /** @brief Updates annual aggregate identified by command id. */
    void updateAnnual(const core::ports::workspace::AnnualCommand& command);
    /** @brief Deletes annual aggregate by id. */
    void deleteAnnual(const std::string& id);

    /** @brief Validates actor command data without mutating state. */
    [[nodiscard]] core::ports::workspace::ValidationResult validate(const core::ports::workspace::ActorCommand& command) const;
    /** @brief Validates property command data without mutating state. */
    [[nodiscard]] core::ports::workspace::ValidationResult validate(const core::ports::workspace::PropertyCommand& command) const;
    /** @brief Validates contract command data without mutating state. */
    [[nodiscard]] core::ports::workspace::ValidationResult validate(const core::ports::workspace::ContractCommand& command) const;
    /** @brief Validates statement command data without mutating state. */
    [[nodiscard]] core::ports::workspace::ValidationResult validate(const core::ports::workspace::StatementCommand& command) const;
    /** @brief Validates transaction command data without mutating state. */
    [[nodiscard]] core::ports::workspace::ValidationResult validate(const core::ports::workspace::TransactionCommand& command) const;
    /** @brief Validates analysis command data without mutating state. */
    [[nodiscard]] core::ports::workspace::ValidationResult validate(const core::ports::workspace::AnalysisCommand& command) const;
    /** @brief Validates annual command data without mutating state. */
    [[nodiscard]] core::ports::workspace::ValidationResult validate(const core::ports::workspace::AnnualCommand& command) const;

private:
    static std::string commitCreated(WorkspaceCommandService& service, std::string id);
    static void commitIfChanged(WorkspaceCommandService& service, bool changed);

    core::domain::catalog::WorkspaceCatalog& mutableCatalogState() noexcept;
    const core::domain::catalog::WorkspaceCatalog& catalogState() const noexcept;
    void commit();
    void notifyCatalogChanged();

    WorkspaceSession* session_ = nullptr;
};

} // namespace core::application
