/**
 * @file ui/src/viewmodels/AnnualViewModel.cpp
 * @brief Implements the QML-facing AnnualViewModel API.
 */

#include "ui/viewmodels/AnnualViewModel.h"

#include <algorithm>
#include <cmath>

#include <QDate>
#include <QMetaType>
#include <QSet>

#include "ui/i18n/Text.h"
#include "ui/observability/Trace.h"
#include "ui/presentation/PayloadKeys.h"
#include "ui/workflows/AnnualWorkflow.h"
#include "ui/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceSelection.h"
#include "ui/workspace/WorkspaceSelectors.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {

namespace {

constexpr int kYearMin = 1900;
constexpr int kYearMax = 2500;
constexpr auto kPlot = "plot";
constexpr auto kTab = "tab";
constexpr auto kXlsx = "xlsx";
constexpr auto kCsv = "csv";
constexpr auto kPng = "png";
constexpr auto kJpg = "jpg";
constexpr auto kDeduplicated = "deduplicated";
constexpr auto kSimilar = "similar";
constexpr auto kDivergent = "divergent";
constexpr auto kWorkspaceOnly = "workspaceOnly";
constexpr auto kMissingLive = "missingLive";

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
} // namespace

AnnualViewModel::AnnualViewModel(QObject* parent) : QObject(parent) {
  sectionExpanded_.insert(qstr(kDeduplicated), true);
  sectionExpanded_.insert(qstr(kSimilar), true);
  sectionExpanded_.insert(qstr(kDivergent), true);
  sectionExpanded_.insert(qstr(kWorkspaceOnly), true);
  sectionExpanded_.insert(qstr(kMissingLive), true);
  verificationIssues_ = emptyIssues();
  statusMetrics_ = emptyStatusMetrics();
  year_ = defaultYear();
}

