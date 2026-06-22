#pragma once

#include <string_view>

namespace core::application::analysis::keys {

inline constexpr std::string_view kTypeTab = "tab";
inline constexpr std::string_view kTypePlot = "plot";
inline constexpr std::string_view kTypeCalculation = "calc";
inline constexpr std::string_view kPlotTypeKey = "plotType";
inline constexpr std::string_view kPlotMeasureKey = "plotMeasure";
inline constexpr std::string_view kPropertiesKey = "properties";
inline constexpr std::string_view kContractTypesKey = "contractTypes";

namespace plotTypes {
inline constexpr std::string_view kPie = "pie";
inline constexpr std::string_view kHistogram = "histogram";
} // namespace plotTypes

namespace plotMeasures {
inline constexpr std::string_view kCount = "count";
inline constexpr std::string_view kAverageAmount = "averageAmount";
inline constexpr std::string_view kTotalAmount = "totalAmount";
} // namespace plotMeasures

namespace metricKeys {
inline constexpr std::string_view kTotalAmount = "totalAmount";
inline constexpr std::string_view kRowCount = "rowCount";
inline constexpr std::string_view kMatchedTransactions = "matchedTx";
inline constexpr std::string_view kRows = "rows";
} // namespace metricKeys

namespace calculation {
inline constexpr std::string_view kStrategyKey = "strategy";
inline constexpr std::string_view kPercentKey = "percent";
inline constexpr std::string_view kStrategyTax = "tax";
} // namespace calculation

namespace resultFields {
inline constexpr std::string_view kMonth = "month";
inline constexpr std::string_view kTotal = "total";
inline constexpr std::string_view kByContract = "byContract";
inline constexpr std::string_view kByProperty = "byProperty";
inline constexpr std::string_view kAmountOriginal = "amount_original";
inline constexpr std::string_view kAmountAdjusted = "amount_adjusted";
inline constexpr std::string_view kTaxPercent = "taxPercent";
inline constexpr std::string_view kTaxFactor = "taxFactor";
inline constexpr std::string_view kTransactionId = "txId";
} // namespace resultFields

namespace labels {
inline constexpr std::string_view kNoProperty = "(no-property)";
} // namespace labels

} // namespace core::application::analysis::keys

namespace core::application::analysis::filterKeys {

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

} // namespace core::application::analysis::filterKeys
