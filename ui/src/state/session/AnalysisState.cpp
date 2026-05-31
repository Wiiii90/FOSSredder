/**
 * @file ui/src/state/session/AnalysisState.cpp
 * @brief Implements the UI state adapter for the Analysis view.
 */

#include "ui/state/session/AnalysisState.h"

#include <algorithm>
#include <cmath>
#include <optional>

#include <QDate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QSet>
#include <QUrl>

#include "ui/viewmodels/system/SettingsViewModel.h"
#include "ui/shared/payload/PayloadKeys.h"
#include "ui/workflows/analysis/AnalysisWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

namespace {

constexpr int kPreviewDebounceMs = 40;
constexpr auto kPlot = "plot";
constexpr auto kTable = "table";
constexpr auto kTab = "tab";
constexpr auto kTabular = "tabular";
constexpr auto kPie = "pie";
constexpr auto kHistogram = "histogram";
constexpr auto kPng = "png";
constexpr auto kJpg = "jpg";
constexpr auto kXlsx = "xlsx";
constexpr auto kCsv = "csv";
constexpr auto kAll = "all";
constexpr auto kAllocatable = "allocatable";
constexpr auto kNonAllocatable = "non-allocatable";
constexpr auto kUnassigned = "unassigned";

QString qstr(const char *value) { return QString::fromLatin1(value); }

QString normalizedType(const QString &value) {
  const QString trimmed = value.trimmed().toLower();
  return trimmed == qstr(kTabular) ? qstr(kTab) : trimmed;
}

double numberFromVariant(const QVariant &value) {
  bool ok = false;
  const double out = value.toDouble(&ok);
  return ok && std::isfinite(out) ? out : 0.0;
}

std::optional<double> parsePercentInput(const QString &text) {
  QString normalized = text.trimmed();
  if (normalized.isEmpty()) {
    return std::nullopt;
  }

  if (normalized.endsWith(QLatin1Char('%'))) {
    normalized.chop(1);
    normalized = normalized.trimmed();
  }
  normalized.remove(QLatin1Char(' '));

  bool ok = false;
  const double direct = normalized.toDouble(&ok);
  if (ok && std::isfinite(direct)) {
    return direct;
  }

  const int lastComma = normalized.lastIndexOf(QLatin1Char(','));
  const int lastDot = normalized.lastIndexOf(QLatin1Char('.'));
  if (lastComma >= 0 && lastDot >= 0) {
    if (lastComma > lastDot) {
      normalized.remove(QLatin1Char('.'));
      normalized.replace(QLatin1Char(','), QLatin1Char('.'));
    } else {
      normalized.remove(QLatin1Char(','));
    }
  } else if (lastComma >= 0) {
    normalized.replace(QLatin1Char(','), QLatin1Char('.'));
  }

  const double parsed = normalized.toDouble(&ok);
  if (ok && std::isfinite(parsed)) {
    return parsed;
  }
  return std::nullopt;
}

QString nonEmptyString(const QVariantMap &map, const QString &key,
                       const QString &fallback = {}) {
  const QString value = map.value(key).toString();
  return value.isEmpty() ? fallback : value;
}

QString amountText(double value) {
  return QString::number(value, 'f', 2);
}

bool containsPropertyId(const QVariantList &rows, const QString &id) {
  for (const QVariant &value : rows) {
    if (value.toMap().value(QStringLiteral("id")).toString() == id) {
      return true;
    }
  }
  return false;
}

QString rowKey(const QVariant &value, const QString &idField) {
  if (!idField.isEmpty()) {
    return value.toMap().value(idField).toString();
  }
  const QVariantMap row = value.toMap();
  if (row.contains(QStringLiteral("value"))) {
    return row.value(QStringLiteral("value")).toString();
  }
  return value.toString();
}

bool containsRowValue(const QVariantList &rows, const QString &value) {
  for (const QVariant &row : rows) {
    if (rowKey(row, {}) == value) {
      return true;
    }
  }
  return false;
}

void appendContractTypeRow(QVariantList &rows, const QString &value,
                           const QString &label) {
  if (value.isEmpty() || containsRowValue(rows, value)) {
    return;
  }
  rows.push_back(QVariantMap{{QStringLiteral("value"), value},
                             {QStringLiteral("label"), label}});
}

} // namespace

AnalysisState::AnalysisState(QObject *parent) : QObject(parent) {
  previewDebounce_.setInterval(kPreviewDebounceMs);
  previewDebounce_.setSingleShot(true);
  connect(&previewDebounce_, &QTimer::timeout, this,
          &AnalysisState::refreshPreview);
}

void AnalysisState::setWorkspace(WorkspaceFacade *value) {
  if (workspace_ == value) {
    return;
  }
  bindWorkspace(value);
  refreshFromSelection();
  emitChanged();
}

void AnalysisState::setAnalysisWorkflow(AnalysisWorkflow *value) {
  if (analysisWorkflow_ == value) {
    return;
  }
  analysisWorkflow_ = value;
  refreshFromSelection();
  emitChanged();
}

void AnalysisState::setSettingsViewModel(SettingsViewModel *value) {
  if (settingsViewModel_ == value) {
    return;
  }
  settingsViewModel_ = value;
  refreshFromSelection();
  emitChanged();
}

bool AnalysisState::isEdit() const { return !selectedAnalysisId().isEmpty(); }

void AnalysisState::setName(const QString &value) {
  if (name_ == value) {
    return;
  }
  name_ = value;
  emitChanged();
}

void AnalysisState::setMainTypeIndex(int value) {
  const int next = value == 1 ? 1 : 0;
  if (mainTypeIndex_ == next) {
    return;
  }
  mainTypeIndex_ = next;
  activeResultType_ = strategyType();
  exportFormat_ = normalizedExportFormat(exportFormat_, uiType());
  emitChanged();
}

