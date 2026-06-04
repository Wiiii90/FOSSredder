/**
 * @file core/src/application/analysis/AnalysisRequest.cpp
 * @brief Implements analysis filter-spec parsing for analysis port requests.
 */

#include "core/ports/usecases/analysis/AnalysisRequest.h"

#include "core/constants/filters.h"
#include "core/domain/values/FilterSpec.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <ctime>
#include <sstream>
#include <unordered_set>

namespace core::ports::analysis {

namespace {

std::string trim(const std::string &value) {
  std::size_t first = 0;
  while (first < value.size() &&
         std::isspace(static_cast<unsigned char>(value[first])) != 0) {
    ++first;
  }

  std::size_t last = value.size();
  while (last > first &&
         std::isspace(static_cast<unsigned char>(value[last - 1])) != 0) {
    --last;
  }

  return value.substr(first, last - first);
}

std::string toLower(std::string value) {
  std::transform(
      value.begin(), value.end(), value.begin(),
      [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
  return value;
}

int defaultAnalysisYear() {
  const std::time_t now = std::time(nullptr);
  const std::tm *local = std::localtime(&now);
  if (!local) {
    return 2000;
  }
  return local->tm_year + 1900 - 1;
}

std::vector<std::string> splitList(const std::string &raw) {
  std::vector<std::string> out;
  std::unordered_set<std::string> seen;
  std::string token;
  std::istringstream ss(raw);
  while (std::getline(ss, token, core::constants::filters::separators::kList)) {
    const std::string trimmed = trim(token);
    if (trimmed.empty() || seen.contains(trimmed)) {
      continue;
    }
    seen.insert(trimmed);
    out.push_back(trimmed);
  }
  return out;
}

} // namespace

AnalysisFilterSelection
parseAnalysisFilterSelection(const std::string &filterSpec) {
  AnalysisFilterSelection out;
  out.year = std::to_string(defaultAnalysisYear());
  const std::string normalized =
      core::domain::FilterSpec::normalize(filterSpec);
  if (normalized.empty()) {
    return out;
  }

  std::istringstream ss(normalized);
  std::string clause;
  while (
      std::getline(ss, clause, core::constants::filters::separators::kClause)) {
    clause = trim(clause);
    if (clause.empty()) {
      continue;
    }

    if (clause.rfind(
            std::string(core::constants::filters::kDate) +
                std::string(core::constants::filters::operators::kGreaterEqual),
            0) == 0) {
      out.dateFrom = clause.substr(6);
      continue;
    }
    if (clause.rfind(
            std::string(core::constants::filters::kDate) +
                std::string(core::constants::filters::operators::kLessEqual),
            0) == 0) {
      out.dateTo = clause.substr(6);
      continue;
    }
    if (clause.rfind(
            std::string(core::constants::filters::kDateField) +
                std::string(core::constants::filters::operators::kEqual),
            0) == 0) {
      const std::string value = toLower(trim(clause.substr(10)));
      out.dateField = value == "valuta" ? "valuta" : "bookingDate";
      continue;
    }
    if (clause.rfind(
            std::string(core::constants::filters::kPropertyId) +
                std::string(core::constants::filters::operators::kEqual),
            0) == 0) {
      const std::vector<std::string> values = splitList(clause.substr(11));
      out.propertyIdsUnassigned =
          std::find(values.begin(), values.end(),
                    std::string(core::constants::filters::kUnassigned)) !=
          values.end();
      out.propertyIds.clear();
      for (const auto &value : values) {
        if (value != core::constants::filters::kUnassigned) {
          out.propertyIds.push_back(value);
        }
      }
      continue;
    }
    if (clause.rfind(
            std::string(core::constants::filters::kContractType) +
                std::string(core::constants::filters::operators::kEqual),
            0) == 0) {
      const std::vector<std::string> values = splitList(clause.substr(14));
      out.contractTypesUnassigned =
          std::find(values.begin(), values.end(),
                    std::string(core::constants::filters::kUnassigned)) !=
          values.end();
      out.contractTypes.clear();
      for (auto value : values) {
        value = toLower(value);
        if (value != core::constants::filters::kUnassigned) {
          out.contractTypes.push_back(std::move(value));
        }
      }
      continue;
    }
    if (clause.rfind(
            std::string(core::constants::filters::kAllocatable) +
                std::string(core::constants::filters::operators::kEqual),
            0) == 0) {
      const std::string mode = toLower(trim(clause.substr(12)));
      if (mode == "allocatable" || mode == "non-allocatable") {
        out.allocatableMode = mode;
      } else {
        out.allocatableMode = "all";
      }
      continue;
    }
  }

  if (out.dateFrom.size() == 10 && out.dateTo.size() == 10 &&
      out.dateFrom.ends_with("-01-01") && out.dateTo.ends_with("-12-31") &&
      out.dateFrom.substr(0, 4) == out.dateTo.substr(0, 4)) {
    out.dateMode = "year";
    out.year = out.dateFrom.substr(0, 4);
  } else if (!out.dateFrom.empty() || !out.dateTo.empty()) {
    out.dateMode = "range";
  }

  return out;
}

std::string buildAnalysisFilterSpec(const AnalysisFilterSelection &selection) {
  std::vector<std::string> clauses;
  const std::string normalizedDateField = toLower(trim(selection.dateField));
  if (normalizedDateField == "valuta") {
    clauses.emplace_back("dateField=valuta");
  }

  const std::string normalizedDateMode = toLower(trim(selection.dateMode));
  const std::string year = trim(selection.year);
  if (normalizedDateMode == "range") {
    const std::string from = trim(selection.dateFrom);
    const std::string to = trim(selection.dateTo);
    bool hasDateClause = false;
    if (!from.empty()) {
      clauses.push_back("date>=" + from);
      hasDateClause = true;
    }
    if (!to.empty()) {
      clauses.push_back("date<=" + to);
      hasDateClause = true;
    }
    if (!hasDateClause) {
      const std::string fallbackYear = std::to_string(defaultAnalysisYear());
      clauses.push_back("date>=" + fallbackYear + "-01-01");
      clauses.push_back("date<=" + fallbackYear + "-12-31");
    }
  } else {
    const std::string resolvedYear =
        year.empty() ? std::to_string(defaultAnalysisYear()) : year;
    clauses.push_back("date>=" + resolvedYear + "-01-01");
    clauses.push_back("date<=" + resolvedYear + "-12-31");
  }

  std::unordered_set<std::string> propertySeen;
  std::vector<std::string> propertyValues;
  for (const auto &raw : selection.propertyIds) {
    const std::string value = trim(raw);
    if (value.empty() || propertySeen.contains(value)) {
      continue;
    }
    propertySeen.insert(value);
    propertyValues.push_back(value);
  }
  if (selection.propertyIdsUnassigned &&
      !propertySeen.contains(
          std::string(core::constants::filters::kUnassigned))) {
    propertyValues.push_back(
        std::string(core::constants::filters::kUnassigned));
  }
  if (!propertyValues.empty()) {
    std::ostringstream propertyClause;
    propertyClause << "propertyId=";
    for (std::size_t i = 0; i < propertyValues.size(); ++i) {
      if (i > 0) {
        propertyClause << ',';
      }
      propertyClause << propertyValues[i];
    }
    clauses.push_back(propertyClause.str());
  }

  std::unordered_set<std::string> contractSeen;
  std::vector<std::string> contractValues;
  for (auto raw : selection.contractTypes) {
    raw = toLower(trim(raw));
    if (raw.empty() || contractSeen.contains(raw)) {
      continue;
    }
    contractSeen.insert(raw);
    contractValues.push_back(raw);
  }
  if (selection.contractTypesUnassigned &&
      !contractSeen.contains(
          std::string(core::constants::filters::kUnassigned))) {
    contractValues.push_back(
        std::string(core::constants::filters::kUnassigned));
  }
  if (!contractValues.empty()) {
    std::ostringstream contractClause;
    contractClause << "contract.type=";
    for (std::size_t i = 0; i < contractValues.size(); ++i) {
      if (i > 0) {
        contractClause << ',';
      }
      contractClause << contractValues[i];
    }
    clauses.push_back(contractClause.str());
  }

  const std::string allocatable = toLower(trim(selection.allocatableMode));
  if (allocatable == "allocatable" || allocatable == "non-allocatable") {
    clauses.push_back("allocatable=" + allocatable);
  }

  std::ostringstream out;
  for (std::size_t i = 0; i < clauses.size(); ++i) {
    if (i > 0) {
      out << ';';
    }
    out << clauses[i];
  }
  return core::domain::FilterSpec::normalize(out.str());
}

} // namespace core::ports::analysis
