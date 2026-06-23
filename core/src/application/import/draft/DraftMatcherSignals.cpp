/**
 * @file core/src/application/import/draft/DraftMatcherSignals.cpp
 * @brief Implements import-draft text signal extraction helpers.
 */

#include "core/pch.h"
#include "core/application/import/draft/DraftMatcher.h"
#include "core/domain/policies/DraftMatchingPolicy.h"

namespace core::application::importing::draft {

namespace policy = core::domain::policies::matching;

std::string normalizeDraftText(const std::string& text)
{
    return policy::normalizeText(text);
}

bool matchesDraftText(const std::string& left, const std::string& right)
{
    return policy::matchesDraftText(left, right);
}

std::vector<std::string> referenceAliasesFromMetadata(const std::string& metadata)
{
    return policy::referenceAliasesFromMetadata(metadata);
}

DraftTextSignals buildDraftTextSignals(const core::domain::catalog::WorkspaceCatalog& state,
                                       const core::application::importing::draft::TransactionDraft& transaction)
{
    DraftTextSignals signals;
    const auto metadataText = policy::metadataSignalText(transaction.metadata);
    signals.sharedText = metadataText.empty() ? transaction.metadata : metadataText;

    const auto lines = policy::splitLines(transaction.metadata);
    signals.typeText = policy::extractTypeText(state, signals.sharedText, lines);
    signals.actorText = policy::extractActorText(lines, signals.typeText);
    if (signals.actorText.empty()) signals.actorText = policy::firstMeaningfulLine(transaction.metadata);
    if (signals.actorText.empty()) signals.actorText = policy::leadingPhrase(signals.sharedText, 4);
    signals.propertyText = policy::trim(policy::joinNonEmptyLines({signals.sharedText, signals.actorText, signals.typeText}, " "));
    signals.contractText = policy::trim(policy::joinNonEmptyLines({signals.typeText, signals.actorText, signals.sharedText}, " "));
    return signals;
}

}