void AnalysisState::setPlotSubtypeIndex(int value) {
  const int next = value == 1 ? 1 : 0;
  if (plotSubtypeIndex_ == next) {
    return;
  }
  plotSubtypeIndex_ = next;
  emitChanged();
}

QVariantList AnalysisState::plotTypeOptions() const {
  return {QVariantMap{{QStringLiteral("value"), qstr(kPie)},
                      {QStringLiteral("label"), tr("Pie chart")}},
          QVariantMap{{QStringLiteral("value"), qstr(kHistogram)},
                      {QStringLiteral("label"), tr("Histogram")}}};
}

QVariantList AnalysisState::exportFormatOptions() const {
  if (uiType() == qstr(kTable)) {
    return {QVariantMap{{QStringLiteral("value"), qstr(kXlsx)},
                        {QStringLiteral("label"), tr("XLSX")}},
            QVariantMap{{QStringLiteral("value"), qstr(kCsv)},
                        {QStringLiteral("label"), tr("CSV")}}};
  }
  return {QVariantMap{{QStringLiteral("value"), qstr(kPng)},
                      {QStringLiteral("label"), tr("PNG")}},
          QVariantMap{{QStringLiteral("value"), qstr(kJpg)},
                      {QStringLiteral("label"), tr("JPG")}}};
}

void AnalysisState::setExportFormat(const QString &value) {
  const QString next = normalizedExportFormat(value, uiType());
  if (exportFormat_ == next) {
    return;
  }
  exportFormat_ = next;
  emitChanged();
}

int AnalysisState::exportFormatIndex() const {
  const QVariantList options = exportFormatOptions();
  const QString current = normalizedExportFormat(exportFormat_, uiType());
  for (int index = 0; index < options.size(); ++index) {
    if (options.at(index).toMap().value(QStringLiteral("value")).toString() ==
        current) {
      return index;
    }
  }
  return 0;
}

void AnalysisState::setExportFormatIndex(int value) {
  const QVariantList options = exportFormatOptions();
  if (options.isEmpty()) {
    return;
  }
  const int index = std::clamp(value, 0, static_cast<int>(options.size()) - 1);
  setExportFormat(
      options.at(index).toMap().value(QStringLiteral("value")).toString());
}

void AnalysisState::setIncludeCalcAdjustments(bool value) {
  if (includeCalcAdjustments_ == value) {
    return;
  }
  includeCalcAdjustments_ = value;
  refreshAnalysisResult();
  updateResultState();
  emitChanged();
}

void AnalysisState::setExportStateJson(const QString &value) {
  const QString next = normalizedExportStateJson(value);
  if (exportStateJson_ == next) {
    return;
  }
  exportStateJson_ = next;
  emitChanged();
}

void AnalysisState::setFilterEditMode(bool value) {
  if (filterEditMode_ == value) {
    return;
  }
  filterEditMode_ = value;
  emitChanged();
}

void AnalysisState::setDateFieldIndex(int value) {
  const int next = value == 1 ? 1 : 0;
  if (dateFieldIndex_ == next) {
    return;
  }
  dateFieldIndex_ = next;
  requestPreviewRefresh();
  emitChanged();
}

void AnalysisState::setDateModeIndex(int value) {
  const int next = value == 1 ? 1 : 0;
  if (dateModeIndex_ == next) {
    return;
  }
  dateModeIndex_ = next;
  requestPreviewRefresh();
  emitChanged();
}

void AnalysisState::setYearValue(const QString &value) {
  if (yearValue_ == value) {
    return;
  }
  yearValue_ = value;
  requestPreviewRefresh();
  emitChanged();
}

void AnalysisState::setDateFromValue(const QString &value) {
  if (dateFromValue_ == value) {
    return;
  }
  dateFromValue_ = value;
  requestPreviewRefresh();
  emitChanged();
}

void AnalysisState::setDateToValue(const QString &value) {
  if (dateToValue_ == value) {
    return;
  }
  dateToValue_ = value;
  requestPreviewRefresh();
  emitChanged();
}

void AnalysisState::setSelectedPropertyIds(const QVariantList &value) {
  if (selectedPropertyIds_ == value) {
    return;
  }
  selectedPropertyIds_ = value;
  requestPreviewRefresh();
  emitChanged();
}

void AnalysisState::setSelectedContractTypes(const QVariantList &value) {
  if (selectedContractTypes_ == value) {
    return;
  }
  selectedContractTypes_ = value;
  requestPreviewRefresh();
  emitChanged();
}

void AnalysisState::setAllocatableMode(const QString &value) {
  const QString lower = value.trimmed().toLower();
  const QString next = lower == qstr(kAllocatable) ||
                               lower == qstr(kNonAllocatable)
                           ? lower
                           : qstr(kAll);
  if (allocatableMode_ == next) {
    return;
  }
  allocatableMode_ = next;
  requestPreviewRefresh();
  emitChanged();
}

