/**
 * @file ui/src/viewmodels/AnalysisViewModel.cpp
 * @brief Implements the QML-facing AnalysisViewModel API.
 */

#include "ui/viewmodels/AnalysisViewModel.h"

#include <algorithm>
#include <cmath>

#include <QDate>
#include <QSet>
#include <QStringList>
#include <QUrl>

#include "ui/observability/Trace.h"
#include "ui/presentation/PayloadKeys.h"
#include "ui/shell/Settings.h"
#include "ui/workflows/AnalysisWorkflow.h"
#include "ui/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceSelection.h"
#include "ui/workspace/WorkspaceSelectors.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {

namespace {

constexpr int kPreviewDebounceMs = 40;
constexpr auto kPlot = "plot";
constexpr auto kTab = "tab";
constexpr auto kXlsx = "xlsx";
constexpr auto kCsv = "csv";
constexpr auto kPng = "png";
constexpr auto kJpg = "jpg";
constexpr auto kTable = "table";
constexpr auto kTabular = "tabular";
constexpr auto kPie = "pie";
constexpr auto kHistogram = "histogram";
constexpr auto kAll = "all";
constexpr auto kAllocatable = "allocatable";
constexpr auto kNonAllocatable = "non-allocatable";
constexpr auto kUnassigned = "unassigned";

QString qstr(const char* value) {
  return QString::fromLatin1(value);
}

QString rowIdAt(const QVariantList& rows, int index,
                const QString& idKey = QStringLiteral("id")) {
  if (index < 0 || index >= rows.size()) {
    return {};
  }
  return rows.at(index).toMap().value(idKey).toString();
}

int indexOfId(const QVariantList& rows, const QString& id,
              const QString& idKey = QStringLiteral("id")) {
  if (id.isEmpty()) {
    return -1;
  }
  for (int i = 0; i < rows.size(); ++i) {
    if (rows.at(i).toMap().value(idKey).toString() == id) {
      return i;
    }
  }
  return -1;
}

QVariantMap rowById(const QVariantList& rows, const QString& id,
                    const QString& idKey = QStringLiteral("id")) {
  const int index = indexOfId(rows, id.trimmed(), idKey);
  return index >= 0 ? rows.at(index).toMap() : QVariantMap{};
}

QString navigatedSelectionId(const QVariantList& rows, const QString& currentId,
                             int delta, int defaultIndex = 0,
                             const QString& idKey = QStringLiteral("id")) {
  const int currentIndex = indexOfId(rows, currentId, idKey);
  if (currentIndex < 0) {
    if (rows.isEmpty()) {
      return {};
    }
    if (delta > 0) {
      return rowIdAt(rows, 0, idKey);
    }
    if (delta < 0) {
      return rowIdAt(rows, rows.size() - 1, idKey);
    }
    return rowIdAt(rows, defaultIndex, idKey);
  }
  if (delta > 0) {
    return currentIndex >= rows.size() - 1
               ? QString()
               : rowIdAt(rows, currentIndex + 1, idKey);
  }
  if (delta < 0) {
    return currentIndex <= 0 ? QString()
                             : rowIdAt(rows, currentIndex - 1, idKey);
  }
  return rowIdAt(rows, currentIndex, idKey);
}

QString deleteNextSelectionId(const QVariantList& rows,
                              const QString& removedId, int defaultIndex = 0,
                              const QString& idKey = QStringLiteral("id")) {
  if (rows.isEmpty()) {
    return {};
  }
  const int removedIndex = indexOfId(rows, removedId, idKey);
  const int nextIndex = removedIndex >= 0 ? removedIndex + 1 : defaultIndex;
  const int wrapped = nextIndex % rows.size();
  return rowIdAt(rows, wrapped < 0 ? wrapped + rows.size() : wrapped, idKey);
}

double numberFromVariant(const QVariant& value) {
  bool ok = false;
  const double out = value.toDouble(&ok);
  return ok && std::isfinite(out) ? out : 0.0;
}

QString amountText(double value) {
  return QString::number(value, 'f', 2);
}

QString amountText(const QVariant& value) {
  return amountText(numberFromVariant(value));
}

QString nonEmptyString(const QVariantMap& map, const QString& key,
                       const QString& defaultValue = {}) {
  const QString value = map.value(key).toString();
  return value.isEmpty() ? defaultValue : value;
}

QVariantList mapKeys(const QVariantMap& value) {
  QVariantList keys;
  keys.reserve(value.size());
  for (auto it = value.cbegin(); it != value.cend(); ++it) {
    if (!it.key().trimmed().isEmpty()) {
      keys.push_back(it.key());
    }
  }
  return keys;
}

QString normalizedType(const QString& value) {
  const QString trimmed = value.trimmed().toLower();
  return trimmed == qstr(kTabular) ? qstr(kTab) : trimmed;
}

QString plotTypeFromSubtypeIndex(int plotSubtypeIndex) {
  return plotSubtypeIndex == 1 ? qstr(kHistogram) : qstr(kPie);
}

bool containsPropertyId(const QVariantList& rows, const QString& id) {
  for (const QVariant& value : rows) {
    if (value.toMap().value(QStringLiteral("id")).toString() == id) {
      return true;
    }
  }
  return false;
}

QString rowKey(const QVariant& value, const QString& idField) {
  if (!idField.isEmpty()) {
    return value.toMap().value(idField).toString();
  }
  const QVariantMap row = value.toMap();
  if (row.contains(QStringLiteral("value"))) {
    return row.value(QStringLiteral("value")).toString();
  }
  return value.toString();
}

bool containsRowValue(const QVariantList& rows, const QString& value) {
  for (const QVariant& row : rows) {
    if (rowKey(row, {}) == value) {
      return true;
    }
  }
  return false;
}

void appendContractTypeRow(QVariantList& rows, const QString& value,
                           const QString& label) {
  if (value.isEmpty() || containsRowValue(rows, value)) {
    return;
  }
  rows.push_back(QVariantMap{{QStringLiteral("value"), value},
                             {QStringLiteral("label"), label}});
}

} // namespace

