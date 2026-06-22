#include "AnalysisFilter.h"

#include "core/constants/analysis.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Transaction.h"

#include <algorithm>
#include <cctype>
#include <memory>
#include <optional>
#include <sstream>
#include <string_view>

namespace core::application::analysis {

namespace {

using core::domain::Transaction;
using core::domain::catalog::WorkspaceCatalog;

enum class AnalysisFilterOperator {
  GreaterEqual,
  LessEqual,
  NotEqual,
  Greater,
  Less,
  Equal
};

enum class AnalysisDateField { BookingDate, Valuta };

struct AnalysisFilterClause {
  std::string key;
  std::string value;
  AnalysisFilterOperator op;
};

std::string trim(const std::string &value) {
  std::size_t first = 0;
  while (first < value.size() &&
         std::isspace(static_cast<unsigned char>(value[first])))
    ++first;

  std::size_t last = value.size();
  while (last > first &&
         std::isspace(static_cast<unsigned char>(value[last - 1])))
    --last;

  return value.substr(first, last - first);
}

std::string cleanValue(const std::string &value);

std::vector<std::string> splitList(const std::string &value) {
  std::vector<std::string> wanted;
  std::string current;
  for (const char ch : cleanValue(value)) {
    if (ch == core::constants::filters::separators::kAlternatives ||
        ch == core::constants::filters::separators::kList) {
      const auto token = trim(current);
      if (!token.empty())
        wanted.push_back(token);
      current.clear();
      continue;
    }

    current.push_back(ch);
  }

  const auto tail = trim(current);
  if (!tail.empty())
    wanted.push_back(tail);
  return wanted;
}

std::string cleanValue(const std::string &value) {
  auto cleaned = trim(value);
  if (cleaned.size() >= 2 &&
      ((cleaned.front() == '"' && cleaned.back() == '"') ||
       (cleaned.front() == '\'' && cleaned.back() == '\''))) {
    cleaned = cleaned.substr(1, cleaned.size() - 2);
  }
  return trim(cleaned);
}

std::string toLowerStr(const std::string &value) {
  std::string lowered;
  lowered.reserve(value.size());
  for (const char ch : value) {
    lowered.push_back(
        static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
  }
  return lowered;
}

int dateToInt(const std::string &value) {
  if (value.empty())
    return 0;

  std::string cleaned = cleanValue(value);
  if (cleaned.find('-') != std::string::npos) {
    std::string digits = cleaned;
    digits.erase(std::remove(digits.begin(), digits.end(), '-'), digits.end());
    if (digits.size() == 8) {
      try {
        return std::stoi(digits);
      } catch (...) {
        return 0;
      }
    }
    return 0;
  }

  if (cleaned.find(core::constants::filters::separators::kDateParts) !=
      std::string::npos) {
    std::vector<std::string> parts;
    std::istringstream stream(cleaned);
    std::string token;
    while (std::getline(stream, token,
                        core::constants::filters::separators::kDateParts)) {
      parts.push_back(trim(token));
    }

    if (parts.size() == 3) {
      std::string dd = parts[0];
      std::string mm = parts[1];
      std::string yyyy = parts[2];
      if (yyyy.size() == 4 && !dd.empty() && !mm.empty()) {
        if (dd.size() == 1)
          dd = std::string("0") + dd;
        if (mm.size() == 1)
          mm = std::string("0") + mm;
        const std::string out = yyyy + mm + dd;
        try {
          return std::stoi(out);
        } catch (...) {
          return 0;
        }
      }
    }
    return 0;
  }

  std::string digits;
  for (const char ch : cleaned) {
    if (std::isdigit(static_cast<unsigned char>(ch)))
      digits.push_back(ch);
  }
  if (digits.size() == 8) {
    try {
      return std::stoi(digits);
    } catch (...) {
      return 0;
    }
  }
  return 0;
}

bool compareInt(AnalysisFilterOperator op, int lhs, int rhs) {
  switch (op) {
  case AnalysisFilterOperator::GreaterEqual:
    return lhs >= rhs;
  case AnalysisFilterOperator::LessEqual:
    return lhs <= rhs;
  case AnalysisFilterOperator::Greater:
    return lhs > rhs;
  case AnalysisFilterOperator::Less:
    return lhs < rhs;
  case AnalysisFilterOperator::Equal:
    return lhs == rhs;
  case AnalysisFilterOperator::NotEqual:
    return lhs != rhs;
  }
  return false;
}

bool compareDouble(AnalysisFilterOperator op, double lhs, double rhs) {
  switch (op) {
  case AnalysisFilterOperator::GreaterEqual:
    return lhs >= rhs;
  case AnalysisFilterOperator::LessEqual:
    return lhs <= rhs;
  case AnalysisFilterOperator::Greater:
    return lhs > rhs;
  case AnalysisFilterOperator::Less:
    return lhs < rhs;
  case AnalysisFilterOperator::Equal:
    return lhs == rhs;
  case AnalysisFilterOperator::NotEqual:
    return lhs != rhs;
  }
  return false;
}

std::optional<AnalysisFilterClause> parseClause(const std::string &token) {
  struct OperatorSpec {
    std::string_view token;
    AnalysisFilterOperator op;
  };

  static const OperatorSpec operators[] = {
      {core::constants::filters::operators::kGreaterEqual,
       AnalysisFilterOperator::GreaterEqual},
      {core::constants::filters::operators::kLessEqual,
       AnalysisFilterOperator::LessEqual},
      {core::constants::filters::operators::kNotEqual,
       AnalysisFilterOperator::NotEqual},
      {core::constants::filters::operators::kGreater,
       AnalysisFilterOperator::Greater},
      {core::constants::filters::operators::kLess,
       AnalysisFilterOperator::Less},
      {core::constants::filters::operators::kEqual,
       AnalysisFilterOperator::Equal},
  };

  for (const auto &spec : operators) {
    const auto pos = token.find(spec.token);
    if (pos == std::string::npos)
      continue;

    return AnalysisFilterClause{
        trim(token.substr(0, pos)),
        trim(token.substr(pos + spec.token.size())),
        spec.op,
    };
  }

  return std::nullopt;
}

std::vector<std::string> splitContractTypes(const std::string &value) {
  std::vector<std::string> wanted;
  std::string current;
  for (const char ch : toLowerStr(cleanValue(value))) {
    if (ch == core::constants::filters::separators::kAlternatives ||
        ch == core::constants::filters::separators::kList) {
      const auto token = trim(current);
      if (!token.empty())
        wanted.push_back(token);
      current.clear();
      continue;
    }

    current.push_back(ch);
  }

  const auto tail = trim(current);
  if (!tail.empty())
    wanted.push_back(tail);
  return wanted;
}

AnalysisDateField parseDateField(const std::string &value) {
  return toLowerStr(cleanValue(value)) == "valuta"
             ? AnalysisDateField::Valuta
             : AnalysisDateField::BookingDate;
}

void addDatePredicate(core::application::analysis::AnalysisFilter &filter,
                      const AnalysisFilterClause &clause,
                      AnalysisDateField dateField) {
  const int target = dateToInt(clause.value);
  filter.addPredicate([target, op = clause.op, dateField](
                          const std::shared_ptr<Transaction> &transaction,
                          const WorkspaceCatalog &) {
    if (!transaction)
      return false;
    const std::string sourceDate = dateField == AnalysisDateField::Valuta
                                       ? transaction->valuta()
                                       : transaction->bookingDate();
    return compareInt(op, dateToInt(sourceDate), target);
  });
}

void addAmountPredicate(core::application::analysis::AnalysisFilter &filter,
                        const AnalysisFilterClause &clause) {
  double value = 0.0;
  try {
    value = std::stod(clause.value);
  } catch (...) {
    value = 0.0;
  }

  filter.addPredicate(
      [value, op = clause.op](const std::shared_ptr<Transaction> &transaction,
                              const WorkspaceCatalog &) {
        return transaction && compareDouble(op, transaction->amount(), value);
      });
}

void addContractTypePredicate(
    core::application::analysis::AnalysisFilter &filter,
    const AnalysisFilterClause &clause) {
  const auto wanted = splitContractTypes(clause.value);
  if (wanted.empty())
    return;

  filter.addPredicate([wanted](const std::shared_ptr<Transaction> &transaction,
                               const WorkspaceCatalog &state) {
    if (!transaction)
      return false;

    const bool allowUnassigned =
        std::find(wanted.begin(), wanted.end(),
                  std::string(core::constants::filters::kUnassigned)) !=
        wanted.end();
    if (transaction->contractId().empty())
      return allowUnassigned;

    const std::string contractId =
        toLowerStr(cleanValue(transaction->contractId()));
    for (const auto &contract : state.contracts()) {
      if (!contract)
        continue;
      if (toLowerStr(cleanValue(contract->id())) != contractId)
        continue;

      const std::string contractType = toLowerStr(cleanValue(contract->type()));
      return std::find(wanted.begin(), wanted.end(), contractType) !=
             wanted.end();
    }

    return false;
  });
}

void addPropertyPredicate(core::application::analysis::AnalysisFilter &filter,
                          const AnalysisFilterClause &clause) {
  const auto wanted = splitList(clause.value);
  if (wanted.empty())
    return;

  filter.addPredicate([wanted](const std::shared_ptr<Transaction> &transaction,
                               const WorkspaceCatalog &state) {
    if (!transaction)
      return false;
    const bool allowUnassigned =
        std::find(wanted.begin(), wanted.end(),
                  std::string(core::constants::filters::kUnassigned)) !=
        wanted.end();
    bool hasAnyProperty = false;

    for (const auto &propertyId : wanted) {
      if (std::find(transaction->propertyIds().begin(),
                    transaction->propertyIds().end(),
                    propertyId) != transaction->propertyIds().end()) {
        return true;
      }
    }
    if (!transaction->propertyIds().empty()) {
      hasAnyProperty = true;
    }

    if (transaction->contractId().empty())
      return allowUnassigned && !hasAnyProperty;
    const std::string contractId = cleanValue(transaction->contractId());
    for (const auto &contract : state.contracts()) {
      if (!contract)
        continue;
      if (cleanValue(contract->id()) != contractId)
        continue;

      if (!contract->propertyIds().empty()) {
        hasAnyProperty = true;
      }
      for (const auto &propertyId : wanted) {
        if (std::find(contract->propertyIds().begin(),
                      contract->propertyIds().end(),
                      propertyId) != contract->propertyIds().end()) {
          return true;
        }
      }
      return allowUnassigned && !hasAnyProperty;
    }

    return allowUnassigned && !hasAnyProperty;
  });
}

void addAllocatablePredicate(
    core::application::analysis::AnalysisFilter &filter,
    const AnalysisFilterClause &clause) {
  const std::string mode = toLowerStr(cleanValue(clause.value));
  if (mode != "allocatable" && mode != "non-allocatable")
    return;

  const bool expected = mode == "allocatable";
  filter.addPredicate(
      [expected](const std::shared_ptr<Transaction> &transaction,
                 const WorkspaceCatalog &) {
        return transaction && transaction->isAllocatable() == expected;
      });
}

} // namespace

AnalysisFilter parseAnalysisFilterSpec(const std::string &spec) {
  AnalysisFilter f;
  if (spec.empty())
    return f;

  std::vector<AnalysisFilterClause> clauses;
  AnalysisDateField dateField = AnalysisDateField::BookingDate;
  std::istringstream ss(spec);
  std::string token;
  while (
      std::getline(ss, token, core::constants::filters::separators::kClause)) {
    token = trim(token);
    if (token.empty())
      continue;

    const auto clause = parseClause(token);
    if (!clause)
      continue;
    if (clause->key == core::constants::filters::kDateField) {
      dateField = parseDateField(clause->value);
      continue;
    }
    clauses.push_back(*clause);
  }

  for (const auto &clause : clauses) {
    if (clause.key == core::constants::filters::kDate) {
      addDatePredicate(f, clause, dateField);
    } else if (clause.key == core::constants::filters::kAmount) {
      addAmountPredicate(f, clause);
    } else if (clause.key == core::constants::filters::kContractType) {
      addContractTypePredicate(f, clause);
    } else if (clause.key == core::constants::filters::kPropertyId) {
      addPropertyPredicate(f, clause);
    } else if (clause.key == core::constants::filters::kAllocatable) {
      addAllocatablePredicate(f, clause);
    }
  }

  return f;
}

std::vector<std::shared_ptr<core::domain::Transaction>>
collectAnalysisTransactions(
    const core::domain::catalog::WorkspaceCatalog &state,
    const AnalysisFilter &filter) {
  std::vector<std::shared_ptr<core::domain::Transaction>> out;
  out.reserve(state.transactions().size());

  for (const auto &transaction : state.transactions()) {
    if (!transaction) {
      continue;
    }
    if (!filter.empty() && !filter.matches(transaction, state)) {
      continue;
    }
    out.push_back(transaction);
  }

  return out;
}

} // namespace core::application::analysis