QVariantList AnalysisState::previewTransactionRows() const {
  QVariantList rows;
  rows.reserve(previewTransactions_.size());
  const QString calculationLabel =
      calcName_.trimmed().isEmpty() ? tr("calc") : calcName_.trimmed();

  for (const QVariant &value : previewTransactions_) {
    const QVariantMap source = value.toMap();
    const QString txId = source.value(QStringLiteral("id")).toString();
    const double baseAmount =
        numberFromVariant(source.value(QStringLiteral("amount")));
    const bool hasAdjusted =
        includeCalcAdjustments_ && !txId.isEmpty() &&
        adjustmentAmountsById_.contains(txId);
    const double adjustedAmount =
        hasAdjusted ? numberFromVariant(adjustmentAmountsById_.value(txId))
                    : baseAmount;
    QString renderedAmount = amountText(baseAmount);
    if (hasAdjusted) {
      renderedAmount = QStringLiteral("%1 (%2 %3 %4)")
                           .arg(amountText(baseAmount), amountText(adjustedAmount),
                                tr("plus"), calculationLabel);
    }

    rows.push_back(QVariantMap{
        {QStringLiteral("id"), txId},
        {QStringLiteral("statementName"),
         source.value(QStringLiteral("statementName")).toString()},
        {QStringLiteral("transactionName"),
         source.value(QStringLiteral("transactionName")).toString()},
        {QStringLiteral("date"), source.value(QStringLiteral("date")).toString()},
        {QStringLiteral("valuta"),
         source.value(QStringLiteral("valuta")).toString()},
        {QStringLiteral("actorName"),
         source.value(QStringLiteral("actorName")).toString()},
        {QStringLiteral("contractName"),
         source.value(QStringLiteral("contractName")).toString()},
        {QStringLiteral("contractType"),
         nonEmptyString(source, QStringLiteral("contractType"), tr("Unassigned"))},
        {QStringLiteral("propertiesLabel"),
         source.value(QStringLiteral("propertiesLabel")).toString()},
        {QStringLiteral("amountText"), renderedAmount}});
  }
  return rows;
}

QString AnalysisState::previewStatementCountText() const {
  const int count = previewMetrics_.value(QStringLiteral("statementCount")).toInt();
  return tr("Statements: %1").arg(count);
}

QString AnalysisState::previewTransactionCountText() const {
  const int count =
      previewMetrics_.value(QStringLiteral("transactionCount")).toInt();
  return tr("Transactions: %1").arg(count);
}

QString AnalysisState::previewAmountSumText() const {
  const double amount =
      numberFromVariant(previewMetrics_.value(QStringLiteral("amountSum")));
  return tr("Amount sum: %1").arg(amountText(amount));
}

void AnalysisState::setSelectedAdjustmentTxIds(const QVariantList &value) {
  if (selectedAdjustmentTxIds_ == value) {
    return;
  }
  selectedAdjustmentTxIds_ = value;
  emitChanged();
}

void AnalysisState::setCalcName(const QString &value) {
  if (calcName_ == value) {
    return;
  }
  calcName_ = value;
  emitChanged();
}

void AnalysisState::setCalcPercentText(const QString &value) {
  if (calcPercentText_ == value) {
    return;
  }
  calcPercentText_ = value;
  emitChanged();
}

QString AnalysisState::currentResultType() const {
  const QVariantMap row = analysisRowById(selectedAnalysisId());
  const QString type =
      normalizedType(nonEmptyString(row, QStringLiteral("type"), activeResultType_));
  return type.isEmpty() ? activeResultType_ : type;
}

bool AnalysisState::currentResultIsTable() const {
  return currentResultType() == qstr(kTab);
}

bool AnalysisState::canSubmit() const {
  return !name_.trimmed().isEmpty() && !selectedPropertyIds_.isEmpty() &&
         !selectedContractTypes_.isEmpty();
}

bool AnalysisState::hasRows() const { return !analysisRows().isEmpty(); }

void AnalysisState::refreshFromSelection() {
  ensureChoices();
  loadSelectedAnalysis();
  updateResultState();
  emitChanged();
}

void AnalysisState::refreshPreview() {
  if (!analysisWorkflow_) {
    return;
  }
  const QVariantMap preview =
      analysisWorkflow_->previewTransactions(currentFilterSpec());
  previewTransactions_ = preview.value(QStringLiteral("transactions")).toList();
  previewMetrics_ = preview.value(QStringLiteral("metrics")).toMap();
  if (previewMetrics_.isEmpty()) {
    previewMetrics_.insert(QStringLiteral("statementCount"), 0);
    previewMetrics_.insert(QStringLiteral("transactionCount"), 0);
    previewMetrics_.insert(QStringLiteral("amountSum"), 0.0);
  }
  emitChanged();
}

void AnalysisState::requestPreviewRefresh() { previewDebounce_.start(); }

void AnalysisState::applySelectedCalc() {
  refreshPendingAdjustmentsFromCalcSelection();
  refreshAnalysisResult();
  updateResultState();
  emitChanged();
}

bool AnalysisState::isPropertySelected(const QString &id) const {
  return selectedPropertyIds_.contains(id);
}

void AnalysisState::setPropertySelected(const QString &id, bool selected) {
  const QString trimmed = id.trimmed();
  if (trimmed.isEmpty()) {
    return;
  }
  QVariantList next = selectedPropertyIds_;
  if (selected && !next.contains(trimmed)) {
    next.push_back(trimmed);
  }
  if (!selected) {
    next.removeAll(trimmed);
  }
  setSelectedPropertyIds(next);
}

void AnalysisState::selectAllProperties() { setSelectedPropertyIds(allPropertyIds()); }

void AnalysisState::selectNoProperties() { setSelectedPropertyIds({}); }

void AnalysisState::selectUnassignedProperties() {
  setSelectedPropertyIds({qstr(kUnassigned)});
}

bool AnalysisState::isContractTypeSelected(const QString &type) const {
  return selectedContractTypes_.contains(type);
}

void AnalysisState::setContractTypeSelected(const QString &type, bool selected) {
  const QString trimmed = type.trimmed();
  if (trimmed.isEmpty()) {
    return;
  }
  QVariantList next = selectedContractTypes_;
  if (selected && !next.contains(trimmed)) {
    next.push_back(trimmed);
  }
  if (!selected) {
    next.removeAll(trimmed);
  }
  setSelectedContractTypes(next);
}

void AnalysisState::selectAllContractTypes() {
  setSelectedContractTypes(allContractTypes());
}