AnalysisViewModel::AnalysisViewModel(QObject* parent) : QObject(parent) {
  previewDebounce_.setInterval(kPreviewDebounceMs);
  previewDebounce_.setSingleShot(true);
  connect(&previewDebounce_, &QTimer::timeout, this,
          &AnalysisViewModel::refreshPreview);
}

void AnalysisViewModel::setWorkspaceRoles(WorkspaceStore* store,
                                          WorkspaceCommands* commands,
                                          WorkspaceSelection* selection,
                                          WorkspaceSelectors* selectors) {
  if (store_ == store && commands_ == commands && selection_ == selection &&
      selectors_ == selectors) {
    return;
  }
  bindWorkspaceRoles(store, commands, selection, selectors);
  refreshFromSelection();
  emitChanged();
}

void AnalysisViewModel::setAnalysisWorkflow(AnalysisWorkflow* value) {
  if (analysisWorkflow_ == value) {
    return;
  }
  analysisWorkflow_ = value;
  refreshFromSelection();
  emitChanged();
}

void AnalysisViewModel::setSettings(Settings* value) {
  if (settings_ == value) {
    return;
  }
  settings_ = value;
  refreshFromSelection();
  emitChanged();
}

bool AnalysisViewModel::isEdit() const {
  return !selectedAnalysisId().isEmpty();
}

bool AnalysisViewModel::hasRows() const {
  return !analysisRows().isEmpty();
}

void AnalysisViewModel::refreshFromSelection() {
  refreshFilterRows();
  loadSelectedAnalysis();
  updateResultState();
  emitChanged();
}

void AnalysisViewModel::emitChanged() {
  emit changed();
}

void AnalysisViewModel::bindWorkspaceRoles(WorkspaceStore* store,
                                           WorkspaceCommands* commands,
                                           WorkspaceSelection* selection,
                                           WorkspaceSelectors* selectors) {
  if (store_) {
    disconnect(store_, nullptr, this, nullptr);
  }
  if (selection_) {
    disconnect(selection_, nullptr, this, nullptr);
  }
  store_ = store;
  commands_ = commands;
  selection_ = selection;
  selectors_ = selectors;
  if (!store_ || !selection_) {
    return;
  }
  connect(selection_, &WorkspaceSelection::selectedAnalysisIdChanged, this,
          &AnalysisViewModel::refreshFromSelection);
  connect(store_, &WorkspaceStore::dataRevisionChanged, this,
          &AnalysisViewModel::refreshFromSelection);
}

