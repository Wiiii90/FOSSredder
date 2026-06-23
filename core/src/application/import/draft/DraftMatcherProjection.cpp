/**
 * @file core/src/application/import/draft/DraftMatcherProjection.cpp
 * @brief Implements import-draft projection helpers for derived UI-agnostic
 * state.
 */

#include "core/application/import/draft/DraftMatcher.h"
#include "core/domain/policies/DraftMatchingPolicy.h"
#include "core/pch.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace core::application::importing::draft {

namespace policy = core::domain::policies::matching;

namespace {

constexpr double kSyntheticConfidence = 0.35;
constexpr double kSuggestionYellowThreshold = 0.4;
constexpr double kSuggestionGreenThreshold = 0.9;

std::string normalizeAllocatableMode(std::string value) {
  auto normalized = policy::trim(std::move(value));
  std::transform(
      normalized.begin(), normalized.end(), normalized.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  if (normalized == "non allocatable")
    normalized = "non-allocatable";
  return normalized;
}

bool containsId(const std::vector<std::string> &ids, const std::string &id) {
  return std::find(ids.begin(), ids.end(), id) != ids.end();
}

bool appendUnique(std::vector<std::string> &values, const std::string &value) {
  if (value.empty() || containsId(values, value))
    return false;
  values.push_back(value);
  return true;
}

template <typename TEntity>
std::vector<core::domain::Alias> aliasUsages(const TEntity &entity) {
  std::vector<core::domain::Alias> out;
  out.reserve(entity.aliases().size());
  for (const auto &alias : entity.aliases()) {
    if (alias.value().empty())
      continue;
    out.push_back(alias);
  }
  return out;
}

const DraftSuggestionCandidate *
topSuggestion(const DraftSuggestionBucket &bucket) {
  return bucket.candidates.empty() ? nullptr : &bucket.candidates.front();
}

int confidencePercent(const DraftSuggestionCandidate *suggestion) {
  return suggestion ? static_cast<int>(std::round(
                          std::clamp(suggestion->confidence, 0.0, 1.0) * 100.0))
                    : 0;
}

std::string actorDisplay(const core::domain::Actor &actor) {
  return actor.name();
}

std::string propertyDisplay(const core::domain::Property &property) {
  return property.name();
}

DraftChoiceRow actorRow(const std::shared_ptr<core::domain::Actor> &actor) {
  DraftChoiceRow row;
  if (!actor)
    return row;
  row.id = actor->id();
  row.name = actor->name();
  row.display = actorDisplay(*actor);
  for (const auto &alias : actor->aliases()) {
    row.aliases.push_back(alias.value());
  }
  return row;
}

DraftChoiceRow
propertyRow(const std::shared_ptr<core::domain::Property> &property) {
  DraftChoiceRow row;
  if (!property)
    return row;
  row.id = property->id();
  row.name = property->name();
  row.display = propertyDisplay(*property);
  for (const auto &alias : property->aliases()) {
    row.aliases.push_back(alias.value());
  }
  return row;
}

DraftChoiceRow
contractRow(const std::shared_ptr<core::domain::Contract> &contract) {
  DraftChoiceRow row;
  if (!contract)
    return row;
  row.id = contract->id();
  row.name = contract->name();
  row.display = contract->name();
  row.type = contract->type();
  row.allocatableMode = contract->allocatableMode();
  for (const auto &alias : contract->aliases()) {
    row.aliases.push_back(alias.value());
  }
  row.actorIds = contract->actorIds();
  row.propertyIds = contract->propertyIds();
  return row;
}

bool hasAnyPropertyOverlap(const std::vector<std::string> &lhs,
                           const std::vector<std::string> &rhs) {
  for (const auto &id : lhs) {
    if (containsId(rhs, id))
      return true;
  }
  return false;
}

std::vector<DraftChoiceRow>
actorRows(const core::domain::catalog::WorkspaceCatalog &state) {
  std::vector<DraftChoiceRow> rows;
  rows.reserve(state.actors().size());
  for (const auto &actor : state.actors()) {
    auto row = actorRow(actor);
    if (!row.id.empty())
      rows.push_back(std::move(row));
  }
  return rows;
}

std::vector<DraftChoiceRow>
propertyRows(const core::domain::catalog::WorkspaceCatalog &state) {
  std::vector<DraftChoiceRow> rows;
  rows.reserve(state.properties().size());
  for (const auto &property : state.properties()) {
    auto row = propertyRow(property);
    if (!row.id.empty())
      rows.push_back(std::move(row));
  }
  return rows;
}

std::vector<DraftChoiceRow>
contractRows(const core::domain::catalog::WorkspaceCatalog &state) {
  std::vector<DraftChoiceRow> rows;
  rows.reserve(state.contracts().size());
  for (const auto &contract : state.contracts()) {
    auto row = contractRow(contract);
    if (!row.id.empty())
      rows.push_back(std::move(row));
  }
  return rows;
}

int rowIndexById(const std::vector<DraftChoiceRow> &rows,
                 const std::string &id) {
  if (id.empty())
    return -1;
  for (std::size_t i = 0; i < rows.size(); ++i) {
    if (rows[i].id == id)
      return static_cast<int>(i);
  }
  return -1;
}

const DraftChoiceRow *rowByIndex(const std::vector<DraftChoiceRow> &rows,
                                 int index) {
  return (index >= 0 && static_cast<std::size_t>(index) < rows.size())
             ? &rows[static_cast<std::size_t>(index)]
             : nullptr;
}

std::string firstExistingText(const DraftChoiceRow &row) {
  if (!row.display.empty())
    return row.display;
  if (!row.name.empty())
    return row.name;
  if (!row.type.empty())
    return row.type;
  return row.id;
}

bool rowMatchesText(const DraftChoiceRow &row, const std::string &text) {
  const auto key = policy::normalizeText(text);
  if (key.empty())
    return false;

  if (policy::normalizeText(row.name) == key)
    return true;
  if (policy::normalizeText(row.display) == key)
    return true;
  if (policy::normalizeText(row.type) == key)
    return true;
  for (const auto &alias : row.aliases) {
    if (policy::normalizeText(alias) == key)
      return true;
  }
  return false;
}

const DraftChoiceRow *findRowByText(const std::vector<DraftChoiceRow> &rows,
                                    const std::string &text) {
  for (const auto &row : rows) {
    if (rowMatchesText(row, text))
      return &row;
  }
  return nullptr;
}

std::string actorDisplayText(const DraftLinkSelection &selection,
                             const std::vector<DraftChoiceRow> &rows,
                             const DraftSuggestionCandidate *top) {
  if (!selection.actorId.empty()) {
    const auto index = rowIndexById(rows, selection.actorId);
    const auto *row = rowByIndex(rows, index);
    return row ? firstExistingText(*row) : selection.actorId;
  }
  if (top && !top->label.empty())
    return top->label;
  const auto actorText = policy::trim(selection.actorText);
  if (!actorText.empty())
    return actorText;
  return {};
}

std::string contractDisplayText(const DraftLinkSelection &selection,
                                const std::vector<DraftChoiceRow> &rows,
                                const DraftSuggestionCandidate *top) {
  if (!selection.contractId.empty()) {
    const auto index = rowIndexById(rows, selection.contractId);
    const auto *row = rowByIndex(rows, index);
    if (row) {
      if (!row->name.empty())
        return row->name;
      return firstExistingText(*row);
    }
    return selection.contractId;
  }
  (void)top;
  return {};
}

std::string actorSeedText(const DraftLinkSelection &selection,
                          const std::string &displayText,
                          const DraftSuggestionCandidate *top) {
  if (!selection.actorId.empty())
    return displayText;
  const auto actorText = policy::trim(selection.actorText);
  if (selection.actorSelected && actorText.empty())
    return {};
  if (!actorText.empty())
    return actorText;
  return top ? top->label : std::string{};
}

std::string contractSeedText(const DraftLinkSelection &selection,
                             const std::string &displayText,
                             const DraftSuggestionCandidate *top) {
  if (!selection.contractId.empty())
    return displayText;
  const auto type = policy::trim(selection.type);
  if (selection.contractSelected && type.empty())
    return {};
  if (!type.empty())
    return type;
  return {};
}

std::string propertySuggestionSummary(const DraftSuggestionBucket &bucket) {
  const auto *top = topSuggestion(bucket);
  if (!top)
    return "0% Confidence - No suggestion";
  const auto pct = confidencePercent(top);
  if (top->confidence < kSuggestionYellowThreshold) {
    return std::to_string(pct) + "% Confidence - No suggestion";
  }

  std::vector<std::string> labels;
  const auto maxCount = std::min<std::size_t>(2, bucket.candidates.size());
  labels.reserve(maxCount);
  for (std::size_t i = 0; i < maxCount; ++i) {
    if (!bucket.candidates[i].label.empty())
      labels.push_back(bucket.candidates[i].label);
  }

  if (labels.empty())
    return std::to_string(pct) + "% Confidence - No suggestion";
  return std::to_string(pct) + "% Confidence - (" +
         policy::joinNonEmptyLines(labels, ", ") + ")";
}

std::string actorSuggestionSummary(const DraftSuggestionBucket &bucket) {
  const auto *top = topSuggestion(bucket);
  if (!top)
    return "0% Confidence - No suggestion";
  const auto pct = confidencePercent(top);
  if (top->confidence < kSuggestionYellowThreshold) {
    return std::to_string(pct) + "% Confidence - No suggestion";
  }
  if (top->label.empty())
    return std::to_string(pct) + "% Confidence - No suggestion";
  return std::to_string(pct) + "% Confidence - " + top->label;
}

std::string
contractSuggestionSummary(const DraftDerivedState &derived,
                          const std::vector<DraftChoiceRow> &contractChoices,
                          const std::string &selectedContractId) {
  const auto confidence =
      std::clamp(derived.contractSuggestionConfidence, 0.0, 1.0);
  const auto confidencePercent =
      static_cast<int>(std::round(confidence * 100.0));
  if (confidence < kSuggestionYellowThreshold) {
    return std::to_string(confidencePercent) + "% Confidence - No suggestion";
  }

  const DraftChoiceRow *row = nullptr;
  if (!selectedContractId.empty()) {
    const auto idx = rowIndexById(contractChoices, selectedContractId);
    row = rowByIndex(contractChoices, idx);
  }
  if (!row && derived.hasContractTopSuggestion &&
      !derived.contractTopSuggestion.entityId.empty()) {
    const auto idx =
        rowIndexById(contractChoices, derived.contractTopSuggestion.entityId);
    row = rowByIndex(contractChoices, idx);
  }

  std::string suggestion;
  if (row) {
    const auto n = !row->name.empty() ? row->name : row->display;
    const auto t = row->type;
    if (!n.empty() && !t.empty())
      suggestion = "(" + n + ", " + t + ")";
    else if (!n.empty())
      suggestion = n;
    else
      suggestion = t;
  } else if (derived.hasContractTopSuggestion) {
    suggestion = derived.contractTopSuggestion.label;
  }

  if (suggestion.empty())
    return std::to_string(confidencePercent) + "% Confidence - No suggestion";
  return std::to_string(confidencePercent) + "% Confidence - " + suggestion;
}

std::vector<std::string>
propertyAutoSelectIds(const DraftLinkSelection &selection,
                      const std::vector<DraftChoiceRow> &properties,
                      const std::vector<DraftChoiceRow> &contracts,
                      const std::string &selectedContractId) {
  std::vector<std::string> ids;
  const std::string source = !policy::trim(selection.propertyText).empty()
                                 ? selection.propertyText
                                 : selection.metadata + " " +
                                       selection.actorText + " " +
                                       selection.type;
  const auto containsNormalized = [](const std::string &haystack,
                                     const std::string &needle) {
    const auto h = policy::normalizeText(haystack);
    const auto n = policy::normalizeText(needle);
    return !h.empty() && !n.empty() && h.find(n) != std::string::npos;
  };

  for (const auto &row : properties) {
    if (row.id.empty())
      continue;
    if (containsNormalized(source, row.name) ||
        containsNormalized(source, row.display)) {
      appendUnique(ids, row.id);
      continue;
    }
    for (const auto &alias : row.aliases) {
      if (containsNormalized(source, alias)) {
        appendUnique(ids, row.id);
        break;
      }
    }
  }

  for (const auto &suggestion : selection.propertySuggestions.candidates) {
    if (suggestion.confidence < 0.25)
      continue;
    for (const auto &row : properties) {
      if (row.id.empty())
        continue;
      if (row.id == suggestion.entityId ||
          containsNormalized(suggestion.label, row.name) ||
          containsNormalized(suggestion.label, row.display)) {
        appendUnique(ids, row.id);
        break;
      }
    }
  }

  const auto contractId =
      !selectedContractId.empty() ? selectedContractId : selection.contractId;
  if (!contractId.empty()) {
    const auto contractIndex = rowIndexById(contracts, contractId);
    if (const auto *contractRow = rowByIndex(contracts, contractIndex)) {
      for (const auto &propertyId : contractRow->propertyIds)
        appendUnique(ids, propertyId);
    }
  }

  return ids;
}

double contractScore(const DraftLinkSelection &selection,
                     const DraftChoiceRow &row,
                     const std::vector<DraftChoiceRow> &actors,
                     const DraftSuggestionCandidate *topContract,
                     const std::string &actorDisplay) {
  double score = 0.0;
  const auto typeText = policy::trim(selection.type);
  const auto actorText = policy::trim(selection.actorText).empty()
                             ? actorDisplay
                             : policy::trim(selection.actorText);

  if (!typeText.empty()) {
    if (policy::matchesDraftText(row.type, typeText))
      score += 600.0;
    if (policy::matchesDraftText(row.name, typeText))
      score += 420.0;
    if (policy::matchesDraftText(row.display, typeText))
      score += 360.0;
  }

  if (!actorText.empty()) {
    for (const auto &actorId : row.actorIds) {
      const auto actorIndex = rowIndexById(actors, actorId);
      const auto *actorRowPtr = rowByIndex(actors, actorIndex);
      const auto actorName =
          actorRowPtr ? firstExistingText(*actorRowPtr) : actorId;
      if (policy::matchesDraftText(actorName, actorText)) {
        score += 520.0;
        break;
      }
    }
    if (policy::matchesDraftText(row.name, actorText) ||
        policy::matchesDraftText(row.type, actorText))
      score += 180.0;
  }

  for (const auto &propertyId : selection.propertyIds) {
    if (containsId(row.propertyIds, propertyId)) {
      score += 160.0;
      break;
    }
  }

  if (topContract && row.id == topContract->entityId)
    score += 40.0;
  return score;
}

template <typename EntityRange, typename LabelFn, typename TextFn>
DraftSuggestionBucket buildSuggestionBucket(const EntityRange &entities,
                                            const std::string &entityType,
                                            LabelFn &&labelFn, TextFn &&textFn,
                                            const std::string &sourceText) {
  DraftSuggestionBucket bucket;
  bucket.sourceText = sourceText;

  const auto sourceTokens = policy::tokens(sourceText);
  const auto sourceLeadTokens =
      policy::tokens(policy::leadingText(sourceText, 4));

  struct RankedSuggestion {
    DraftSuggestionCandidate value;
  };
  std::vector<RankedSuggestion> ranked;
  ranked.reserve(entities.size());

  for (const auto &entity : entities) {
    if (!entity)
      continue;

    DraftSuggestionCandidate suggestion;
    suggestion.entityType = entityType;
    suggestion.entityId = entity->id();
    suggestion.label = labelFn(*entity);
    suggestion.sourceText = sourceText;

    const auto candidateText = policy::normalizeText(textFn(*entity));
    const auto candidateTokens = policy::tokens(textFn(*entity));
    const auto usages = aliasUsages(*entity);

    double score = 0.0;
    std::vector<std::string> matchedAliases;
    double aliasScore = 0.0;
    double recencyScore = 0.0;
    int bestHitCount = 0;
    std::string bestLastUsedAt;

    double strongestUsageFactor = 0.0;
    for (const auto &usage : usages) {
      strongestUsageFactor =
          std::max(strongestUsageFactor,
                   policy::aliasHitWeight(usage.hitCount()) * 0.65 +
                       policy::aliasRecencyWeight(usage.lastUsedAt()) * 0.35);
    }
    if (strongestUsageFactor <= 0.0)
      strongestUsageFactor = 0.15;

    if (!candidateText.empty() &&
        normalizeDraftText(sourceText).find(candidateText) !=
            std::string::npos) {
      score += 40.0 + (40.0 * strongestUsageFactor * 0.2);
    }
    score += policy::tokenOverlapScore(sourceTokens, candidateTokens) *
             (12.0 + strongestUsageFactor * 4.0);
    score +=
        policy::tokenOverlapScore(sourceLeadTokens, candidateTokens) * 18.0;

    for (const auto &usage : usages) {
      const std::string alias = usage.value();
      const auto aliasNorm = policy::normalizeText(alias);
      if (aliasNorm.empty())
        continue;

      const double hitWeight = policy::aliasHitWeight(usage.hitCount());
      const double recentWeight =
          policy::aliasRecencyWeight(usage.lastUsedAt());
      const double usageFactor = 1.0 + hitWeight + recentWeight * 0.75;

      if (normalizeDraftText(sourceText).find(aliasNorm) != std::string::npos) {
        score += 80.0 * usageFactor;
        matchedAliases.push_back(alias);
        aliasScore += hitWeight;
        recencyScore += recentWeight;
        bestHitCount = std::max(bestHitCount, usage.hitCount());
        if (!usage.lastUsedAt().empty() &&
            (bestLastUsedAt.empty() || usage.lastUsedAt() > bestLastUsedAt))
          bestLastUsedAt = usage.lastUsedAt();
        continue;
      }

      const auto aliasTokens = policy::tokens(alias);
      const int aliasOverlap =
          policy::tokenOverlapScore(sourceTokens, aliasTokens);
      if (aliasOverlap > 0) {
        score += (10.0 * aliasOverlap) *
                 (1.0 + hitWeight * 0.5 + recentWeight * 0.25);
        matchedAliases.push_back(alias);
        aliasScore += hitWeight * 0.5;
        recencyScore += recentWeight * 0.5;
        bestHitCount = std::max(bestHitCount, usage.hitCount());
        if (!usage.lastUsedAt().empty() &&
            (bestLastUsedAt.empty() || usage.lastUsedAt() > bestLastUsedAt))
          bestLastUsedAt = usage.lastUsedAt();
      }
    }

    if (score <= 0.0) {
      score = 0.05;
      suggestion.rationale = "Fallback";
    }

    suggestion.score = score;
    suggestion.confidence = std::clamp(score / 220.0, 0.0, 1.0);
    suggestion.aliasWeight = std::clamp(aliasScore, 0.0, 1.0);
    suggestion.recencyWeight = std::clamp(recencyScore, 0.0, 1.0);
    suggestion.hitCount = bestHitCount;
    suggestion.lastUsedAt = bestLastUsedAt;
    suggestion.matchedAliases = matchedAliases;
    if (!matchedAliases.empty()) {
      suggestion.rationale = "Alias match";
    } else if (!candidateText.empty() &&
               normalizeDraftText(sourceText).find(candidateText) !=
                   std::string::npos) {
      suggestion.rationale = "Name match";
    } else {
      suggestion.rationale = "Token overlap";
    }

    ranked.push_back({std::move(suggestion)});
  }

  std::sort(ranked.begin(), ranked.end(),
            [](const RankedSuggestion &lhs, const RankedSuggestion &rhs) {
              if (lhs.value.score == rhs.value.score) {
                if (lhs.value.confidence == rhs.value.confidence)
                  return lhs.value.label < rhs.value.label;
                return lhs.value.confidence > rhs.value.confidence;
              }
              return lhs.value.score > rhs.value.score;
            });

  constexpr std::size_t kMaxCandidates = 5;
  bucket.candidates.reserve(std::min(kMaxCandidates, ranked.size()));
  for (std::size_t i = 0; i < ranked.size() && i < kMaxCandidates; ++i) {
    bucket.candidates.push_back(std::move(ranked[i].value));
  }

  return bucket;
}

} // namespace

DraftDerivedState
buildDraftDerivedState(const core::domain::catalog::WorkspaceCatalog &state,
                       const DraftLinkSelection &selection) {
  auto effectiveSelection = selection;
  if (policy::trim(effectiveSelection.type).empty() &&
      !policy::trim(effectiveSelection.metadata).empty()) {
    const auto metadataLines = policy::splitLines(effectiveSelection.metadata);
    effectiveSelection.type = policy::extractTypeText(
        state, effectiveSelection.metadata, metadataLines);
  }

  DraftDerivedState derived;
  const auto actors = actorRows(state);
  const auto properties = propertyRows(state);
  const auto contracts = contractRows(state);

  derived.propertyRows = properties;
  derived.proofSource.clear();

  const auto *actorTop = topSuggestion(effectiveSelection.actorSuggestions);
  const auto *propertyTop =
      topSuggestion(effectiveSelection.propertySuggestions);
  const auto *contractTop =
      topSuggestion(effectiveSelection.contractSuggestions);
  if (actorTop) {
    derived.actorTopSuggestion = *actorTop;
    derived.hasActorTopSuggestion = true;
  }
  if (propertyTop) {
    derived.propertyTopSuggestion = *propertyTop;
    derived.hasPropertyTopSuggestion = true;
  }
  if (contractTop) {
    derived.contractTopSuggestion = *contractTop;
    derived.hasContractTopSuggestion = true;
  }

  derived.actorDisplayText =
      actorDisplayText(effectiveSelection, actors, actorTop);
  derived.contractDisplayText =
      contractDisplayText(effectiveSelection, contracts, contractTop);
  derived.actorSeedText =
      actorSeedText(effectiveSelection, derived.actorDisplayText, actorTop);
  derived.contractSeedText = contractSeedText(
      effectiveSelection, derived.contractDisplayText, contractTop);
  derived.propertySuggestionSummary =
      propertySuggestionSummary(effectiveSelection.propertySuggestions);

  derived.actorChoices = actors;
  DraftChoiceRow newActor;
  newActor.name = "New Actor";
  newActor.display = "New Actor";
  newActor.synthetic = true;
  newActor.confidence = actorTop ? actorTop->confidence : kSyntheticConfidence;
  newActor.sourceText = derived.actorSeedText;
  derived.actorChoices.insert(derived.actorChoices.begin(),
                              std::move(newActor));

  std::vector<std::pair<double, DraftChoiceRow>> scoredContracts;
  scoredContracts.reserve(contracts.size());
  for (const auto &row : contracts) {
    scoredContracts.push_back(
        {contractScore(effectiveSelection, row, actors, contractTop,
                       derived.actorDisplayText),
         row});
  }
  std::sort(scoredContracts.begin(), scoredContracts.end(),
            [](const auto &lhs, const auto &rhs) {
              if (lhs.first == rhs.first)
                return lhs.second.display < rhs.second.display;
              return lhs.first > rhs.first;
            });

  DraftChoiceRow newContract;
  newContract.name = "New Contract";
  newContract.display = "New Contract";
  newContract.type = derived.contractSeedText;
  newContract.synthetic = true;
  newContract.confidence =
      contractTop ? contractTop->confidence : kSyntheticConfidence;
  newContract.sourceText = derived.contractSeedText;
  derived.contractChoices.push_back(std::move(newContract));
  for (const auto &[score, row] : scoredContracts) {
    (void)score;
    derived.contractChoices.push_back(row);
  }

  if (!effectiveSelection.actorId.empty()) {
    derived.actorCurrentIndex =
        rowIndexById(derived.actorChoices, effectiveSelection.actorId);
  } else if (effectiveSelection.actorSelected) {
    derived.actorCurrentIndex = 0;
  } else if (actorTop && !actorTop->entityId.empty() &&
             actorTop->confidence >= 0.2) {
    derived.actorCurrentIndex =
        rowIndexById(derived.actorChoices, actorTop->entityId);
  } else if (!derived.actorSeedText.empty()) {
    if (const auto *match = findRowByText(actors, derived.actorSeedText)) {
      derived.actorCurrentIndex = rowIndexById(derived.actorChoices, match->id);
    }
  }
  if (derived.actorCurrentIndex < 0)
    derived.actorCurrentIndex = derived.actorChoices.empty() ? -1 : 0;

  if (!effectiveSelection.contractId.empty()) {
    derived.contractCurrentIndex =
        rowIndexById(derived.contractChoices, effectiveSelection.contractId);
  } else if (effectiveSelection.contractSelected) {
    derived.contractCurrentIndex = 0;
  } else if (!scoredContracts.empty() &&
             scoredContracts.front().first >= 900.0) {
    derived.contractCurrentIndex = rowIndexById(
        derived.contractChoices, scoredContracts.front().second.id);
  } else if (!derived.contractSeedText.empty() &&
             (!policy::trim(effectiveSelection.actorText).empty() ||
              !effectiveSelection.actorId.empty() ||
              !effectiveSelection.propertyIds.empty())) {
    if (const auto *match =
            findRowByText(contracts, derived.contractSeedText)) {
      derived.contractCurrentIndex =
          rowIndexById(derived.contractChoices, match->id);
    }
  }
  if (derived.contractCurrentIndex < 0)
    derived.contractCurrentIndex = derived.contractChoices.empty() ? -1 : 0;

  std::string selectedContractId = effectiveSelection.contractId;
  if (selectedContractId.empty() && derived.contractCurrentIndex > 0) {
    if (const auto *contractRow =
            rowByIndex(derived.contractChoices, derived.contractCurrentIndex);
        contractRow && !contractRow->synthetic) {
      selectedContractId = contractRow->id;
    }
  }

  if (!derived.hasContractTopSuggestion && !scoredContracts.empty()) {
    const auto &best = scoredContracts.front();
    if (!best.second.id.empty()) {
      derived.contractTopSuggestion.entityId = best.second.id;
      derived.contractTopSuggestion.entityType = "contract";
      derived.contractTopSuggestion.label =
          !best.second.display.empty()
              ? best.second.display
              : (!best.second.name.empty() ? best.second.name
                                           : best.second.type);
      derived.contractTopSuggestion.sourceText = derived.contractSeedText;
      derived.contractTopSuggestion.score = best.first;
      derived.contractTopSuggestion.confidence =
          std::clamp(best.first / 220.0, 0.0, 1.0);
      derived.contractTopSuggestion.rationale = "Derived contract score";
      derived.hasContractTopSuggestion = true;
    }
  }

  derived.actorSuggestionConfidence =
      derived.hasActorTopSuggestion
          ? std::clamp(derived.actorTopSuggestion.confidence, 0.0, 1.0)
          : 0.0;
  derived.propertySuggestionConfidence =
      derived.hasPropertyTopSuggestion
          ? std::clamp(derived.propertyTopSuggestion.confidence, 0.0, 1.0)
          : 0.0;
  derived.contractSuggestionConfidence =
      derived.hasContractTopSuggestion
          ? std::clamp(derived.contractTopSuggestion.confidence, 0.0, 1.0)
          : 0.0;

  if (derived.contractCurrentIndex <= 0 && derived.hasContractTopSuggestion &&
      !derived.contractTopSuggestion.entityId.empty() &&
      derived.contractSuggestionConfidence >= kSuggestionGreenThreshold) {
    const auto suggestedIndex = rowIndexById(
        derived.contractChoices, derived.contractTopSuggestion.entityId);
    if (suggestedIndex > 0)
      derived.contractCurrentIndex = suggestedIndex;
  }

  const std::string effectiveContractId = !selectedContractId.empty()
                                              ? selectedContractId
                                              : effectiveSelection.contractId;
  if (!effectiveContractId.empty()) {
    for (const auto &scored : scoredContracts) {
      if (scored.second.id == effectiveContractId) {
        const auto scoredConfidence =
            std::clamp(scored.first / 220.0, 0.0, 1.0);
        derived.contractSuggestionConfidence =
            std::max(derived.contractSuggestionConfidence, scoredConfidence);
        break;
      }
    }
  } else if (!scoredContracts.empty()) {
    const auto scoredConfidence =
        std::clamp(scoredContracts.front().first / 220.0, 0.0, 1.0);
    derived.contractSuggestionConfidence =
        std::max(derived.contractSuggestionConfidence, scoredConfidence);
  }

  if (derived.hasContractTopSuggestion && !effectiveContractId.empty()) {
    const auto suggestedIndex = rowIndexById(contracts, effectiveContractId);
    if (const auto *suggestedRow = rowByIndex(contracts, suggestedIndex)) {
      double boost = 0.0;
      const auto typeText = policy::trim(effectiveSelection.type);
      if (!typeText.empty() &&
          policy::matchesDraftText(suggestedRow->type, typeText))
        boost += 0.18;
      if (!effectiveSelection.actorId.empty() &&
          containsId(suggestedRow->actorIds, effectiveSelection.actorId))
        boost += 0.2;
      if (hasAnyPropertyOverlap(effectiveSelection.propertyIds,
                                suggestedRow->propertyIds))
        boost += 0.12;
      if (boost > 0.0) {
        derived.contractSuggestionConfidence =
            std::clamp(derived.contractSuggestionConfidence +
                           boost * (1.0 - derived.contractSuggestionConfidence),
                       0.0, 1.0);
      }
    }
  }
  const auto effectiveContractIndex =
      rowIndexById(contracts, effectiveContractId);
  const auto *effectiveContractRow =
      rowByIndex(contracts, effectiveContractIndex);
  const auto mode = normalizeAllocatableMode(
      effectiveContractRow ? effectiveContractRow->allocatableMode
                           : std::string{});
  if (effectiveSelection.allocatableSelected) {
    derived.effectiveAllocatable = effectiveSelection.allocatable;
  } else if (mode == "allocatable") {
    derived.effectiveAllocatable = true;
  } else if (mode == "non-allocatable") {
    derived.effectiveAllocatable = false;
  } else {
    derived.effectiveAllocatable =
        core::application::importing::draft::contractIsFullyAllocatable(
            state, effectiveContractId) ||
        effectiveSelection.allocatable;
  }

  if (mode == "allocatable" || mode == "non-allocatable") {
    derived.allocatableSuggestionConfidence = std::clamp(
        derived.contractSuggestionConfidence * 0.85 + 0.15, 0.0, 1.0);
  } else {
    derived.allocatableSuggestionConfidence =
        std::clamp(derived.contractSuggestionConfidence * 0.5, 0.0, 1.0);
  }
  derived.actorSuggestionSummary =
      actorSuggestionSummary(effectiveSelection.actorSuggestions);
  derived.propertySuggestionSummary =
      propertySuggestionSummary(effectiveSelection.propertySuggestions);
  derived.contractSuggestionSummary = contractSuggestionSummary(
      derived, derived.contractChoices, effectiveContractId);
  const auto allocConfidence =
      std::clamp(derived.allocatableSuggestionConfidence, 0.0, 1.0);
  const auto allocPercent =
      static_cast<int>(std::round(allocConfidence * 100.0));
  if (allocConfidence < kSuggestionYellowThreshold) {
    derived.allocatableSuggestionSummary =
        std::to_string(allocPercent) + "% Confidence - No suggestion";
  } else {
    derived.allocatableSuggestionSummary =
        std::to_string(allocPercent) + "% Confidence - " +
        (derived.effectiveAllocatable ? "Allocatable" : "Not allocatable");
  }

  derived.autoPropertyIds = propertyAutoSelectIds(
      effectiveSelection, properties, contracts, selectedContractId);

  return derived;
}

DraftImportSuggestions buildImportSuggestions(
    const core::domain::catalog::WorkspaceCatalog &state,
    const core::application::importing::draft::TransactionDraft &transaction) {
  const auto signals = buildDraftTextSignals(state, transaction);

  const auto actorSourceText = policy::trim(
      policy::joinNonEmptyLines({signals.actorText, signals.sharedText}, " "));
  const auto &propertySourceText = signals.propertyText;
  const auto contractSourceText = signals.sharedText;

  DraftImportSuggestions suggestions;
  suggestions.actor = buildSuggestionBucket(
      state.actors(), "actor",
      [](const core::domain::Actor &actor) { return actor.name(); },
      [](const core::domain::Actor &actor) { return actor.name(); },
      actorSourceText);

  suggestions.property = buildSuggestionBucket(
      state.properties(), "property",
      [](const core::domain::Property &property) { return property.name(); },
      [](const core::domain::Property &property) { return property.name(); },
      propertySourceText);

  suggestions.contract = buildSuggestionBucket(
      state.contracts(), "contract",
      [](const core::domain::Contract &contract) {
        if (contract.name().empty())
          return contract.type();
        if (contract.type().empty())
          return contract.name();
        return contract.type() + " — " + contract.name();
      },
      [](const core::domain::Contract &contract) {
        return contract.type().empty()
                   ? contract.name()
                   : contract.type() + " " + contract.name();
      },
      contractSourceText);

  return suggestions;
}

namespace {

template <typename TTarget, typename TValue>
bool assignIfChanged(TTarget& target, TValue&& value) {
  TTarget next = [&]() {
    if constexpr (std::is_enum_v<TTarget> &&
                  std::is_integral_v<std::remove_cvref_t<TValue>>) {
      return static_cast<TTarget>(value);
    } else {
      return TTarget{std::forward<TValue>(value)};
    }
  }();
  if (target == next) {
    return false;
  }
  target = std::move(next);
  return true;
}

struct DraftAutoSelectionPolicy {
  double actorConfidenceThreshold = -1.0;
  double contractConfidenceThreshold = -1.0;
  bool actorTextFromChoice = true;
  bool actorSeedRequiresEmptyText = true;
  bool contractTypeRequiresEmptyDraftType = true;
  bool contractPropertiesRequireEmptyDraftProperties = true;
  bool contractSelected = false;
  bool updateActorSelectedFromContract = false;
  bool actorSelectedFromContract = false;
};

DraftAutoSelectionPolicy policyFor(DraftAutoSelectionMode mode) {
  if (mode == DraftAutoSelectionMode::InteractiveSync) {
    return {.actorConfidenceThreshold = 0.9,
            .contractConfidenceThreshold = 0.9,
            .actorTextFromChoice = false,
            .actorSeedRequiresEmptyText = false,
            .contractTypeRequiresEmptyDraftType = false,
            .contractPropertiesRequireEmptyDraftProperties = false,
            .contractSelected = true,
            .updateActorSelectedFromContract = true,
            .actorSelectedFromContract = false};
  }
  return {};
}

bool passesConfidenceGate(double threshold, bool hasSuggestion,
                          double confidence) {
  return threshold < 0.0 || (hasSuggestion && confidence >= threshold);
}

std::string choiceDisplayText(const DraftChoiceRow& row) {
  if (!row.display.empty()) {
    return row.display;
  }
  if (!row.name.empty()) {
    return row.name;
  }
  if (!row.type.empty()) {
    return row.type;
  }
  return row.id;
}

const DraftChoiceRow* choiceAt(const std::vector<DraftChoiceRow>& rows,
                               int index) {
  if (index <= 0 || static_cast<std::size_t>(index) >= rows.size()) {
    return nullptr;
  }
  const auto& row = rows[static_cast<std::size_t>(index)];
  return !row.synthetic && !row.id.empty() ? &row : nullptr;
}

const DraftChoiceRow* findChoiceRowById(const std::vector<DraftChoiceRow>& rows,
                                        const std::string& id) {
  if (id.empty()) {
    return nullptr;
  }
  for (const auto& row : rows) {
    if (row.id == id) {
      return &row;
    }
  }
  return nullptr;
}

bool applyActorSelection(TransactionDraft& draft,
                         const DraftDerivedState& derived,
                         const DraftAutoSelectionPolicy& policy) {
  if (!draft.actorId.empty()) {
    return false;
  }

  bool changed = false;
  const double actorConfidence =
      derived.hasActorTopSuggestion ? derived.actorTopSuggestion.confidence
                                    : 0.0;
  const auto* actorRow =
      choiceAt(derived.actorChoices, derived.actorCurrentIndex);
  if (actorRow &&
      passesConfidenceGate(policy.actorConfidenceThreshold,
                           derived.hasActorTopSuggestion, actorConfidence)) {
    changed |= assignIfChanged(draft.actorId, actorRow->id);
    changed |= assignIfChanged(
        draft.actorText,
        policy.actorTextFromChoice ? choiceDisplayText(*actorRow)
                                   : std::string{});
    changed |= assignIfChanged(draft.actorSelected, false);
    return changed;
  }

  if (!derived.actorSeedText.empty() &&
      (!policy.actorSeedRequiresEmptyText || draft.actorText.empty())) {
    changed |= assignIfChanged(draft.actorText, derived.actorSeedText);
  }
  return changed;
}

bool applyContractSelection(TransactionDraft& draft,
                            const DraftDerivedState& derived,
                            const DraftAutoSelectionPolicy& policy) {
  if (!draft.contractId.empty()) {
    return false;
  }

  bool changed = false;
  const auto* contractRow =
      choiceAt(derived.contractChoices, derived.contractCurrentIndex);
  if (contractRow &&
      passesConfidenceGate(policy.contractConfidenceThreshold,
                           derived.hasContractTopSuggestion,
                           derived.contractSuggestionConfidence)) {
    changed |= assignIfChanged(draft.contractId, contractRow->id);
    if (!contractRow->type.empty() &&
        (!policy.contractTypeRequiresEmptyDraftType || draft.type.empty())) {
      changed |= assignIfChanged(draft.type, contractRow->type);
    }
    if (!contractRow->actorIds.empty()) {
      changed |= assignIfChanged(draft.actorId, contractRow->actorIds.front());
      if (const auto* actorRow =
              findChoiceRowById(derived.actorChoices,
                                contractRow->actorIds.front())) {
        changed |= assignIfChanged(draft.actorText,
                                   choiceDisplayText(*actorRow));
      }
      if (policy.updateActorSelectedFromContract) {
        changed |= assignIfChanged(draft.actorSelected,
                                   policy.actorSelectedFromContract);
      }
    }
    if (!contractRow->propertyIds.empty() &&
        (!policy.contractPropertiesRequireEmptyDraftProperties ||
         draft.propertyIds.empty())) {
      changed |= assignIfChanged(draft.propertyIds, contractRow->propertyIds);
    }
    changed |=
        assignIfChanged(draft.contractSelected, policy.contractSelected);
    return changed;
  }

  if (draft.type.empty() && !derived.contractSeedText.empty()) {
    changed |= assignIfChanged(draft.type, derived.contractSeedText);
  }
  return changed;
}

} // namespace

bool applyDerivedSelections(TransactionDraft& draft,
                            const DraftDerivedState& derived,
                            DraftAutoSelectionMode mode) {
  bool changed = false;
  const DraftAutoSelectionPolicy policy = policyFor(mode);
  changed |= applyActorSelection(draft, derived, policy);
  changed |= applyContractSelection(draft, derived, policy);

  if (mode == DraftAutoSelectionMode::InitialImport) {
    if (draft.contractId.empty() && !draft.contractSelected) {
      changed |= assignIfChanged(draft.type, std::string{});
    }
  }

  if (draft.propertyIds.empty() && !derived.autoPropertyIds.empty()) {
    changed |= assignIfChanged(draft.propertyIds, derived.autoPropertyIds);
  }

  if (!draft.allocatableSelected) {
    changed |= assignIfChanged(draft.allocatable,
                               derived.effectiveAllocatable);
  }

  return changed;
}

bool applyActorSelection(TransactionDraft& draft, const std::string& actorId) {
  if (actorId.empty()) {
    return false;
  }

  bool changed = false;
  changed |= assignIfChanged(draft.actorId, actorId);
  changed |= assignIfChanged(draft.actorText, std::string{});
  changed |= assignIfChanged(draft.actorSelected, true);
  changed |= assignIfChanged(draft.contractId, std::string{});
  changed |= assignIfChanged(draft.contractSelected, false);
  return changed;
}

bool clearActorSelection(TransactionDraft& draft) {
  bool changed = false;
  changed |= assignIfChanged(draft.actorId, std::string{});
  changed |= assignIfChanged(draft.actorText, std::string{});
  changed |= assignIfChanged(draft.actorSelected, false);
  changed |= assignIfChanged(draft.contractId, std::string{});
  changed |= assignIfChanged(draft.contractSelected, false);
  return changed;
}

bool applyPropertySelection(TransactionDraft& draft,
                            const std::string& propertyId) {
  if (propertyId.empty()) {
    return false;
  }

  bool changed = false;
  if (std::find(draft.propertyIds.begin(), draft.propertyIds.end(),
                propertyId) == draft.propertyIds.end()) {
    draft.propertyIds.push_back(propertyId);
    changed = true;
  }
  changed |= assignIfChanged(draft.contractId, std::string{});
  changed |= assignIfChanged(draft.contractSelected, false);
  return changed;
}

bool setPropertySelected(TransactionDraft& draft,
                         const std::string& propertyId,
                         bool selected) {
  if (propertyId.empty()) {
    return false;
  }

  auto propertyIds = draft.propertyIds;
  if (selected) {
    if (std::find(propertyIds.begin(), propertyIds.end(), propertyId) ==
        propertyIds.end()) {
      propertyIds.push_back(propertyId);
    }
  } else {
    propertyIds.erase(std::remove(propertyIds.begin(), propertyIds.end(),
                                  propertyId),
                      propertyIds.end());
  }

  bool changed = false;
  changed |= assignIfChanged(draft.propertyIds, propertyIds);
  if (changed) {
    changed |= assignIfChanged(draft.contractId, std::string{});
    changed |= assignIfChanged(draft.contractSelected, false);
  }
  return changed;
}

bool applyContractSelection(TransactionDraft& draft,
                            const DraftChoiceRow& contract) {
  if (contract.id.empty()) {
    return false;
  }

  bool changed = false;
  changed |= assignIfChanged(draft.contractId, contract.id);
  changed |= assignIfChanged(draft.contractSelected, true);
  if (!contract.type.empty()) {
    changed |= assignIfChanged(draft.type, contract.type);
  }
  if (!contract.actorIds.empty()) {
    changed |= assignIfChanged(draft.actorId, contract.actorIds.front());
    changed |= assignIfChanged(draft.actorText, std::string{});
    changed |= assignIfChanged(draft.actorSelected, false);
  }
  if (!contract.propertyIds.empty()) {
    changed |= assignIfChanged(draft.propertyIds, contract.propertyIds);
  }

  const auto mode = normalizeDraftText(contract.allocatableMode);
  if (mode == "allocatable") {
    changed |= assignIfChanged(draft.allocatable, true);
    changed |= assignIfChanged(draft.allocatableSelected, false);
  } else if (mode == "non-allocatable") {
    changed |= assignIfChanged(draft.allocatable, false);
    changed |= assignIfChanged(draft.allocatableSelected, false);
  }
  return changed;
}

bool applyContractSelection(TransactionDraft& draft,
                            const core::domain::catalog::WorkspaceCatalog& state,
                            const std::string& contractId) {
  if (contractId.empty()) {
    return false;
  }
  for (const auto& contract : contractRows(state)) {
    if (contract.id == contractId) {
      return applyContractSelection(draft, contract);
    }
  }
  return false;
}

bool clearContractSelection(TransactionDraft& draft) {
  bool changed = false;
  changed |= assignIfChanged(draft.contractId, std::string{});
  changed |= assignIfChanged(draft.contractSelected, false);
  return changed;
}

bool applyTransactionPatch(TransactionDraft& draft,
                           const TransactionDraftPatch& patch) {
  bool changed = false;
  if (patch.hasName) {
    changed |= assignIfChanged(draft.name, patch.name);
  }
  if (patch.hasBookingDate) {
    changed |= assignIfChanged(draft.bookingDate, patch.bookingDate);
  }
  if (patch.hasValuta) {
    changed |= assignIfChanged(draft.valuta, patch.valuta);
  }
  if (patch.hasAmount) {
    changed |= assignIfChanged(draft.amount, patch.amount);
  }
  if (patch.hasStatus) {
    changed |= assignIfChanged(
        draft.status,
        static_cast<core::domain::Transaction::Status>(patch.status));
  }
  if (patch.hasAllocatable) {
    changed |= assignIfChanged(draft.allocatable, patch.allocatable);
    changed |= assignIfChanged(draft.allocatableSelected,
                               patch.allocatableSelected);
  }
  return changed;
}

int insertTransactionAfter(StatementDraft& draft, int currentIndex) {
  const int newIndex = std::clamp(currentIndex + 1, 0,
                                  static_cast<int>(draft.transactions.size()));
  TransactionDraft transaction;
  transaction.id = "tx-new-" + std::to_string(draft.transactions.size() + 1);
  transaction.statementDraftId = draft.id;
  transaction.position = newIndex;
  transaction.name = "Transaction " + std::to_string(draft.transactions.size() + 1);

  draft.transactions.insert(draft.transactions.begin() + newIndex,
                            std::move(transaction));
  draft.transactionIds.clear();
  draft.transactionIds.reserve(draft.transactions.size());
  for (std::size_t i = 0; i < draft.transactions.size(); ++i) {
    draft.transactions[i].position = static_cast<int>(i);
    draft.transactions[i].statementDraftId = draft.id;
    draft.transactionIds.push_back(draft.transactions[i].id);
  }
  return newIndex;
}

int removeTransactionAt(StatementDraft& draft, int index) {
  if (draft.transactions.size() <= 1 || index < 0 ||
      static_cast<std::size_t>(index) >= draft.transactions.size()) {
    return -1;
  }

  draft.transactions.erase(draft.transactions.begin() + index);
  draft.transactionIds.clear();
  draft.transactionIds.reserve(draft.transactions.size());
  for (std::size_t i = 0; i < draft.transactions.size(); ++i) {
    draft.transactions[i].position = static_cast<int>(i);
    draft.transactions[i].statementDraftId = draft.id;
    draft.transactionIds.push_back(draft.transactions[i].id);
  }
  return std::min(index, static_cast<int>(draft.transactions.size()) - 1);
}

bool renameStatementDraft(StatementDraft& draft, const std::string& name) {
  return assignIfChanged(draft.name, name);
}

} // namespace core::application::importing::draft