void AnalysisState::selectNoContractTypes() { setSelectedContractTypes({}); }

void AnalysisState::selectUnassignedContractTypes() {
  setSelectedContractTypes({qstr(kUnassigned)});
}

bool AnalysisState::isAdjustmentTransactionSelected(const QString &id) const {
  return selectedAdjustmentTxIds_.contains(id);
}

void AnalysisState::setAdjustmentTransactionSelected(const QString &id,
                                                     bool selected) {
  const QString trimmed = id.trimmed();
  if (trimmed.isEmpty()) {
    return;
  }
  QVariantList next = selectedAdjustmentTxIds_;
  if (selected && !next.contains(trimmed)) {
    next.push_back(trimmed);
  }
  if (!selected) {
    next.removeAll(trimmed);
  }
  setSelectedAdjustmentTxIds(next);
}

void AnalysisState::setAllocatableModeIndex(int index) {
  if (index == 1) {
    setAllocatableMode(qstr(kAllocatable));
    return;
  }
  if (index == 2) {
    setAllocatableMode(qstr(kNonAllocatable));
    return;
  }
  setAllocatableMode(qstr(kAll));
}

void AnalysisState::toggleFilterWorkspace() {
  filterWorkspaceIndex_ = filterWorkspaceIndex_ == 0 ? 1 : 0;
  emitChanged();
}

void AnalysisState::clearFilters() {
  dateFieldIndex_ = 0;
  dateModeIndex_ = 0;
  yearValue_ = defaultAnalysisYear();
  dateFromValue_.clear();
  dateToValue_.clear();
  selectedPropertyIds_ = allPropertyIds();
  selectedContractTypes_ = allContractTypes();
  allocatableMode_ = qstr(kAll);
  resetAdjustments();
  refreshPreview();
  emitChanged();
}

void AnalysisState::submitCreate() {
  if (!analysisWorkflow_ || !workspace_ || !canSubmit()) {
    return;
  }
  refreshPreview();
  refreshPendingAdjustmentsFromCalcSelection();
  const QString adjustmentsJson = pendingAdjustmentsJson_;

  const QString type = strategyType();
  activeResultType_ = type;
  const QString plotType =
      plotSubtypeIndex_ == 1 ? qstr(kHistogram) : qstr(kPie);
  const QString configJson = analysisWorkflow_->analysisConfigJson(
      type, plotType, QStringLiteral("totalAmount"),
      effectiveSelectedPropertyIds(), effectiveSelectedContractTypes(), 0.0);
  const QString filterSpec = currentFilterSpec();
  const QString selectedExportFormat = normalizedExportFormat(exportFormat_, uiType());
  snapshotTransactionsJson_ = previewSnapshotJson();
  const QString newId = analysisWorkflow_->createAnalysis(
      name_, type, configJson, filterSpec, selectedExportFormat,
      includeCalcAdjustments_, normalizedExportStateJson(exportStateJson_),
      snapshotTransactionsJson_, adjustmentsJson);
  if (newId.isEmpty()) {
    return;
  }
  workspace_->setSelectedAnalysisId(newId);
  pendingAdjustmentsJson_ = adjustmentsJson;
  adjustmentAmountsById_ = parseJsonObject(pendingAdjustmentsJson_);
  selectedAdjustmentTxIds_ = adjustmentIds(adjustmentAmountsById_);
  if (workspace_->analyses()) {
    workspace_->analyses()->setAdjustmentsById(newId, adjustmentsJson);
  }
  refreshAnalysisResult();
  filterEditMode_ = false;
  captureSavedState();
  emitChanged();
}

void AnalysisState::submitUpdate() {
  if (!workspace_ || selectedAnalysisId().isEmpty() || !canSubmit()) {
    return;
  }
  const QString adjustmentsJson = pendingAdjustmentsJson_;

  const QString type = strategyType();
  activeResultType_ = type;
  const QString plotType =
      plotSubtypeIndex_ == 1 ? qstr(kHistogram) : qstr(kPie);
  const QString configJson = analysisWorkflow_
                                 ? analysisWorkflow_->analysisConfigJson(
                                       type, plotType, QStringLiteral("totalAmount"),
                                       effectiveSelectedPropertyIds(),
                                       effectiveSelectedContractTypes(), 0.0)
                                 : QStringLiteral("{}");
  const QString filterSpec = currentFilterSpec();
  workspace_->updateAnalysis(selectedAnalysisId(), name_, type, configJson,
                             filterSpec, normalizedExportFormat(exportFormat_, uiType()),
                             includeCalcAdjustments_,
                             normalizedExportStateJson(exportStateJson_),
                             snapshotTransactionsJson_, adjustmentsJson);
  pendingAdjustmentsJson_ = adjustmentsJson;
  adjustmentAmountsById_ = parseJsonObject(pendingAdjustmentsJson_);
  selectedAdjustmentTxIds_ = adjustmentIds(adjustmentAmountsById_);
  if (workspace_->analyses()) {
    workspace_->analyses()->setAdjustmentsById(selectedAnalysisId(),
                                               adjustmentsJson);
  }
  refreshAnalysisResult();
  filterEditMode_ = false;
  captureSavedState();
  emitChanged();
}

void AnalysisState::deleteCurrent() {
  const QString removedId = selectedAnalysisId();
  if (removedId.isEmpty() || !analysisWorkflow_ || !workspace_) {
    return;
  }
  const int currentIndex = analysisIndexById(removedId);
  analysisWorkflow_->deleteAnalysis(removedId);
  const QVariantList rows = analysisRows();
  if (rows.isEmpty()) {
    workspace_->setSelectedAnalysisId(QString());
    return;
  }
  const int nextIndex =
      std::min(currentIndex, static_cast<int>(rows.size()) - 1);
  const QVariantMap nextRow = analysisRowAt(nextIndex);
  workspace_->setSelectedAnalysisId(nextRow.value(QStringLiteral("id")).toString());
  emitChanged();
}