void AnalysisViewModel::submitCreate() {
  if (!analysisWorkflow_ || !commands_ || !selection_ || !canSubmit()) {
    return;
  }
  observability::traceViewModel(
      "AnalysisViewModel::submitCreate", "Analysis create submitted",
      {{observability::context::kName, name_.toStdString()}});
  refreshPreview();
  refreshAdjustmentAmountsFromSelection();
  const QString type = strategyType();
  activeResultType_ = type;
  const QString selectedExportFormat =
      normalizedExportFormat(exportFormat_, uiType());
  snapshotTransactions_ = previewTransactions_;
  const QString newId = commands_->addAnalysis(
      name_, type, currentAnalysisConfig(), currentAnalysisFilter(),
      selectedExportFormat, includeAdjustments_, snapshotTransactions_,
      adjustmentAmountsById_);
  if (newId.isEmpty()) {
    return;
  }
  selection_->selectAnalysis(newId);
  selectedAdjustmentTxIds_ = mapKeys(adjustmentAmountsById_);
  refreshAnalysisResult();
  filterEditMode_ = false;
  emitChanged();
}

void AnalysisViewModel::submitUpdate() {
  if (!analysisWorkflow_ || !commands_ || selectedAnalysisId().isEmpty() ||
      !canSubmit()) {
    return;
  }
  observability::traceViewModel(
      "AnalysisViewModel::submitUpdate", "Analysis update submitted",
      {{observability::context::kId, selectedAnalysisId().toStdString()},
       {observability::context::kName, name_.toStdString()}});
  const QString type = strategyType();
  activeResultType_ = type;
  commands_->updateAnalysis(
      selectedAnalysisId(), name_, type, currentAnalysisConfig(),
      currentAnalysisFilter(), normalizedExportFormat(exportFormat_, uiType()),
      includeAdjustments_, snapshotTransactions_, adjustmentAmountsById_);
  selectedAdjustmentTxIds_ = mapKeys(adjustmentAmountsById_);
  refreshAnalysisResult();
  filterEditMode_ = false;
  emitChanged();
}

void AnalysisViewModel::deleteCurrent() {
  const QString removedId = selectedAnalysisId();
  if (removedId.isEmpty() || !commands_ || !selection_) {
    return;
  }
  observability::traceViewModel(
      "AnalysisViewModel::deleteCurrent", "Analysis delete submitted",
      {{observability::context::kId, removedId.toStdString()}});
  const int currentIndex = indexOfId(analysisRows(), removedId);
  commands_->deleteAnalysis(removedId);
  selection_->selectAnalysis(
      deleteNextSelectionId(analysisRows(), removedId, currentIndex));
  emitChanged();
}

void AnalysisViewModel::navigate(int delta) {
  if (!selection_ || analysisRows().isEmpty()) {
    return;
  }

  const QVariantList rows = analysisRows();
  selection_->selectAnalysis(
      navigatedSelectionId(rows, selectedAnalysisId(), delta));
}

void AnalysisViewModel::selectAnalysis(const QString& id) {
  if (selection_) {
    selection_->selectAnalysis(id);
  }
}

QString AnalysisViewModel::selectedAnalysisId() const {
  return selection_ ? selection_->selectedAnalysisId() : QString();
}

QVariantList AnalysisViewModel::analysisRows() const {
  return selectors_ ? selectors_->analysisRows() : QVariantList();
}

QVariantMap AnalysisViewModel::analysisRowById(const QString& id) const {
  return rowById(analysisRows(), id);
}

QStringList AnalysisViewModel::stringList(const QVariantList& values) const {
  QStringList out;
  out.reserve(values.size());
  for (const QVariant& value : values) {
    const QString string = value.toString();
    if (!string.isEmpty()) {
      out.push_back(string);
    }
  }
  return out;
}

QVariantList AnalysisViewModel::allPropertyIds() const {
  QVariantList out;
  for (const QVariant& value : propertyFilterRows_) {
    const QString id = value.toMap().value(QStringLiteral("id")).toString();
    if (!id.isEmpty()) {
      out.push_back(id);
    }
  }
  return out;
}

QVariantList AnalysisViewModel::allContractTypes() const {
  QVariantList out;
  for (const QVariant& value : contractTypeRows_) {
    const QString type = rowKey(value, {});
    if (!type.isEmpty()) {
      out.push_back(type);
    }
  }
  return out;
}

QVariantList
AnalysisViewModel::pruneSelection(const QVariantList& values,
                                  const QVariantList& availableRows,
                                  const QString& idField) const {
  QSet<QString> available;
  for (const QVariant& value : availableRows) {
    const QString key = rowKey(value, idField);
    if (!key.isEmpty()) {
      available.insert(key);
    }
  }

  QVariantList out;
  for (const QVariant& value : values) {
    const QString key = value.toString();
    if (!key.isEmpty() && available.contains(key)) {
      out.push_back(key);
    }
  }
  return out;
}

