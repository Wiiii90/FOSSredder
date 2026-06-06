/**
 * @file core/src/application/import/draft/DraftFinalizer.cpp
 * @brief Materializes imported draft statements into domain entities.
 */

#include "core/application/import/draft/DraftFinalizer.h"

#include "core/constants/app_state.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"
#include "core/domain/policies/TransactionPolicy.h"
#include "../../../utils/StableId.h"
#include "../../../utils/Util.h"

#include <charconv>

namespace {

int nextGeneratedContractIndex(const std::vector<std::shared_ptr<Contract>>& contracts)
{
    int maxIdx = 0;
    constexpr auto prefix = core::constants::appState::kGeneratedContractPrefix;
    for (const auto& contractPtr : contracts) {
        if (!contractPtr) continue;
        const std::string& contractName = contractPtr->name();
        if (contractName.size() <= prefix.size() || contractName.rfind(prefix.data(), 0) != 0) continue;
        const std::string rest = contractName.substr(prefix.size());
        int idx = 0;
        const auto result = std::from_chars(rest.data(), rest.data() + rest.size(), idx);
        if (result.ec == std::errc{} && result.ptr == rest.data() + rest.size())
            maxIdx = std::max(maxIdx, idx);
    }
    return maxIdx + 1;
}

}

namespace core::application::importing::draft {

std::string DraftFinalizer::finalize(core::domain::catalog::WorkspaceCatalog& state, const core::application::importing::draft::StatementDraft& draft)
{
    if (draft.transactions.empty()) return {};

    auto contracts = state.contracts();
    auto transactions = state.transactions();
    auto statements = state.statements();

    auto statement = std::make_shared<Statement>();
    statement->setId(core::utils::makeStableId());
    statement->rename(::core::utils::trim(draft.name).empty()
        ? std::string(core::constants::appState::kDefaultImportedStatementName)
        : draft.name);

    std::size_t addedTransactions = 0;

    for (const auto& item : draft.transactions) {
        auto transaction = std::make_shared<Transaction>();
        transaction->setId(core::utils::makeStableId());
        transaction->setName(item.name);
        transaction->setBookingDate(item.bookingDate);
        transaction->setValuta(item.valuta);
        transaction->setAmount(item.amount);
        transaction->setStatementId(statement->id());
        transaction->setStatus(item.status);
        transaction->setActorId(item.actorId);
        transaction->setContractId(item.contractId);
        transaction->setAllocatable(item.allocatable);
        transaction->setPropertyIds(item.propertyIds);
        if (!core::domain::policies::transaction::canFinalizeFromDraft(transaction->bookingDate(),
                                                                       transaction->amount(),
                                                                       transaction->statementId())) {
            continue;
        }
        const std::string normalizedContractId = core::domain::policies::transaction::trimCopy(item.contractId);
        if (!normalizedContractId.empty()) {
            transaction->setContractId(normalizedContractId);
        } else {
            const std::string normalizedType = core::domain::policies::transaction::trimCopy(item.type);
            if (!normalizedType.empty()) {
                constexpr auto prefix = core::constants::appState::kGeneratedContractPrefix;
                auto contract = std::make_shared<Contract>();
                contract->setId(core::utils::makeStableId());
                contract->rename(std::string(prefix) + std::to_string(nextGeneratedContractIndex(contracts)));
                contract->setType(normalizedType);
                contract->setPropertyIds(item.propertyIds);
                contracts.push_back(contract);
                transaction->setContractId(contract->id());
            }
        }

        transactions.push_back(transaction);
        statement->addTransaction(transaction->id());
        ++addedTransactions;
    }

    if (addedTransactions == 0) {
        return {};
    }

    statements.push_back(statement);
    state.setContracts(std::move(contracts));
    state.setTransactions(std::move(transactions));
    state.setStatements(std::move(statements));
    return statement->id();
}

}