void AnalysisState::navigate(int delta) {
  if (!workspace_ || analysisRows().isEmpty()) {
    return;
  }

  const QVariantList rows = analysisRows();
  const QString currentId = selectedAnalysisId();
  int index = analysisIndexById(currentId);
  if ((delta > 0 && index == rows.size() - 1) || (delta < 0 && index == 0)) {
    workspace_->setSelectedAnalysisId(QString());
    return;
  }
  if (index < 0) {
    index = delta > 0 ? -1 : rows.size();
  }

  const QVariantMap row = analysisRowAt(index + delta);
  workspace_->setSelectedAnalysisId(row.value(QStringLiteral("id")).toString());
}

void AnalysisState::selectAnalysis(const QString &id) {
  if (workspace_) {
    workspace_->setSelectedAnalysisId(id);
  }
}

QString AnalysisState::selectedAnalysisId() const {
  return workspace_ ? workspace_->selectedAnalysisId() : QString();
}

QVariantList AnalysisState::analysisRows() const {
  return workspace_ ? workspace_->analysisRows() : QVariantList();
}

QVariantMap AnalysisState::analysisRowById(const QString &id) const {
  if (id.isEmpty()) {
    return {};
  }
  for (const QVariant &value : analysisRows()) {
    const QVariantMap row = value.toMap();
    if (row.value(QStringLiteral("id")).toString() == id) {
      return row;
    }
  }
  return {};
}

QVariantMap AnalysisState::analysisRowAt(int index) const {
  const QVariantList rows = analysisRows();
  if (index < 0 || index >= rows.size()) {
    return {};
  }
  return rows.at(index).toMap();
}

int AnalysisState::analysisIndexById(const QString &id) const {
  if (id.isEmpty()) {
    return -1;
  }
  const QVariantList rows = analysisRows();
  for (int i = 0; i < rows.size(); ++i) {
    if (rows.at(i).toMap().value(QStringLiteral("id")).toString() == id) {
      return i;
    }
  }
  return -1;
}

QString AnalysisState::uiType() const {
  return mainTypeIndex_ == 1 ? qstr(kTable) : qstr(kPlot);
}

QString AnalysisState::strategyType() const {
  return uiType() == qstr(kTable) ? qstr(kTab) : qstr(kPlot);
}

QString AnalysisState::currentDateField() const {
  return dateFieldIndex_ == 1 ? QStringLiteral("valuta")
                              : QStringLiteral("bookingDate");
}

QString AnalysisState::currentDateMode() const {
  return dateModeIndex_ == 0 ? QStringLiteral("year") : QStringLiteral("range");
}

QString AnalysisState::defaultAnalysisDateMode() const {
  if (settingsViewModel_) {
    const QString mode = settingsViewModel_->analysisDefaultDateMode().toLower();
    return mode == QStringLiteral("range") ? QStringLiteral("range")
                                           : QStringLiteral("year");
  }
  return QStringLiteral("year");
}

QString AnalysisState::defaultAnalysisYear() const {
  if (settingsViewModel_ && settingsViewModel_->analysisDefaultYear() > 0) {
    return QString::number(settingsViewModel_->analysisDefaultYear());
  }
  return QString::number(QDate::currentDate().year() - 1);
}

QStringList AnalysisState::stringList(const QVariantList &values) const {
  QStringList out;
  out.reserve(values.size());
  for (const QVariant &value : values) {
    const QString string = value.toString();
    if (!string.isEmpty()) {
      out.push_back(string);
    }
  }
  return out;
}

QVariantList AnalysisState::allPropertyIds() const {
  QVariantList out;
  for (const QVariant &value : propertyFilterRows_) {
    const QString id = value.toMap().value(QStringLiteral("id")).toString();
    if (!id.isEmpty()) {
      out.push_back(id);
    }
  }
  return out;
}

QVariantList AnalysisState::allContractTypes() const {
  QVariantList out;
  for (const QVariant &value : contractTypeRows_) {
    const QString type = rowKey(value, {});
    if (!type.isEmpty()) {
      out.push_back(type);
    }
  }
  return out;
}

QVariantList AnalysisState::pruneSelection(const QVariantList &values,
                                           const QVariantList &availableRows,
                                           const QString &idField) const {
  QSet<QString> available;
  for (const QVariant &value : availableRows) {
    const QString key = rowKey(value, idField);
    if (!key.isEmpty()) {
      available.insert(key);
    }
  }

  QVariantList out;
  for (const QVariant &value : values) {
    const QString key = value.toString();
    if (!key.isEmpty() && available.contains(key)) {
      out.push_back(key);
    }
  }
  return out;
}

bool AnalysisState::isAllSelected(const QVariantList &selectedIds,
                                  const QVariantList &availableRows,
                                  const QString &idField) const {
  if (availableRows.isEmpty()) {
    return true;
  }
  QSet<QString> selected;
  for (const QVariant &value : selectedIds) {
    const QString key = value.toString();
    if (!key.isEmpty()) {
      selected.insert(key);
    }
  }
  if (selected.size() != availableRows.size()) {
    return false;
  }
  for (const QVariant &value : availableRows) {
    const QString key = rowKey(value, idField);
    if (key.isEmpty() || !selected.contains(key)) {
      return false;
    }
  }
  return true;
}

QStringList AnalysisState::effectiveSelectedPropertyIds() const {
  if (propertyFilterRows_.isEmpty() ||
      isAllSelected(selectedPropertyIds_, propertyFilterRows_,
                    QStringLiteral("id"))) {
    return {};
  }
  return selectedPropertyIds_.isEmpty() ? QStringList{}
                                        : stringList(selectedPropertyIds_);
}