bool AnalysisViewModel::isAllSelected(const QVariantList& selectedIds,
                                      const QVariantList& availableRows,
                                      const QString& idField) const {
  if (availableRows.isEmpty()) {
    return true;
  }
  QSet<QString> selected;
  for (const QVariant& value : selectedIds) {
    const QString key = value.toString();
    if (!key.isEmpty()) {
      selected.insert(key);
    }
  }
  if (selected.size() != availableRows.size()) {
    return false;
  }
  for (const QVariant& value : availableRows) {
    const QString key = rowKey(value, idField);
    if (key.isEmpty() || !selected.contains(key)) {
      return false;
    }
  }
  return true;
}

void AnalysisViewModel::refreshFilterRows() {
  const bool propertyWasAll = isAllSelected(
      selectedPropertyIds_, propertyFilterRows_, QStringLiteral("id"));
  const bool contractTypesWereAll =
      isAllSelected(selectedContractTypes_, contractTypeRows_);

  propertyFilterRows_ =
      selectors_ ? selectors_->propertyRows() : QVariantList();
  if (!containsPropertyId(propertyFilterRows_, qstr(kUnassigned))) {
    propertyFilterRows_.push_back(QVariantMap{
        {QStringLiteral("id"), qstr(kUnassigned)},
        {QStringLiteral("name"), tr("Unassigned")},
    });
  }
  selectedPropertyIds_ =
      propertyWasAll ? allPropertyIds()
                     : pruneSelection(selectedPropertyIds_, propertyFilterRows_,
                                      QStringLiteral("id"));

  QVariantList types;
  if (analysisWorkflow_) {
    for (const QString& type : analysisWorkflow_->contractTypes()) {
      appendContractTypeRow(types, type, type);
    }
  }
  appendContractTypeRow(types, qstr(kUnassigned), tr("Unassigned"));
  contractTypeRows_ = types;
  selectedContractTypes_ =
      contractTypesWereAll
          ? allContractTypes()
          : pruneSelection(selectedContractTypes_, contractTypeRows_);
}

QString AnalysisViewModel::currentDateField() const {
  return dateFieldIndex_ == 1 ? QStringLiteral("valuta")
                              : QStringLiteral("bookingDate");
}

QString AnalysisViewModel::currentDateMode() const {
  return dateModeIndex_ == 0 ? QStringLiteral("year") : QStringLiteral("range");
}

QString AnalysisViewModel::defaultAnalysisDateMode() const {
  if (settings_) {
    const QString mode = settings_->analysisDefaultDateMode().toLower();
    return mode == QStringLiteral("range") ? QStringLiteral("range")
                                           : QStringLiteral("year");
  }
  return QStringLiteral("year");
}

QString AnalysisViewModel::defaultAnalysisYear() const {
  if (settings_ && settings_->analysisDefaultYear() > 0) {
    return QString::number(settings_->analysisDefaultYear());
  }
  return QString::number(QDate::currentDate().year() - 1);
}

QStringList AnalysisViewModel::effectiveSelectedPropertyIds() const {
  if (propertyFilterRows_.isEmpty() ||
      isAllSelected(selectedPropertyIds_, propertyFilterRows_,
                    QStringLiteral("id"))) {
    return {};
  }
  return selectedPropertyIds_.isEmpty() ? QStringList{}
                                        : stringList(selectedPropertyIds_);
}

QStringList AnalysisViewModel::effectiveSelectedContractTypes() const {
  if (contractTypeRows_.isEmpty() ||
      isAllSelected(selectedContractTypes_, contractTypeRows_)) {
    return {};
  }
  return selectedContractTypes_.isEmpty() ? QStringList{}
                                          : stringList(selectedContractTypes_);
}

QString AnalysisViewModel::currentFilterSpec() const {
  if (!analysisWorkflow_) {
    return {};
  }
  const QString mode = allocatableMode_.toLower();
  return analysisWorkflow_->analysisFilterSpec(
      currentDateField(), currentDateMode(), yearValue_, dateFromValue_,
      dateToValue_, effectiveSelectedPropertyIds(),
      effectiveSelectedContractTypes(),
      mode == qstr(kAllocatable) || mode == qstr(kNonAllocatable) ? mode
                                                                  : QString());
}