void AnnualViewModel::setWorkspaceRoles(WorkspaceStore* store,
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

void AnnualViewModel::setAnnualWorkflow(AnnualWorkflow* value) {
  if (annualWorkflow_ == value) {
    return;
  }
  annualWorkflow_ = value;
  lastPreviewRevision_ = -1;
  refreshFromSelection();
  emitChanged();
}

bool AnnualViewModel::isEdit() const {
  return !selectedAnnualId().isEmpty();
}

QVariantList AnnualViewModel::annualRows() const {
  return selectors_ ? selectors_->annualRows() : QVariantList();
}

QString AnnualViewModel::selectedAnnualId() const {
  return selection_ ? selection_->selectedAnnualId() : QString();
}

void AnnualViewModel::setName(const QString& value) {
  if (name_ == value) {
    return;
  }
  name_ = value;
  emitChanged();
}

void AnnualViewModel::setYear(int value) {
  const int next = std::clamp(value, kYearMin, kYearMax);
  if (year_ == next) {
    return;
  }
  year_ = next;
  rebuildAnnualResultState();
  emitChanged();
}

bool AnnualViewModel::hasRows() const {
  return !annualRows().isEmpty();
}

bool AnnualViewModel::hasChanges() const {
  if (!isEdit()) {
    return canSubmit();
  }
  QStringList current = analysisIds_;
  QStringList saved = savedAnalysisIds_;
  current.sort();
  saved.sort();
  return savedName_ != name_ || savedYear_ != year_ || saved != current ||
         analysisMetadataDirty_;
}

void AnnualViewModel::refreshFromSelection() {
  loadSelectedAnnual();
  rebuildAnnualResultState();
  emitChanged();
}

void AnnualViewModel::resetCreateState() {
  if (isEdit() && selection_) {
    selection_->selectAnnual({});
  }
  loadCreateState();
  rebuildAnnualResultState();
  emitChanged();
}

void AnnualViewModel::toggleContent() {
  contentIndex_ = contentIndex_ == 0 ? 1 : 0;
  emitChanged();
}

void AnnualViewModel::stepYear(int delta) {
  setYear(year_ + delta);
}

int AnnualViewModel::workspaceRevision() const {
  return selectors_ ? selectors_->dataRevision() : -1;
}

void AnnualViewModel::bindWorkspaceRoles(WorkspaceStore* store,
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
  connect(selection_, &WorkspaceSelection::selectedAnnualIdChanged, this,
          &AnnualViewModel::refreshFromSelection);
  connect(store_, &WorkspaceStore::dataRevisionChanged, this,
          &AnnualViewModel::refreshFromSelection);
}

void AnnualViewModel::emitChanged() {
  emit changed();
}

QStringList
AnnualViewModel::normalizedAnalysisIds(const QVariant& values) const {
  QStringList out;
  QSet<QString> seen;
  const QVariantList list = values.toList();
  if (!list.isEmpty()) {
    for (const QVariant& value : list) {
      const QString id = analysisIdFromVariant(value);
      if (id.isEmpty() || seen.contains(id)) {
        continue;
      }
      seen.insert(id);
      out.push_back(id);
    }
    return out;
  }
  const QString single = analysisIdFromVariant(values);
  if (!single.isEmpty()) {
    out.push_back(single);
  }
  return out;
}

QString AnnualViewModel::analysisIdFromVariant(const QVariant& value) const {
  if (!value.isValid()) {
    return {};
  }
  if (value.typeId() == QMetaType::QString) {
    return value.toString().trimmed();
  }
  const QVariantMap map = value.toMap();
  if (!map.isEmpty()) {
    const QString id = map.value(QStringLiteral("id")).toString().trimmed();
    return id.isEmpty()
               ? map.value(QStringLiteral("objectId")).toString().trimmed()
               : id;
  }
  return value.toString().trimmed();
}

QVariantMap AnnualViewModel::analysisRowById(const QString& id) const {
  return rowById(normalizedAnalysisRows(), id);
}

QVariantMap AnnualViewModel::normalizeAnalysisRow(const QVariant& value) const {
  const QVariantMap source = value.toMap();
  const QString id = analysisIdFromVariant(value);
  if (id.isEmpty()) {
    return {};
  }
  const QString name = source.value(QStringLiteral("name")).toString();
  const QString display = source.value(QStringLiteral("display")).toString();
  const QString type =
      nonEmptyString(source, QStringLiteral("type"), qstr(kTab));
  QVariantMap out;
  out.insert(QStringLiteral("id"), id);
  out.insert(QStringLiteral("name"), name);
  out.insert(QStringLiteral("display"),
             !display.isEmpty() ? display : (!name.isEmpty() ? name : id));
  out.insert(QStringLiteral("type"), type);
  out.insert(QStringLiteral("config"),
             source.value(QStringLiteral("config"), QStringLiteral("{}")));
  out.insert(
      QStringLiteral("filter"),
      source.value(QStringLiteral("filter"),
                   source.value(QStringLiteral("filterSpec"), QString())));
  out.insert(QStringLiteral("exportFormat"),
             source.value(QStringLiteral("exportFormat"), QString()));
  out.insert(
      payload::keys::analysis::kIncludeCalcAdjustments,
      source.value(payload::keys::analysis::kIncludeCalcAdjustments, true));
  out.insert(QStringLiteral("snapshotTransactions"),
             source.value(QStringLiteral("snapshotTransactions"),
                          QStringLiteral("[]")));
  return augmentAnalysisRow(out);
}

QVariantList AnnualViewModel::normalizedAnalysisRows() const {
  QVariantList rows;
  if (!selectors_) {
    return rows;
  }
  const QVariantList source = selectors_->analysisRows();
  rows.reserve(source.size());
  for (const QVariant& value : source) {
    const QVariantMap row = normalizeAnalysisRow(value);
    if (!row.isEmpty()) {
      rows.push_back(row);
    }
  }
  return rows;
}

QVariantList AnnualViewModel::buildAssignedAnalysisRows() const {
  const QVariantList rows = normalizedAnalysisRows();
  QVariantList out;
  out.reserve(analysisIds_.size());
  for (const QString& id : analysisIds_) {
    const QVariantMap row = rowById(rows, id);
    if (!row.isEmpty()) {
      out.push_back(row);
    }
  }
  return out;
}

QVariantList AnnualViewModel::buildAvailableAnalysisRows() const {
  const QVariantList rows = normalizedAnalysisRows();
  const QSet<QString> selected(analysisIds_.begin(), analysisIds_.end());
  QVariantList out;
  for (const QVariant& value : rows) {
    const QString id = value.toMap().value(QStringLiteral("id")).toString();
    if (!id.isEmpty() && !selected.contains(id)) {
      out.push_back(value);
    }
  }
  return out;
}

QVariantMap AnnualViewModel::augmentAnalysisRow(const QVariantMap& row) const {
  QVariantMap out = row;
  const QString type = row.value(QStringLiteral("type")).toString().toLower();
  const QVariantList options = exportOptionsForType(type);
  out.insert(QStringLiteral("typeLabel"),
             type == qstr(kPlot) ? tr("Plot") : tr("Table"));
  out.insert(QStringLiteral("exportFormatOptions"), options);
  out.insert(
      QStringLiteral("exportFormatIndex"),
      exportFormatIndex(options,
                        row.value(QStringLiteral("exportFormat")).toString()));
  return out;
}

QVariantList AnnualViewModel::exportOptionsForType(const QString& type) const {
  if (type.trimmed().toLower() == qstr(kPlot)) {
    return {QStringLiteral("PNG"), QStringLiteral("JPG")};
  }
  return {QStringLiteral("XLSX"), QStringLiteral("CSV")};
}

int AnnualViewModel::exportFormatIndex(const QVariantList& options,
                                       const QString& exportFormat) const {
  const QString normalized = exportFormat.trimmed().toUpper();
  for (int i = 0; i < options.size(); ++i) {
    if (options.at(i).toString().toUpper() == normalized) {
      return i;
    }
  }
  return 0;
}

QString AnnualViewModel::normalizedExportFormat(const QString& value,
                                                const QString& type) const {
  const QString normalized = value.trimmed().toLower();
  const QString normalizedType = type.trimmed().toLower();
  const QStringList allowed = normalizedType == qstr(kPlot)
                                  ? QStringList{qstr(kPng), qstr(kJpg)}
                                  : QStringList{qstr(kXlsx), qstr(kCsv)};
  return allowed.contains(normalized) ? normalized : allowed.first();
}

QVariantList AnnualViewModel::analysisIds() const {
  QVariantList out;
  out.reserve(analysisIds_.size());
  for (const QString& id : analysisIds_) {
    out.push_back(id);
  }
  return out;
}

void AnnualViewModel::addAvailableAnalysisAtIndex(int index) {
  if (index < 0 || index >= availableAnalysisRows_.size()) {
    return;
  }
  const QString id = availableAnalysisRows_.at(index)
                         .toMap()
                         .value(QStringLiteral("id"))
                         .toString();
  if (id.isEmpty() || analysisIds_.contains(id)) {
    return;
  }
  QStringList next = analysisIds_;
  next.push_back(id);
  setAnalysisIds(next);
}

void AnnualViewModel::removeAnalysis(const QString& id) {
  const QString trimmed = id.trimmed();
  if (trimmed.isEmpty() || !analysisIds_.contains(trimmed)) {
    return;
  }
  QStringList next = analysisIds_;
  next.removeAll(trimmed);
  setAnalysisIds(next);
}

void AnnualViewModel::setAnalysisExportFormat(const QString& id,
                                              const QString& exportFormat) {
  if (!commands_) {
    return;
  }
  const QVariantMap row = analysisRowById(id.trimmed());
  if (row.isEmpty()) {
    return;
  }
  const QString type = nonEmptyString(row, QStringLiteral("type"), qstr(kTab));
  const QString nextFormat = normalizedExportFormat(exportFormat, type);
  commands_->updateAnalysisExportFormat(id.trimmed(), nextFormat);
  analysisMetadataDirty_ = true;
  refreshAnalysisSelectionRows();
  rebuildAnnualResultState();
  emitChanged();
}

void AnnualViewModel::refreshAnalysisSelectionRows() {
  assignedAnalysisRows_ = buildAssignedAnalysisRows();
  availableAnalysisRows_ = buildAvailableAnalysisRows();
}

void AnnualViewModel::setAnalysisIds(const QStringList& ids) {
  QStringList next;
  QSet<QString> seen;
  for (const QString& id : ids) {
    const QString trimmed = id.trimmed();
    if (trimmed.isEmpty() || seen.contains(trimmed)) {
      continue;
    }
    seen.insert(trimmed);
    next.push_back(trimmed);
  }
  if (analysisIds_ == next) {
    return;
  }
  analysisIds_ = next;
  refreshAnalysisSelectionRows();
  rebuildAnnualResultState();
  emitChanged();
}

void AnnualViewModel::submitCreate() {
  if (!commands_ || !selection_ || !canSubmit()) {
    return;
  }
  observability::traceViewModel(
      "AnnualViewModel::submitCreate", "Annual create submitted",
      {{observability::context::kName, name_.toStdString()}});
  const QString id = commands_->addAnnual(name_, year_, analysisIds_);
  if (id.isEmpty()) {
    return;
  }
  selection_->selectAnnual(id);
  captureSavedState();
  emitChanged();
}

void AnnualViewModel::submitUpdate() {
  if (!commands_ || selectedAnnualId().isEmpty() || !canSubmit()) {
    return;
  }
  observability::traceViewModel(
      "AnnualViewModel::submitUpdate", "Annual update submitted",
      {{observability::context::kId, selectedAnnualId().toStdString()},
       {observability::context::kName, name_.toStdString()}});
  commands_->updateAnnual(selectedAnnualId(), name_, year_, analysisIds_);
  captureSavedState();
  emitChanged();
}

void AnnualViewModel::deleteCurrent() {
  if (!commands_ || !selection_ || selectedAnnualId().isEmpty()) {
    return;
  }
  const QString removedId = selectedAnnualId();
  observability::traceViewModel(
      "AnnualViewModel::deleteCurrent", "Annual delete submitted",
      {{observability::context::kId, removedId.toStdString()}});
  commands_->deleteAnnual(removedId);

  const QString nextId = deleteNextSelectionId(annualRows(), removedId);
  selection_->selectAnnual(nextId);
  if (nextId.isEmpty()) {
    loadCreateState();
  }
  emitChanged();
}

void AnnualViewModel::navigate(int delta) {
  if (!selection_) {
    return;
  }
  const QVariantList rows = annualRows();
  if (rows.isEmpty()) {
    return;
  }
  selection_->selectAnnual(
      navigatedSelectionId(rows, selectedAnnualId(), delta));
}

void AnnualViewModel::selectAnnual(const QString& id) {
  if (selection_) {
    selection_->selectAnnual(id.trimmed());
  }
}

QVariantMap AnnualViewModel::annualRowById(const QString& id) const {
  return rowById(annualRows(), id);
}

int AnnualViewModel::defaultYear() const {
  return QDate::currentDate().year() - 1;
}

void AnnualViewModel::loadCreateState() {
  name_.clear();
  year_ = defaultYear();
  analysisIds_.clear();
  contentIndex_ = 0;
  analysisMetadataDirty_ = false;
  refreshAnalysisSelectionRows();
  captureSavedState();
}

void AnnualViewModel::loadSelectedAnnual() {
  if (!selectors_ || selectedAnnualId().isEmpty()) {
    loadCreateState();
    return;
  }
  const QVariantMap payload = annualRowById(selectedAnnualId());
  if (payload.isEmpty()) {
    loadCreateState();
    return;
  }
  name_ = payload.value(QStringLiteral("name")).toString();
  year_ = payload.value(QStringLiteral("year"), defaultYear()).toInt();
  analysisIds_ =
      normalizedAnalysisIds(payload.value(QStringLiteral("analysisIds")));
  analysisMetadataDirty_ = false;
  refreshAnalysisSelectionRows();
  captureSavedState();
}

void AnnualViewModel::captureSavedState() {
  savedName_ = name_;
  savedYear_ = year_;
  savedAnalysisIds_ = analysisIds_;
  analysisMetadataDirty_ = false;
}

QVariantList AnnualViewModel::transactionSections() const {
  return {
      transactionSection(
          qstr(kDeduplicated), tr("Included entries (exact matches)"),
          annualTransactionGroups_.value(qstr(kDeduplicated)).toList()),
      transactionSection(
          qstr(kSimilar), tr("Included entries (possible variants)"),
          annualTransactionGroups_.value(qstr(kSimilar)).toList()),
      transactionSection(
          qstr(kDivergent), tr("Included entries (unique)"),
          annualTransactionGroups_.value(qstr(kDivergent)).toList()),
      transactionSection(
          qstr(kWorkspaceOnly),
          tr("Missing live transactions from selected year"),
          annualTransactionGroups_.value(qstr(kWorkspaceOnly)).toList()),
      transactionSection(
          qstr(kMissingLive), tr("Included deleted transactions"),
          annualTransactionGroups_.value(qstr(kMissingLive)).toList()),
  };
}

QVariantList AnnualViewModel::verificationRows() const {
  return {
      QVariantMap{{QStringLiteral("label"), tr("Included transactions")},
                  {QStringLiteral("value"), annualTransactions_.size()},
                  {QStringLiteral("tone"), QStringLiteral("primary")}},
      QVariantMap{
          {QStringLiteral("label"), tr("Duplicate entries")},
          {QStringLiteral("value"),
           verificationIssues_.value(QStringLiteral("duplicateCount")).toInt()},
          {QStringLiteral("tone"), QStringLiteral("primary")}},
      QVariantMap{{QStringLiteral("label"),
                   tr("Missing live transactions from selected year")},
                  {QStringLiteral("value"),
                   verificationIssues_.value(QStringLiteral("missingFromYear"))
                       .toInt()},
                  {QStringLiteral("tone"), QStringLiteral("primary")}},
      QVariantMap{
          {QStringLiteral("label"), tr("Included deleted transactions")},
          {QStringLiteral("value"),
           verificationIssues_.value(QStringLiteral("missingLive")).toInt()},
          {QStringLiteral("tone"), QStringLiteral("primary")}},
      QVariantMap{
          {QStringLiteral("label"),
           tr("Transactions from a different year than selected")},
          {QStringLiteral("value"),
           verificationIssues_.value(QStringLiteral("mixedInAnnual")).toInt()},
          {QStringLiteral("tone"),
           verificationIssues_.value(QStringLiteral("mixedInAnnual")).toInt() ==
                   0
               ? QStringLiteral("success")
               : QStringLiteral("danger")}},
      QVariantMap{{QStringLiteral("label"), tr("Assigned analyses")},
                  {QStringLiteral("value"), analysisIds_.size()},
                  {QStringLiteral("tone"), QStringLiteral("primary")}},
  };
}

QString AnnualViewModel::statusSummaryText() const {
  return tr("Neutral: %1, Unverified: %2, Verified: %3, Completed: %4")
      .arg(statusMetrics_.value(QStringLiteral("neutral")).toInt())
      .arg(statusMetrics_.value(QStringLiteral("unverified")).toInt())
      .arg(statusMetrics_.value(QStringLiteral("verified")).toInt())
      .arg(statusMetrics_.value(QStringLiteral("completed")).toInt());
}

void AnnualViewModel::toggleTransactionSection(const QString& key) {
  const QString trimmed = key.trimmed();
  if (trimmed.isEmpty()) {
    return;
  }
  sectionExpanded_.insert(trimmed, !isTransactionSectionExpanded(trimmed));
  emitChanged();
}

bool AnnualViewModel::isTransactionSectionExpanded(const QString& key) const {
  return sectionExpanded_.value(key.trimmed(), true).toBool();
}

void AnnualViewModel::rebuildAnnualResultState() {
  const QString currentAnnualId = selectedAnnualId();
  const int currentRevision = workspaceRevision();
  if (lastPreviewAnnualId_ == currentAnnualId && lastPreviewYear_ == year_ &&
      lastPreviewAnalysisIds_ == analysisIds_ &&
      lastPreviewRevision_ == currentRevision) {
    return;
  }
  lastPreviewAnnualId_ = currentAnnualId;
  lastPreviewYear_ = year_;
  lastPreviewAnalysisIds_ = analysisIds_;
  lastPreviewRevision_ = currentRevision;
  if (!annualWorkflow_) {
    applyAnnualResult({});
    return;
  }
  applyAnnualResult(annualWorkflow_->computeAnnualPreview(currentAnnualId,
                                                          analysisIds_, year_));
}

void AnnualViewModel::applyAnnualResult(const QVariantMap& result) {
  const QVariantMap stats = result.value(QStringLiteral("stats")).toMap();
  verificationIssues_ =
      QVariantMap{{QStringLiteral("missingFromYear"),
                   stats.value(QStringLiteral("missingFromYear"), 0)},
                  {QStringLiteral("mixedInAnnual"),
                   stats.value(QStringLiteral("mixedYear"), 0)},
                  {QStringLiteral("duplicateCount"),
                   stats.value(QStringLiteral("duplicateCount"), 0)},
                  {QStringLiteral("missingLive"),
                   stats.value(QStringLiteral("missingLive"), 0)}};
  statusMetrics_ = QVariantMap{
      {QStringLiteral("neutral"), stats.value(QStringLiteral("neutral"), 0)},
      {QStringLiteral("unverified"),
       stats.value(QStringLiteral("unverified"), 0)},
      {QStringLiteral("verified"), stats.value(QStringLiteral("verified"), 0)},
      {QStringLiteral("completed"),
       stats.value(QStringLiteral("completed"), 0)}};

  annualTransactions_ = rowsFromResultBucket(
      result.value(QStringLiteral("transactions")).toList());
  annualTransactionGroups_ = QVariantMap{
      {qstr(kDeduplicated),
       rowsFromResultBucket(result.value(qstr(kDeduplicated)).toList())},
      {qstr(kSimilar),
       rowsFromResultBucket(result.value(qstr(kSimilar)).toList())},
      {qstr(kDivergent),
       rowsFromResultBucket(result.value(qstr(kDivergent)).toList())},
      {qstr(kWorkspaceOnly),
       rowsFromResultBucket(result.value(qstr(kWorkspaceOnly)).toList())},
      {qstr(kMissingLive),
       rowsFromResultBucket(result.value(qstr(kMissingLive)).toList())}};
}

QVariantMap AnnualViewModel::emptyIssues() const {
  return {{QStringLiteral("missingFromYear"), 0},
          {QStringLiteral("mixedInAnnual"), 0},
          {QStringLiteral("duplicateCount"), 0},
          {QStringLiteral("missingLive"), 0}};
}

QVariantMap AnnualViewModel::emptyStatusMetrics() const {
  return {{QStringLiteral("neutral"), 0},
          {QStringLiteral("unverified"), 0},
          {QStringLiteral("verified"), 0},
          {QStringLiteral("completed"), 0}};
}

QVariantList
AnnualViewModel::rowsFromResultBucket(const QVariantList& rows) const {
  QVariantList out;
  out.reserve(rows.size());
  for (const QVariant& value : rows) {
    out.push_back(transactionRow(value.toMap()));
  }
  return out;
}

QVariantMap AnnualViewModel::transactionRow(const QVariantMap& source) const {
  QVariantMap row = source;
  const QVariantList names =
      source.value(QStringLiteral("sourceAnalysisNames")).toList();
  QStringList nameStrings;
  nameStrings.reserve(names.size());
  for (const QVariant& value : names) {
    const QString text = value.toString();
    if (!text.isEmpty()) {
      nameStrings.push_back(text);
    }
  }
  const int status = source.value(QStringLiteral("status")).toInt();
  row.insert(QStringLiteral("contractType"),
             source.value(QStringLiteral("contractType")).toString());
  row.insert(QStringLiteral("amountText"),
             amountText(source.value(QStringLiteral("amount"))));
  row.insert(QStringLiteral("sourceNamesText"),
             nameStrings.join(QStringLiteral(", ")));
  row.insert(QStringLiteral("allocatableText"),
             source.value(QStringLiteral("allocatable")).toBool()
                 ? tr("Allocatable")
                 : tr("Non-allocatable"));
  row.insert(QStringLiteral("contractTypeLabel"),
             nonEmptyString(source, QStringLiteral("contractType"),
                            tr("No type assigned")));
  row.insert(QStringLiteral("statusText"),
             nonEmptyString(source, QStringLiteral("statusText"),
                            ui::text::transactionStatus::neutral()));
  row.insert(QStringLiteral("statusTone"),
             status == 3   ? QStringLiteral("success")
             : status == 2 ? QStringLiteral("info")
             : status == 1 ? QStringLiteral("warning")
                           : QStringLiteral("primary"));
  return row;
}

QVariantMap
AnnualViewModel::transactionSection(const QString& key, const QString& title,
                                    const QVariantList& rows) const {
  return {{QStringLiteral("key"), key},
          {QStringLiteral("title"), title},
          {QStringLiteral("rows"), rows},
          {QStringLiteral("expanded"), isTransactionSectionExpanded(key)},
          {QStringLiteral("visible"), !rows.isEmpty()}};
}

} // namespace ui