QStringList AnalysisState::effectiveSelectedContractTypes() const {
  if (contractTypeRows_.isEmpty() ||
      isAllSelected(selectedContractTypes_, contractTypeRows_)) {
    return {};
  }
  return selectedContractTypes_.isEmpty() ? QStringList{}
                                          : stringList(selectedContractTypes_);
}

QString AnalysisState::currentFilterSpec() const {
  if (!analysisWorkflow_) {
    return {};
  }
  const QString mode = allocatableMode_.toLower();
  return analysisWorkflow_->analysisFilterSpec(
      currentDateField(), currentDateMode(), yearValue_, dateFromValue_,
      dateToValue_, effectiveSelectedPropertyIds(),
      effectiveSelectedContractTypes(),
      mode == qstr(kAllocatable) ||
              mode == qstr(kNonAllocatable)
          ? mode
          : QString());
}

QString AnalysisState::normalizedExportFormat(const QString &value,
                                              const QString &targetUiType) const {
  const QString normalized = value.trimmed().toLower();
  const QStringList allowed =
      targetUiType == qstr(kTable) ? QStringList{qstr(kXlsx), qstr(kCsv)}
                                   : QStringList{qstr(kPng), qstr(kJpg)};
  return allowed.contains(normalized) ? normalized : allowed.first();
}

QString AnalysisState::normalizedExportStateJson(const QString &value) const {
  const QJsonDocument document = QJsonDocument::fromJson(value.toUtf8());
  if (!document.isObject()) {
    return QStringLiteral("{}");
  }
  return QString::fromUtf8(document.toJson(QJsonDocument::Compact));
}

QVariantMap AnalysisState::parseJsonObject(const QString &value) const {
  const QJsonDocument document = QJsonDocument::fromJson(value.toUtf8());
  return document.isObject() ? document.object().toVariantMap() : QVariantMap();
}

QString AnalysisState::previewSnapshotJson() const {
  QJsonArray snapshot;
  for (const QVariant &value : previewTransactions_) {
    snapshot.push_back(QJsonObject::fromVariantMap(value.toMap()));
  }
  return QString::fromUtf8(QJsonDocument(snapshot).toJson(QJsonDocument::Compact));
}

QVariantList AnalysisState::adjustmentIds(const QVariantMap &adjustments) const {
  QVariantList ids;
  ids.reserve(adjustments.size());
  for (auto it = adjustments.cbegin(); it != adjustments.cend(); ++it) {
    if (!it.key().trimmed().isEmpty()) {
      ids.push_back(it.key());
    }
  }
  return ids;
}

void AnalysisState::ensureChoices() {
  const bool propertyWasAll =
      isAllSelected(selectedPropertyIds_, propertyFilterRows_, QStringLiteral("id"));
  const bool contractTypesWereAll =
      isAllSelected(selectedContractTypes_, contractTypeRows_);

  propertyFilterRows_ = workspace_ ? workspace_->propertyRows() : QVariantList();
  if (!containsPropertyId(propertyFilterRows_, qstr(kUnassigned))) {
    propertyFilterRows_.push_back(QVariantMap{
        {QStringLiteral("id"), qstr(kUnassigned)},
        {QStringLiteral("name"), tr("Unassigned")},
    });
  }
  selectedPropertyIds_ =
      propertyWasAll
          ? allPropertyIds()
          : pruneSelection(selectedPropertyIds_, propertyFilterRows_,
                           QStringLiteral("id"));

  QVariantList types;
  if (analysisWorkflow_) {
    for (const QString &type : analysisWorkflow_->contractTypes()) {
      appendContractTypeRow(types, type, type);
    }
  }
  appendContractTypeRow(types, qstr(kUnassigned), tr("Unassigned"));
  contractTypeRows_ = types;
  selectedContractTypes_ = contractTypesWereAll
                               ? allContractTypes()
                               : pruneSelection(selectedContractTypes_,
                                                contractTypeRows_);
}

void AnalysisState::resetAdjustments() {
  selectedAdjustmentTxIds_.clear();
  adjustmentAmountsById_.clear();
  pendingAdjustmentsJson_ = QStringLiteral("{}");
  calcName_.clear();
  calcPercentText_.clear();
}

void AnalysisState::refreshPendingAdjustmentsFromCalcSelection() {
  if (!analysisWorkflow_ || selectedAdjustmentTxIds_.isEmpty()) {
    return;
  }
  const double taxPercent = parsePercentInput(calcPercentText_).value_or(0.0);
  pendingAdjustmentsJson_ = analysisWorkflow_->analysisAdjustmentsJson(
      previewTransactions_, stringList(selectedAdjustmentTxIds_), taxPercent);
  adjustmentAmountsById_ = parseJsonObject(pendingAdjustmentsJson_);
}

void AnalysisState::refreshAnalysisResult() {
  if (!workspace_ || !analysisWorkflow_ || selectedAnalysisId().isEmpty()) {
    return;
  }
  const QVariantMap result = analysisWorkflow_->computeAnalysisPreview(
      selectedAnalysisId(), currentFilterSpec(), includeCalcAdjustments_,
      pendingAdjustmentsJson_);
  if (!result.isEmpty()) {
    workspace_->setLastAnalysisResult(result);
  }
}

