/**
 * @file core/src/application/export/PropertyContractMatrix.h
 * @brief Declares private helpers for building export matrix data from
 * workspace state.
 */

#pragma once

#include "../../utils/Util.h"
#include "core/constants/export.h"
#include "core/domain/catalog/WorkspaceCatalog.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace core::application::exporting::internal {

/**
 * @brief Holds the matrix representation used by the CSV and XLSX exporters.
 */
struct PropertyContractMatrix {
  std::vector<std::string> propertyNames;
  std::vector<std::string> contractTypes;
  std::unordered_map<std::string, std::unordered_map<std::string, double>>
      amountsByProperty;
};

/**
 * @brief Resolves the final contract type label for one transaction.
 * @param contractId Contract identifier from the transaction.
 * @param idToType Precomputed contract-type lookup table.
 * @param state Current workspace snapshot.
 * @return Contract type label, or the unassigned label when none can be
 * resolved.
 */
inline std::string resolveContractType(
    const std::string &contractId,
    const std::unordered_map<std::string, std::string> &idToType,
    const core::domain::catalog::WorkspaceCatalog &state) {
  const std::string trimmedContractId = core::utils::trim(contractId);
  if (trimmedContractId.empty()) {
    return std::string(core::constants::exportFlow::labels::kUnassigned);
  }

  const auto indexedType = idToType.find(trimmedContractId);
  if (indexedType != idToType.end()) {
    const std::string trimmedType = core::utils::trim(indexedType->second);
    if (!trimmedType.empty()) {
      return trimmedType;
    }
  }

  for (const auto &contract : state.contracts()) {
    if (!contract) {
      continue;
    }
    if (core::utils::trim(contract->id()) != trimmedContractId) {
      continue;
    }

    const std::string trimmedType = core::utils::trim(contract->type());
    if (!trimmedType.empty()) {
      return trimmedType;
    }
    break;
  }

  return std::string(core::constants::exportFlow::labels::kUnassigned);
}

/**
 * @brief Builds the property/contract matrix from the current workspace
 * snapshot.
 * @param state Current workspace snapshot.
 * @return Matrix containing properties, contract types, and aggregated amounts.
 */
inline PropertyContractMatrix buildPropertyContractMatrix(
    const core::domain::catalog::WorkspaceCatalog &state) {
  std::unordered_map<std::string, std::string> propertyNamesById;
  propertyNamesById.reserve(state.properties().size());
  for (const auto &property : state.properties()) {
    if (property) {
      propertyNamesById[property->id()] = property->name();
    }
  }

  std::unordered_map<std::string, std::string> contractTypesById;
  contractTypesById.reserve(state.contracts().size());
  for (const auto &contract : state.contracts()) {
    if (contract) {
      contractTypesById[contract->id()] = contract->type();
    }
  }

  PropertyContractMatrix matrix;
  std::unordered_set<std::string> seenContractTypes;
  std::unordered_set<std::string> seenProperties;

  for (const auto &transaction : state.transactions()) {
    if (!transaction || transaction->propertyIds().empty()) {
      continue;
    }

    const std::string contractType = resolveContractType(
        transaction->contractId(), contractTypesById, state);
    if (seenContractTypes.insert(contractType).second) {
      matrix.contractTypes.push_back(contractType);
    }

    for (const auto &propertyId : transaction->propertyIds()) {
      const auto propertyNameIt = propertyNamesById.find(propertyId);
      const std::string propertyName = propertyNameIt != propertyNamesById.end()
                                           ? propertyNameIt->second
                                           : propertyId;
      matrix.amountsByProperty[propertyName][contractType] +=
          transaction->amount();
      seenProperties.insert(propertyName);
    }
  }

  matrix.propertyNames.reserve(seenProperties.size());
  for (const auto &property : state.properties()) {
    if (!property) {
      continue;
    }
    if (!matrix.amountsByProperty.count(property->name())) {
      continue;
    }
    if (seenProperties.erase(property->name()) > 0) {
      matrix.propertyNames.push_back(property->name());
    }
  }

  for (const auto &[propertyName, _] : matrix.amountsByProperty) {
    if (seenProperties.erase(propertyName) > 0) {
      matrix.propertyNames.push_back(propertyName);
    }
  }

  return matrix;
}

} // namespace core::application::exporting::internal
