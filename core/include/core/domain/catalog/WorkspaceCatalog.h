/**
 * @file core/include/core/domain/catalog/WorkspaceCatalog.h
 * @brief Domain aggregate that stores catalog entities for one workspace.
 */

#pragma once

#include <memory>
#include <vector>

#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"

namespace core::domain::catalog {

class WorkspaceCatalog {
public:
    template <typename T>
    using EntityList = std::vector<std::shared_ptr<T>>;

    using PropertyList = EntityList<core::domain::Property>;
    using ActorList = EntityList<core::domain::Actor>;
    using ContractList = EntityList<core::domain::Contract>;
    using StatementList = EntityList<core::domain::Statement>;
    using TransactionList = EntityList<core::domain::Transaction>;
    using AnalysisList = EntityList<core::domain::Analysis>;
    using AnnualList = EntityList<core::domain::Annual>;

    WorkspaceCatalog();

    /** @brief Returns all properties in the catalog. */
    [[nodiscard]] const PropertyList& properties() const noexcept;
    /** @brief Replaces all properties in the catalog. */
    void setProperties(PropertyList value);

    /** @brief Returns all actors in the catalog. */
    [[nodiscard]] const ActorList& actors() const noexcept;
    /** @brief Replaces all actors in the catalog. */
    void setActors(ActorList value);

    /** @brief Returns all contracts in the catalog. */
    [[nodiscard]] const ContractList& contracts() const noexcept;
    /** @brief Replaces all contracts in the catalog. */
    void setContracts(ContractList value);

    /** @brief Returns all statements in the catalog. */
    [[nodiscard]] const StatementList& statements() const noexcept;
    /** @brief Replaces all statements in the catalog. */
    void setStatements(StatementList value);

    /** @brief Returns all transactions in the catalog. */
    [[nodiscard]] const TransactionList& transactions() const noexcept;
    /** @brief Replaces all transactions in the catalog. */
    void setTransactions(TransactionList value);

    /** @brief Returns all analyses in the catalog. */
    [[nodiscard]] const AnalysisList& analyses() const noexcept;
    /** @brief Replaces all analyses in the catalog. */
    void setAnalyses(AnalysisList value);

    /** @brief Returns all annuals in the catalog. */
    [[nodiscard]] const AnnualList& annuals() const noexcept;
    /** @brief Replaces all annuals in the catalog. */
    void setAnnuals(AnnualList value);

    /** @brief Checks whether all catalog collections are empty. */
    [[nodiscard]] bool empty() const noexcept;

private:
    PropertyList properties_;
    ActorList actors_;
    ContractList contracts_;
    StatementList statements_;
    TransactionList transactions_;
    AnalysisList analyses_;
    AnnualList annuals_;
};

} // namespace core::domain::catalog