void AnalysisState::loadSelectedAnalysis() {
  if (!workspace_ || selectedAnalysisId().isEmpty()) {
    loadCreateState();
    return;
  }

  const QVariantMap row = analysisRowById(selectedAnalysisId());
  if (row.isEmpty()) {
    refreshPreview();
    return;
  }

  name_ = row.value(QStringLiteral("name")).toString();
  const QString type = normalizedType(
      row.value(QStringLiteral("type"), qstr(kPlot)).toString());
  mainTypeIndex_ = type == qstr(kTab) ? 1 : 0;
  activeResultType_ = type.isEmpty() ? qstr(kPlot) : type;

  const QVariantMap config =
      parseJsonObject(row.value(payload::keys::analysis::kConfig).toString());
  plotSubtypeIndex_ =
      config.value(QStringLiteral("plotType")).toString() == qstr(kHistogram)
          ? 1
          : 0;

  const QVariantMap parsed =
      analysisWorkflow_
          ? analysisWorkflow_->parseAnalysisFilterSpec(
                row.value(payload::keys::analysis::kFilter).toString())
          : QVariantMap();
  dateFieldIndex_ =
      parsed.value(QStringLiteral("dateField")).toString() == QStringLiteral("valuta")
          ? 1
          : 0;
  dateModeIndex_ =
      parsed.value(QStringLiteral("dateMode")).toString() == QStringLiteral("year")
          ? 0
          : 1;
  yearValue_ = parsed.value(QStringLiteral("year")).toString();
  dateFromValue_ = parsed.value(QStringLiteral("dateFrom")).toString();
  dateToValue_ = parsed.value(QStringLiteral("dateTo")).toString();
  selectedPropertyIds_ =
      parsed.value(QStringLiteral("propertyIdsNone")).toBool()
          ? QVariantList{qstr(kUnassigned)}
          : (parsed.value(QStringLiteral("propertyIds")).toList().isEmpty()
                 ? allPropertyIds()
                 : parsed.value(QStringLiteral("propertyIds")).toList());
  selectedContractTypes_ =
      parsed.value(QStringLiteral("contractTypesNone")).toBool()
          ? QVariantList{qstr(kUnassigned)}
          : (parsed.value(QStringLiteral("contractTypes")).toList().isEmpty()
                 ? allContractTypes()
                 : parsed.value(QStringLiteral("contractTypes")).toList());
  allocatableMode_ =
      nonEmptyString(parsed, QStringLiteral("allocatableMode"), qstr(kAll));

  pendingAdjustmentsJson_ =
      nonEmptyString(row, payload::keys::analysis::kAdjustments, QStringLiteral("{}"));
  adjustmentAmountsById_ = parseJsonObject(pendingAdjustmentsJson_);
  selectedAdjustmentTxIds_ = adjustmentIds(adjustmentAmountsById_);
  exportFormat_ = normalizedExportFormat(
      row.value(payload::keys::analysis::kExportFormat).toString(), uiType());
  includeCalcAdjustments_ =
      row.value(payload::keys::analysis::kIncludeCalcAdjustments, true).toBool();
  exportStateJson_ = normalizedExportStateJson(
      row.value(payload::keys::analysis::kExportState, QStringLiteral("{}")).toString());
  snapshotTransactionsJson_ = nonEmptyString(
      row, payload::keys::analysis::kSnapshotTransactions, QStringLiteral("{}"));
  filterEditMode_ = false;
  refreshAnalysisResult();
  captureSavedState();
  refreshPreview();
}

void AnalysisState::loadCreateState() {
  name_.clear();
  mainTypeIndex_ = 0;
  activeResultType_ = qstr(kPlot);
  plotSubtypeIndex_ = 0;
  dateFieldIndex_ = 0;
  dateModeIndex_ = defaultAnalysisDateMode() == QStringLiteral("year") ? 0 : 1;
  yearValue_ = defaultAnalysisYear();
  dateFromValue_.clear();
  dateToValue_.clear();
  selectedPropertyIds_ = allPropertyIds();
  selectedContractTypes_ = allContractTypes();
  allocatableMode_ = qstr(kAll);
  filterEditMode_ = true;
  filterWorkspaceIndex_ = 0;
  exportFormat_.clear();
  includeCalcAdjustments_ = true;
  exportStateJson_ = QStringLiteral("{}");
  snapshotTransactionsJson_ = QStringLiteral("{}");
  resetAdjustments();
  captureSavedState();
  refreshPreview();
}

void AnalysisState::captureSavedState() {
  savedState_.clear();
  savedState_.insert(QStringLiteral("name"), name_);
  savedState_.insert(QStringLiteral("mainTypeIndex"), mainTypeIndex_);
  savedState_.insert(QStringLiteral("plotSubtypeIndex"), plotSubtypeIndex_);
  savedState_.insert(QStringLiteral("dateFieldIndex"), dateFieldIndex_);
  savedState_.insert(QStringLiteral("dateModeIndex"), dateModeIndex_);
  savedState_.insert(QStringLiteral("yearValue"), yearValue_);
  savedState_.insert(QStringLiteral("dateFromValue"), dateFromValue_);
  savedState_.insert(QStringLiteral("dateToValue"), dateToValue_);
  savedState_.insert(QStringLiteral("selectedPropertyIds"), selectedPropertyIds_);
  savedState_.insert(QStringLiteral("selectedContractTypes"),
                     selectedContractTypes_);
  savedState_.insert(QStringLiteral("allocatableMode"), allocatableMode_);
  savedState_.insert(QStringLiteral("exportFormat"), exportFormat_);
  savedState_.insert(QStringLiteral("includeCalcAdjustments"),
                     includeCalcAdjustments_);
  savedState_.insert(QStringLiteral("exportStateJson"),
                     normalizedExportStateJson(exportStateJson_));
  savedState_.insert(QStringLiteral("snapshotTransactionsJson"),
                     snapshotTransactionsJson_);
  savedState_.insert(QStringLiteral("pendingAdjustmentsJson"),
                     pendingAdjustmentsJson_);
}