QVariantMap AnalysisViewModel::currentAnalysisConfig() const {
  return QVariantMap{
      {QStringLiteral("type"), strategyType()},
      {QStringLiteral("plotType"), plotTypeFromSubtypeIndex(plotSubtypeIndex_)},
      {QStringLiteral("plotMeasure"), QStringLiteral("totalAmount")},
      {QStringLiteral("propertyIds"), effectiveSelectedPropertyIds()},
      {QStringLiteral("contractTypes"), effectiveSelectedContractTypes()},
      {QStringLiteral("taxPercent"), 0.0},
  };
}

QVariantMap AnalysisViewModel::currentAnalysisFilter() const {
  const QString mode = allocatableMode_.toLower();
  return QVariantMap{
      {QStringLiteral("dateField"), currentDateField()},
      {QStringLiteral("dateMode"), currentDateMode()},
      {QStringLiteral("year"), yearValue_},
      {QStringLiteral("dateFrom"), dateFromValue_},
      {QStringLiteral("dateTo"), dateToValue_},
      {QStringLiteral("propertyIds"), effectiveSelectedPropertyIds()},
      {QStringLiteral("propertyIdsNone"),
       selectedPropertyIds_.contains(qstr(kUnassigned))},
      {QStringLiteral("contractTypes"), effectiveSelectedContractTypes()},
      {QStringLiteral("contractTypesNone"),
       selectedContractTypes_.contains(qstr(kUnassigned))},
      {QStringLiteral("allocatableMode"),
       mode == qstr(kAllocatable) || mode == qstr(kNonAllocatable) ? mode
                                                                   : QString()},
  };
}

void AnalysisViewModel::resetAdjustments() {
  selectedAdjustmentTxIds_.clear();
  adjustmentAmountsById_.clear();
  adjustmentName_.clear();
  adjustmentPercentText_.clear();
}

void AnalysisViewModel::setFilterEditMode(bool value) {
  if (filterEditMode_ == value) {
    return;
  }
  filterEditMode_ = value;
  emitChanged();
}

void AnalysisViewModel::setDateFieldIndex(int value) {
  const int next = value == 1 ? 1 : 0;
  if (dateFieldIndex_ == next) {
    return;
  }
  dateFieldIndex_ = next;
  requestPreviewRefresh();
  emitChanged();
}

void AnalysisViewModel::setDateModeIndex(int value) {
  const int next = value == 1 ? 1 : 0;
  if (dateModeIndex_ == next) {
    return;
  }
  dateModeIndex_ = next;
  requestPreviewRefresh();
  emitChanged();
}

void AnalysisViewModel::setYearValue(const QString& value) {
  if (yearValue_ == value) {
    return;
  }
  yearValue_ = value;
  requestPreviewRefresh();
  emitChanged();
}

void AnalysisViewModel::setDateFromValue(const QString& value) {
  if (dateFromValue_ == value) {
    return;
  }
  dateFromValue_ = value;
  requestPreviewRefresh();
  emitChanged();
}

void AnalysisViewModel::setDateToValue(const QString& value) {
  if (dateToValue_ == value) {
    return;
  }
  dateToValue_ = value;
  requestPreviewRefresh();
  emitChanged();
}

void AnalysisViewModel::setSelectedPropertyIds(const QVariantList& value) {
  if (selectedPropertyIds_ == value) {
    return;
  }
  selectedPropertyIds_ = value;
  requestPreviewRefresh();
  emitChanged();
}

void AnalysisViewModel::setSelectedContractTypes(const QVariantList& value) {
  if (selectedContractTypes_ == value) {
    return;
  }
  selectedContractTypes_ = value;
  requestPreviewRefresh();
  emitChanged();
}

void AnalysisViewModel::setAllocatableMode(const QString& value) {
  const QString lower = value.trimmed().toLower();
  const QString next =
      lower == qstr(kAllocatable) || lower == qstr(kNonAllocatable)
          ? lower
          : qstr(kAll);
  if (allocatableMode_ == next) {
    return;
  }
  allocatableMode_ = next;
  requestPreviewRefresh();
  emitChanged();
}

