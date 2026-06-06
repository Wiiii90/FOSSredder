#pragma once

#include <string_view>

namespace core::constants::filters {

inline constexpr std::string_view kDate = "date";
inline constexpr std::string_view kDateField = "dateField";
inline constexpr std::string_view kAmount = "amount";
inline constexpr std::string_view kContractType = "contract.type";
inline constexpr std::string_view kPropertyId = "propertyId";
inline constexpr std::string_view kAllocatable = "allocatable";
inline constexpr std::string_view kUnassigned = "unassigned";

namespace operators {
inline constexpr std::string_view kGreaterEqual = ">=";
inline constexpr std::string_view kLessEqual = "<=";
inline constexpr std::string_view kNotEqual = "!=";
inline constexpr std::string_view kGreater = ">";
inline constexpr std::string_view kLess = "<";
inline constexpr std::string_view kEqual = "=";
} // namespace operators

namespace separators {
inline constexpr char kClause = ';';
inline constexpr char kAlternatives = '|';
inline constexpr char kList = ',';
inline constexpr char kDateParts = '.';
} // namespace separators

} // namespace core::constants::filters