void AnalysisState::updateResultState() {
  renderedPreviewSource_.clear();
  QVariantMap result =
      workspace_ ? workspace_->lastAnalysisResult().toMap() : QVariantMap();
  const QVariantList artifacts = result.value(QStringLiteral("artifacts")).toList();
  if (!artifacts.isEmpty()) {
    const QString rawPath = artifacts.first().toString().trimmed();
    if (rawPath.startsWith(QStringLiteral("file:")) ||
        rawPath.startsWith(QStringLiteral("qrc:")) ||
        rawPath.startsWith(QStringLiteral("http:")) ||
        rawPath.startsWith(QStringLiteral("https:")) ||
        rawPath.startsWith(QStringLiteral("data:"))) {
      renderedPreviewSource_ = rawPath;
    } else if (!rawPath.isEmpty()) {
      renderedPreviewSource_ = QUrl::fromLocalFile(rawPath).toString();
    }
    if (!renderedPreviewSource_.isEmpty() &&
        !renderedPreviewSource_.startsWith(QStringLiteral("data:")) &&
        !renderedPreviewSource_.startsWith(QStringLiteral("qrc:"))) {
      QUrl previewUrl(renderedPreviewSource_);
      previewUrl.setQuery(QStringLiteral("v=%1").arg(++renderedPreviewRevision_));
      renderedPreviewSource_ = previewUrl.toString();
    }
  }
  rebuildTableState();
}

void AnalysisState::rebuildTableState() {
  tableContractTypes_.clear();
  tablePropertyRows_.clear();
  tableGrandTotal_ = 0.0;

  const QVariantMap result =
      workspace_ ? workspace_->lastAnalysisResult().toMap() : QVariantMap();
  QVariantList transactions = result.value(QStringLiteral("transactions")).toList();
  if (transactions.isEmpty()) {
    const QVariantList table = result.value(QStringLiteral("table")).toList();
    for (int i = 0; i < table.size(); ++i) {
      const QVariantList row = table.at(i).toList();
      if (row.size() < 3) {
        continue;
      }
      transactions.push_back(QVariantMap{
          {QStringLiteral("id"), QStringLiteral("table-row-%1").arg(i)},
          {QStringLiteral("date"), row.value(0).toString()},
          {QStringLiteral("name"), row.value(1).toString()},
          {QStringLiteral("amount"), numberFromVariant(row.value(2))},
          {QStringLiteral("contractType"), tr("Unassigned")},
          {QStringLiteral("propertyIds"), QVariantList{tr("Unassigned")}}});
    }
  }

  QMap<QString, QMap<QString, double>> amountsByProperty;
  QMap<QString, double> totalsByProperty;
  QMap<QString, double> totalsByContract;

  for (const QVariant &value : transactions) {
    const QVariantMap tx = value.toMap();
    const QString contractType =
        nonEmptyString(tx, QStringLiteral("contractType"), tr("Unassigned"));
    const QString txId = tx.value(QStringLiteral("id")).toString();
    const double baseAmount = numberFromVariant(tx.value(QStringLiteral("amount")));
    const double adjustedAmount =
        includeCalcAdjustments_ && !txId.isEmpty() &&
                adjustmentAmountsById_.contains(txId)
            ? numberFromVariant(adjustmentAmountsById_.value(txId))
            : baseAmount;
    QVariantList propertyIds = tx.value(QStringLiteral("propertyIds")).toList();
    if (propertyIds.isEmpty()) {
      propertyIds.push_back(tr("Unassigned"));
    }
    const QVariantList propertyNames =
        tx.value(QStringLiteral("propertyNames")).toList();

    totalsByContract[contractType] += adjustedAmount;
    tableGrandTotal_ += adjustedAmount;
    if (!tableContractTypes_.contains(contractType)) {
      tableContractTypes_.push_back(contractType);
    }

    for (int i = 0; i < propertyIds.size(); ++i) {
      QString propertyName =
          i < propertyNames.size() ? propertyNames.at(i).toString() : QString();
      if (propertyName.isEmpty()) {
        propertyName = propertyIds.at(i).toString();
      }
      if (propertyName.isEmpty()) {
        continue;
      }
      amountsByProperty[propertyName][contractType] += adjustedAmount;
      totalsByProperty[propertyName] += adjustedAmount;
    }
  }

  std::sort(tableContractTypes_.begin(), tableContractTypes_.end(),
            [](const QVariant &a, const QVariant &b) {
              return a.toString() < b.toString();
            });

  for (auto it = amountsByProperty.cbegin(); it != amountsByProperty.cend(); ++it) {
    QVariantList amounts;
    for (const QVariant &contractTypeValue : tableContractTypes_) {
      amounts.push_back(it.value().value(contractTypeValue.toString(), 0.0));
    }
    tablePropertyRows_.push_back(QVariantMap{
        {QStringLiteral("propertyName"), it.key()},
        {QStringLiteral("amounts"), amounts},
        {QStringLiteral("total"), totalsByProperty.value(it.key(), 0.0)}});
  }
}

void AnalysisState::emitChanged() { emit changed(); }

void AnalysisState::bindWorkspace(WorkspaceFacade *value) {
  if (workspace_) {
    disconnect(workspace_, nullptr, this, nullptr);
  }
  workspace_ = value;
  if (!workspace_) {
    return;
  }
  connect(workspace_, &WorkspaceFacade::selectedAnalysisIdChanged, this,
          &AnalysisState::refreshFromSelection);
  connect(workspace_, &WorkspaceFacade::dataRevisionChanged, this,
          &AnalysisState::refreshFromSelection);
  connect(workspace_, &WorkspaceFacade::lastAnalysisResultChanged, this, [this]() {
    updateResultState();
    emitChanged();
  });
}

} // namespace ui