void AnalysisViewModel::setPropertySelected(const QString& id, bool selected) {
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

void AnalysisViewModel::selectAllProperties() {
  setSelectedPropertyIds(allPropertyIds());
}

void AnalysisViewModel::selectNoProperties() {
  setSelectedPropertyIds({});
}

void AnalysisViewModel::setContractTypeSelected(const QString& type,
                                                bool selected) {
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

void AnalysisViewModel::selectAllContractTypes() {
  setSelectedContractTypes(allContractTypes());
}

void AnalysisViewModel::selectNoContractTypes() {
  setSelectedContractTypes({});
}

void AnalysisViewModel::setAdjustmentTransactionSelected(const QString& id,
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

void AnalysisViewModel::setAllocatableModeIndex(int index) {
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

void AnalysisViewModel::toggleFilterContent() {
  filterContentIndex_ = filterContentIndex_ == 0 ? 1 : 0;
  emitChanged();
}

void AnalysisViewModel::clearFilters() {
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

void AnalysisViewModel::setName(const QString& value) {
  if (name_ == value) {
    return;
  }
  name_ = value;
  emitChanged();
}

void AnalysisViewModel::setMainTypeIndex(int value) {
  const int next = value == 1 ? 1 : 0;
  if (mainTypeIndex_ == next) {
    return;
  }
  mainTypeIndex_ = next;
  activeResultType_ = strategyType();
  exportFormat_ = normalizedExportFormat(exportFormat_, uiType());
  emitChanged();
}

void AnalysisViewModel::setPlotSubtypeIndex(int value) {
  const int next = value == 1 ? 1 : 0;
  if (plotSubtypeIndex_ == next) {
    return;
  }
  plotSubtypeIndex_ = next;
  emitChanged();
}

QVariantList AnalysisViewModel::plotTypeOptions() const {
  return {QVariantMap{{QStringLiteral("value"), qstr(kPie)},
                      {QStringLiteral("label"), tr("Pie chart")}},
          QVariantMap{{QStringLiteral("value"), qstr(kHistogram)},
                      {QStringLiteral("label"), tr("Histogram")}}};
}

QVariantList AnalysisViewModel::exportFormatOptions() const {
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

void AnalysisViewModel::setExportFormat(const QString& value) {
  const QString next = normalizedExportFormat(value, uiType());
  if (exportFormat_ == next) {
    return;
  }
  exportFormat_ = next;
  emitChanged();
}

int AnalysisViewModel::exportFormatIndex() const {
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

void AnalysisViewModel::setExportFormatIndex(int value) {
  const QVariantList options = exportFormatOptions();
  if (options.isEmpty()) {
    return;
  }
  const int index = std::clamp(value, 0, static_cast<int>(options.size()) - 1);
  setExportFormat(
      options.at(index).toMap().value(QStringLiteral("value")).toString());
}

void AnalysisViewModel::setIncludeAdjustments(bool value) {
  if (includeAdjustments_ == value) {
    return;
  }
  includeAdjustments_ = value;
  refreshAnalysisResult();
  updateResultState();
  emitChanged();
}

void AnalysisViewModel::setSelectedAdjustmentTxIds(const QVariantList& value) {
  if (selectedAdjustmentTxIds_ == value) {
    return;
  }
  selectedAdjustmentTxIds_ = value;
  emitChanged();
}

void AnalysisViewModel::setAdjustmentName(const QString& value) {
  if (adjustmentName_ == value) {
    return;
  }
  adjustmentName_ = value;
  emitChanged();
}

void AnalysisViewModel::setAdjustmentPercentText(const QString& value) {
  if (adjustmentPercentText_ == value) {
    return;
  }
  adjustmentPercentText_ = value;
  emitChanged();
}

bool AnalysisViewModel::canSubmit() const {
  return !name_.trimmed().isEmpty() && !selectedPropertyIds_.isEmpty() &&
         !selectedContractTypes_.isEmpty();
}

QString AnalysisViewModel::uiType() const {
  return mainTypeIndex_ == 1 ? qstr(kTable) : qstr(kPlot);
}

QString AnalysisViewModel::strategyType() const {
  return uiType() == qstr(kTable) ? qstr(kTab) : qstr(kPlot);
}

QString
AnalysisViewModel::normalizedExportFormat(const QString& value,
                                          const QString& targetUiType) const {
  const QString normalized = value.trimmed().toLower();
  const QStringList allowed = targetUiType == qstr(kTable)
                                  ? QStringList{qstr(kXlsx), qstr(kCsv)}
                                  : QStringList{qstr(kPng), qstr(kJpg)};
  return allowed.contains(normalized) ? normalized : allowed.first();
}

void AnalysisViewModel::loadSelectedAnalysis() {
  if (!selectors_ || selectedAnalysisId().isEmpty()) {
    loadCreateState();
    return;
  }

  const QVariantMap row = analysisRowById(selectedAnalysisId());
  if (row.isEmpty()) {
    refreshPreview();
    return;
  }

  name_ = row.value(QStringLiteral("name")).toString();
  const QString type =
      normalizedType(row.value(QStringLiteral("type"), qstr(kPlot)).toString());
  mainTypeIndex_ = type == qstr(kTab) ? 1 : 0;
  activeResultType_ = type.isEmpty() ? qstr(kPlot) : type;

  const QVariantMap config =
      row.value(payload::keys::analysis::kConfig).toMap();
  plotSubtypeIndex_ =
      config.value(QStringLiteral("plotType")).toString() == qstr(kHistogram)
          ? 1
          : 0;

  const QVariantMap parsed =
      row.value(payload::keys::analysis::kFilter).toMap();
  dateFieldIndex_ = parsed.value(QStringLiteral("dateField")).toString() ==
                            QStringLiteral("valuta")
                        ? 1
                        : 0;
  dateModeIndex_ = parsed.value(QStringLiteral("dateMode")).toString() ==
                           QStringLiteral("year")
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

  adjustmentAmountsById_ =
      row.value(payload::keys::analysis::kAdjustments).toMap();
  selectedAdjustmentTxIds_ = mapKeys(adjustmentAmountsById_);
  exportFormat_ = normalizedExportFormat(
      row.value(payload::keys::analysis::kExportFormat).toString(), uiType());
  includeAdjustments_ =
      row.value(payload::keys::analysis::kIncludeCalcAdjustments, true)
          .toBool();
  snapshotTransactions_ =
      row.value(payload::keys::analysis::kSnapshotTransactions).toList();
  filterEditMode_ = false;
  refreshAnalysisResult();
  refreshPreview();
}

void AnalysisViewModel::loadCreateState() {
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
  filterContentIndex_ = 0;
  exportFormat_.clear();
  includeAdjustments_ = true;
  snapshotTransactions_.clear();
  currentAnalysisResult_.clear();
  resetAdjustments();
  refreshPreview();
}

QVariantList AnalysisViewModel::previewTransactionRows() const {
  QVariantList rows;
  rows.reserve(previewTransactions_.size());
  const QString calculationLabel = adjustmentName_.trimmed().isEmpty()
                                       ? tr("adjustment")
                                       : adjustmentName_.trimmed();

  for (const QVariant& value : previewTransactions_) {
    const QVariantMap source = value.toMap();
    const QString txId = source.value(QStringLiteral("id")).toString();
    const double baseAmount =
        numberFromVariant(source.value(QStringLiteral("amount")));
    const bool hasAdjusted = includeAdjustments_ && !txId.isEmpty() &&
                             adjustmentAmountsById_.contains(txId);
    const double adjustedAmount =
        hasAdjusted ? numberFromVariant(adjustmentAmountsById_.value(txId))
                    : baseAmount;
    QString renderedAmount = amountText(baseAmount);
    if (hasAdjusted) {
      renderedAmount =
          QStringLiteral("%1 (%2 %3 %4)")
              .arg(amountText(baseAmount), amountText(adjustedAmount),
                   tr("plus"), calculationLabel);
    }

    rows.push_back(QVariantMap{
        {QStringLiteral("id"), txId},
        {QStringLiteral("statementName"),
         source.value(QStringLiteral("statementName")).toString()},
        {QStringLiteral("transactionName"),
         source.value(QStringLiteral("transactionName")).toString()},
        {QStringLiteral("date"),
         source.value(QStringLiteral("date")).toString()},
        {QStringLiteral("valuta"),
         source.value(QStringLiteral("valuta")).toString()},
        {QStringLiteral("actorName"),
         source.value(QStringLiteral("actorName")).toString()},
        {QStringLiteral("contractName"),
         source.value(QStringLiteral("contractName")).toString()},
        {QStringLiteral("contractType"),
         nonEmptyString(source, QStringLiteral("contractType"),
                        tr("Unassigned"))},
        {QStringLiteral("propertiesLabel"),
         source.value(QStringLiteral("propertiesLabel")).toString()},
        {QStringLiteral("amountText"), renderedAmount}});
  }
  return rows;
}

QString AnalysisViewModel::previewStatementCountText() const {
  const int count =
      previewMetrics_.value(QStringLiteral("statementCount")).toInt();
  return tr("Statements: %1").arg(count);
}

QString AnalysisViewModel::previewTransactionCountText() const {
  const int count =
      previewMetrics_.value(QStringLiteral("transactionCount")).toInt();
  return tr("Transactions: %1").arg(count);
}

QString AnalysisViewModel::previewAmountSumText() const {
  const double amount =
      numberFromVariant(previewMetrics_.value(QStringLiteral("amountSum")));
  return tr("Amount sum: %1").arg(amountText(amount));
}

void AnalysisViewModel::refreshPreview() {
  if (!analysisWorkflow_) {
    return;
  }
  const QVariantMap preview =
      analysisWorkflow_->previewTransactions(currentFilterSpec());
  previewTransactions_ = preview.value(QStringLiteral("transactions")).toList();
  previewMetrics_ = preview.value(QStringLiteral("metrics")).toMap();
  emitChanged();
}

void AnalysisViewModel::requestPreviewRefresh() {
  previewDebounce_.start();
}

QString AnalysisViewModel::currentResultType() const {
  const QVariantMap row = analysisRowById(selectedAnalysisId());
  const QString type = normalizedType(
      nonEmptyString(row, QStringLiteral("type"), activeResultType_));
  return type.isEmpty() ? activeResultType_ : type;
}

bool AnalysisViewModel::currentResultIsTable() const {
  return currentResultType() == qstr(kTab);
}

QString AnalysisViewModel::renderedPreviewSourceFromResult(
    const QVariantMap& analysisResult) const {
  const QVariantList artifacts =
      analysisResult.value(QStringLiteral("artifacts")).toList();
  if (artifacts.isEmpty()) {
    return {};
  }

  QString renderedSource;
  const QString rawPath = artifacts.first().toString().trimmed();
  if (rawPath.startsWith(QStringLiteral("file:")) ||
      rawPath.startsWith(QStringLiteral("qrc:")) ||
      rawPath.startsWith(QStringLiteral("http:")) ||
      rawPath.startsWith(QStringLiteral("https:")) ||
      rawPath.startsWith(QStringLiteral("data:"))) {
    renderedSource = rawPath;
  } else if (!rawPath.isEmpty()) {
    renderedSource = QUrl::fromLocalFile(rawPath).toString();
  }
  if (renderedSource.isEmpty() ||
      renderedSource.startsWith(QStringLiteral("data:")) ||
      renderedSource.startsWith(QStringLiteral("qrc:"))) {
    return renderedSource;
  }

  QUrl previewUrl(renderedSource);
  previewUrl.setQuery(QStringLiteral("v=%1").arg(renderedPreviewRevision_));
  return previewUrl.toString();
}

void AnalysisViewModel::applySelectedAdjustment() {
  refreshAdjustmentAmountsFromSelection();
  refreshAnalysisResult();
  updateResultState();
  emitChanged();
}

void AnalysisViewModel::refreshAdjustmentAmountsFromSelection() {
  if (!analysisWorkflow_ || selectedAdjustmentTxIds_.isEmpty()) {
    return;
  }
  adjustmentAmountsById_ =
      analysisWorkflow_->analysisAdjustmentAmountsFromPercentText(
          previewTransactions_, stringList(selectedAdjustmentTxIds_),
          adjustmentPercentText_);
}

void AnalysisViewModel::refreshAnalysisResult() {
  if (!selectors_ || !analysisWorkflow_ || selectedAnalysisId().isEmpty()) {
    return;
  }
  const QVariantMap result = analysisWorkflow_->computeAnalysisPreview(
      selectedAnalysisId(), currentFilterSpec(), includeAdjustments_,
      adjustmentAmountsById_);
  currentAnalysisResult_ = result;
}

void AnalysisViewModel::updateResultState() {
  renderedPreviewSource_.clear();
  tableContractTypes_.clear();
  tablePropertyRows_.clear();
  tableGrandTotal_ = 0.0;

  const QVariantMap result = currentAnalysisResult_;
  if (!analysisWorkflow_ || result.isEmpty()) {
    return;
  }

  ++renderedPreviewRevision_;
  renderedPreviewSource_ = renderedPreviewSourceFromResult(result);
  const QVariantMap table = result.value(QStringLiteral("tableState")).toMap();
  tableContractTypes_ = table.value(QStringLiteral("contractTypes")).toList();
  tablePropertyRows_ = table.value(QStringLiteral("propertyRows")).toList();
  tableGrandTotal_ =
      numberFromVariant(table.value(QStringLiteral("grandTotal")));
}

} // namespace ui
