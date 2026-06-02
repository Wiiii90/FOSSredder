/**
 * @file core/src/application/import/internal/ParserHeuristics.cpp
 * @brief Implements lightweight text heuristics for statement parsing.
 */

#include "core/application/import/internal/ParserHeuristics.h"
#include "../../../utils/Util.h"

#include <array>
#include <vector>

using core::utils::lowerAscii;
using core::utils::trim;

namespace core::application::importing::internal {

namespace {

std::string normalizeAlphaNumeric(const std::string &value) {
  std::string normalized;
  normalized.reserve(value.size());
  for (unsigned char c : core::utils::lowerAscii(value)) {
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
      normalized.push_back(static_cast<char>(c));
  }
  return normalized;
}

bool looksLikeAmountToken(const std::string &token) {
  const auto separator = token.find_last_of(".,");
  if (separator == std::string::npos || separator == 0 ||
      separator + 3 != token.size())
    return false;

  const auto isDigit = [](unsigned char c) { return c >= '0' && c <= '9'; };
  if (!isDigit(static_cast<unsigned char>(token[separator + 1])) ||
      !isDigit(static_cast<unsigned char>(token[separator + 2])))
    return false;

  bool hasIntegralDigit = false;
  for (size_t i = 0; i < separator; ++i) {
    const unsigned char c = static_cast<unsigned char>(token[i]);
    if (isDigit(c)) {
      hasIntegralDigit = true;
      continue;
    }
    if (c != '.' && c != ',')
      return false;
  }

  return hasIntegralDigit;
}

bool containsAmountToken(const std::string &value) {
  const auto tokens = core::utils::splitWhitespace(value);
  return std::any_of(
      tokens.begin(), tokens.end(),
      [](const std::string &token) { return looksLikeAmountToken(token); });
}

} // namespace

bool isHeaderNoiseLine(const std::string &line) {
  const auto l = lowerAscii(line);
  if (l.size() <= 2)
    return true;

  const auto toks = core::utils::splitWhitespace(l);
  if (toks.empty())
    return false;

  std::vector<std::string> normalizedTokens;
  normalizedTokens.reserve(toks.size());
  for (const auto &token : toks)
    normalizedTokens.push_back(normalizeAlphaNumeric(token));

  static const std::array<std::string, 7> noise = {
      "kontoauszug",     "auszugnr",      "seitenr",      "ustidnr",
      "ansprechpartner", "telefonnummer", "kontowahrung",
  };

  int matches = 0;
  for (const auto &n : noise) {
    for (const auto &t : normalizedTokens) {
      if (t == n) {
        ++matches;
        break;
      }
    }
    if (matches >= 2)
      return true;
  }

  return false;
}

static bool containsAny(const std::string &hayLower,
                        std::initializer_list<const char *> needlesLower) {
  for (auto n : needlesLower) {
    if (hayLower.find(n) != std::string::npos)
      return true;
  }
  return false;
}

bool isFooterLine(const std::string &line) {
  const auto l = lowerAscii(line);
  return containsAny(
      l, {"folgeseite", "vsa000", "fil200", "ktea", "folgende seite"});
}

bool isPostTransactionFootnote(const std::string &line) {
  const auto l = lowerAscii(line);
  if (containsAmountToken(l))
    return false;

  const std::initializer_list<const char *> strong = {
      "guthaben sind als",
      "einlagensicherungsgesetzes",
      "informationsbogen",
      "der angegebene kontostand",
      "wertstellung der buchungen",
      "kontoüberziehung",
      "neuer kontostand",
      "alter kontostand",
      "folgeseite",
  };
  for (auto p : strong)
    if (l.find(p) != std::string::npos)
      return true;

  int weakMatches = 0;
  if (l.find("kontostand") != std::string::npos)
    ++weakMatches;
  if (l.find("guthaben") != std::string::npos)
    ++weakMatches;
  if (l.find("einlagen") != std::string::npos)
    ++weakMatches;
  if (l.find("betrag") != std::string::npos)
    ++weakMatches;
  return weakMatches >= 2;
}

bool isTransactionsSectionHeader(const std::string &line) {
  const auto l = lowerAscii(line);
  if (l.find("angaben zu den umsätzen") != std::string::npos)
    return true;
  const bool hasValuta = (l.find("valuta") != std::string::npos);
  const bool hasUmsaetze = (l.find("umsätzen") != std::string::npos) ||
                           (l.find("umsaetzen") != std::string::npos);
  const bool hasAngaben = (l.find("angaben") != std::string::npos);
  return hasValuta && (hasUmsaetze || hasAngaben);
}

bool isDebitCreditHeaderLine(const std::string &line) {
  const auto n = normalizeAlphaNumeric(line);
  return (n.find("zuihrenlasten") != std::string::npos) &&
         (n.find("zuihrengunsten") != std::string::npos);
}

} // namespace core::application::importing::internal
