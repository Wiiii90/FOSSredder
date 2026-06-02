/**
 * @file ui/src/viewmodels/AnnualViewModel.cpp
 * @brief Wires the Annual view state to UI services.
 */

#include "ui/viewmodels/AnnualViewModel.h"

#include <algorithm>
#include <cmath>

#include <QDate>
#include <QMetaType>
#include <QSet>

#include "ui/workspace/RowSelectionSupport.h"
#include "ui/workflows/annual/AnnualWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

namespace annual_view_model {

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

QString qstr(const char *value) { return QString::fromLatin1(value); }

double numberFromVariant(const QVariant &value) {
  bool ok = false;
  const double out = value.toDouble(&ok);
  return ok && std::isfinite(out) ? out : 0.0;
}

QString amountText(double value) {
  return QString::number(value, 'f', 2);
}

QString amountText(const QVariant &value) {
  return amountText(numberFromVariant(value));
}

QString nonEmptyString(const QVariantMap &map, const QString &key,
                       const QString &fallback = {}) {
  const QString value = map.value(key).toString();
  return value.isEmpty() ? fallback : value;
}
} // namespace annual_view_model

using namespace annual_view_model;

AnnualViewModel::AnnualViewModel(QObject *parent) : QObject(parent) {
  sectionExpanded_.insert(qstr(kDeduplicated), true);
  sectionExpanded_.insert(qstr(kSimilar), true);
  sectionExpanded_.insert(qstr(kDivergent), true);
  sectionExpanded_.insert(qstr(kWorkspaceOnly), true);
  sectionExpanded_.insert(qstr(kMissingLive), true);
  verificationIssues_ = emptyIssues();
  statusMetrics_ = emptyStatusMetrics();
  year_ = defaultYear();
}

void AnnualViewModel::setWorkspace(WorkspaceFacade *value) {
  if (workspace_ == value) {
    return;
  }
  bindWorkspace(value);
  refreshFromSelection();
  emitChanged();
}

void AnnualViewModel::setAnnualWorkflow(AnnualWorkflow *value) {
  if (annualWorkflow_ == value) {
    return;
  }
  annualWorkflow_ = value;
  lastPreviewRevision_ = -1;
  refreshFromSelection();
  emitChanged();
}

bool AnnualViewModel::isEdit() const { return !selectedAnnualId().isEmpty(); }

QVariantList AnnualViewModel::annualRows() const {
  return workspace_ ? workspace_->annualRows() : QVariantList();
}

QString AnnualViewModel::selectedAnnualId() const {
  return workspace_ ? workspace_->selectedAnnualId() : QString();
}

void AnnualViewModel::setName(const QString &value) {
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

bool AnnualViewModel::hasRows() const { return !annualRows().isEmpty(); }

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
  if (isEdit() && workspace_) {
    workspace_->selectAnnual({});
  }
  loadCreateState();
  rebuildAnnualResultState();
  emitChanged();
}

void AnnualViewModel::toggleWorkspace() {
  workspaceIndex_ = workspaceIndex_ == 0 ? 1 : 0;
  emitChanged();
}

void AnnualViewModel::stepYear(int delta) { setYear(year_ + delta); }

int AnnualViewModel::workspaceRevision() const {
  return workspace_ ? workspace_->dataRevision() : -1;
}

void AnnualViewModel::bindWorkspace(WorkspaceFacade *value) {
  if (workspace_) {
    disconnect(workspace_, nullptr, this, nullptr);
  }
  workspace_ = value;
  if (!workspace_) {
    return;
  }
  connect(workspace_, &WorkspaceFacade::selectedAnnualIdChanged, this,
          &AnnualViewModel::refreshFromSelection);
  connect(workspace_, &WorkspaceFacade::dataRevisionChanged, this,
          &AnnualViewModel::refreshFromSelection);
}

void AnnualViewModel::emitChanged() { emit changed(); }

} // namespace ui






namespace ui {

using namespace annual_view_model;

QStringList AnnualViewModel::normalizedAnalysisIds(const QVariant &values) const {
  QStringList out;
  QSet<QString> seen;
  const QVariantList list = values.toList();
  if (!list.isEmpty()) {
    for (const QVariant &value : list) {
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

QString AnnualViewModel::analysisIdFromVariant(const QVariant &value) const {
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

QVariantMap AnnualViewModel::analysisRowById(const QString &id) const {
  return rowById(normalizedAnalysisRows(), id);
}

QVariantMap AnnualViewModel::normalizeAnalysisRow(const QVariant &value) const {
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
  out.insert(QStringLiteral("includeCalcAdjustments"),
             source.value(QStringLiteral("includeCalcAdjustments"), true));
  out.insert(QStringLiteral("exportViewModel"),
             source.value(QStringLiteral("exportViewModel"), QStringLiteral("{}")));
  out.insert(
      QStringLiteral("snapshotTransactions"),
      source.value(QStringLiteral("snapshotTransactions"),
                   source.value(QStringLiteral("snapshotTransactionsJson"),
                                QStringLiteral("[]"))));
  return augmentAnalysisRow(out);
}

QVariantList AnnualViewModel::normalizedAnalysisRows() const {
  QVariantList rows;
  if (!workspace_) {
    return rows;
  }
  const QVariantList source = workspace_->analysisRows();
  rows.reserve(source.size());
  for (const QVariant &value : source) {
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
  for (const QString &id : analysisIds_) {
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
  for (const QVariant &value : rows) {
    const QString id = value.toMap().value(QStringLiteral("id")).toString();
    if (!id.isEmpty() && !selected.contains(id)) {
      out.push_back(value);
    }
  }
  return out;
}

QVariantMap AnnualViewModel::augmentAnalysisRow(const QVariantMap &row) const {
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

QVariantList AnnualViewModel::exportOptionsForType(const QString &type) const {
  if (type.trimmed().toLower() == qstr(kPlot)) {
    return {QStringLiteral("PNG"), QStringLiteral("JPG")};
  }
  return {QStringLiteral("XLSX"), QStringLiteral("CSV")};
}

int AnnualViewModel::exportFormatIndex(const QVariantList &options,
                                   const QString &exportFormat) const {
  const QString normalized = exportFormat.trimmed().toUpper();
  for (int i = 0; i < options.size(); ++i) {
    if (options.at(i).toString().toUpper() == normalized) {
      return i;
    }
  }
  return 0;
}

QString AnnualViewModel::normalizedExportFormat(const QString &value,
                                            const QString &type) const {
  const QString normalized = value.trimmed().toLower();
  const QString normalizedType = type.trimmed().toLower();
  const QStringList allowed = normalizedType == qstr(kPlot)
                                  ? QStringList{qstr(kPng), qstr(kJpg)}
                                  : QStringList{qstr(kXlsx), qstr(kCsv)};
  return allowed.contains(normalized) ? normalized : allowed.first();
}

} // namespace ui






namespace ui {

using namespace annual_view_model;

QVariantList AnnualViewModel::analysisIds() const {
  QVariantList out;
  out.reserve(analysisIds_.size());
  for (const QString &id : analysisIds_) {
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

void AnnualViewModel::removeAnalysis(const QString &id) {
  const QString trimmed = id.trimmed();
  if (trimmed.isEmpty() || !analysisIds_.contains(trimmed)) {
    return;
  }
  QStringList next = analysisIds_;
  next.removeAll(trimmed);
  setAnalysisIds(next);
}

void AnnualViewModel::setAnalysisExportFormat(const QString &id,
                                          const QString &exportFormat) {
  if (!workspace_) {
    return;
  }
  const QVariantMap row = analysisRowById(id.trimmed());
  if (row.isEmpty()) {
    return;
  }
  const QString type = nonEmptyString(row, QStringLiteral("type"), qstr(kTab));
  const QString nextFormat = normalizedExportFormat(exportFormat, type);
  workspace_->updateAnalysisExportFormat(id.trimmed(), nextFormat);
  analysisMetadataDirty_ = true;
  refreshChoices();
  rebuildAnnualResultState();
  emitChanged();
}

void AnnualViewModel::refreshChoices() {
  assignedAnalysisRows_ = buildAssignedAnalysisRows();
  availableAnalysisRows_ = buildAvailableAnalysisRows();
}

void AnnualViewModel::setAnalysisIds(const QStringList &ids) {
  QStringList next;
  QSet<QString> seen;
  for (const QString &id : ids) {
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
  refreshChoices();
  rebuildAnnualResultState();
  emitChanged();
}

} // namespace ui






namespace ui {

using namespace annual_view_model;

void AnnualViewModel::submitCreate() {
  if (!workspace_ || !canSubmit()) {
    return;
  }
  const QString id = workspace_->addAnnual(name_, year_, analysisIds_);
  if (id.isEmpty()) {
    return;
  }
  workspace_->selectAnnual(id);
  captureSavedState();
  emitChanged();
}

void AnnualViewModel::submitUpdate() {
  if (!workspace_ || selectedAnnualId().isEmpty() || !canSubmit()) {
    return;
  }
  workspace_->updateAnnual(selectedAnnualId(), name_, year_, analysisIds_);
  captureSavedState();
  emitChanged();
}

void AnnualViewModel::deleteCurrent() {
  if (!workspace_ || selectedAnnualId().isEmpty()) {
    return;
  }
  const QString removedId = selectedAnnualId();
  workspace_->deleteAnnual(removedId);

  const QString nextId = deleteNextSelectionId(annualRows(), removedId);
  workspace_->selectAnnual(nextId);
  if (nextId.isEmpty()) {
    loadCreateState();
  }
  emitChanged();
}

void AnnualViewModel::navigate(int delta) {
  if (!workspace_) {
    return;
  }
  const QVariantList rows = annualRows();
  if (rows.isEmpty()) {
    return;
  }
  workspace_->selectAnnual(
      navigatedSelectionId(rows, selectedAnnualId(), delta));
}

void AnnualViewModel::selectAnnual(const QString &id) {
  if (workspace_) {
    workspace_->selectAnnual(id.trimmed());
  }
}

QVariantMap AnnualViewModel::annualRowById(const QString &id) const {
  return rowById(annualRows(), id);
}

int AnnualViewModel::defaultYear() const { return QDate::currentDate().year() - 1; }

void AnnualViewModel::loadCreateState() {
  name_.clear();
  year_ = defaultYear();
  analysisIds_.clear();
  workspaceIndex_ = 0;
  analysisMetadataDirty_ = false;
  refreshChoices();
  captureSavedState();
}

void AnnualViewModel::loadSelectedAnnual() {
  if (!workspace_ || selectedAnnualId().isEmpty()) {
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
  refreshChoices();
  captureSavedState();
}

void AnnualViewModel::captureSavedState() {
  savedName_ = name_;
  savedYear_ = year_;
  savedAnalysisIds_ = analysisIds_;
  analysisMetadataDirty_ = false;
}

} // namespace ui





namespace ui {

using namespace annual_view_model;

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
      transactionSection(qstr(kMissingLive),
                         tr("Included deleted transactions"),
                         missingLiveRows()),
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

void AnnualViewModel::toggleTransactionSection(const QString &key) {
  const QString trimmed = key.trimmed();
  if (trimmed.isEmpty()) {
    return;
  }
  sectionExpanded_.insert(trimmed, !isTransactionSectionExpanded(trimmed));
  emitChanged();
}

bool AnnualViewModel::isTransactionSectionExpanded(const QString &key) const {
  return sectionExpanded_.value(key.trimmed(), true).toBool();
}

} // namespace ui





namespace ui {

using namespace annual_view_model;

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

void AnnualViewModel::applyAnnualResult(const QVariantMap &result) {
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
       rowsFromResultBucket(result.value(qstr(kWorkspaceOnly)).toList())}};

  if (groupedCount() == 0 && !annualTransactions_.isEmpty()) {
    QVariantList deduplicated;
    QVariantList similar;
    QVariantList divergent;
    QVariantList workspaceOnly;
    for (const QVariant &value : annualTransactions_) {
      const QVariantMap row = value.toMap();
      const QString key = row.value(QStringLiteral("key")).toString();
      if (key.startsWith(QStringLiteral("live|"))) {
        workspaceOnly.push_back(row);
      } else if (key.startsWith(QStringLiteral("sim|")) ||
                 row.value(QStringLiteral("isCalcVariant")).toBool()) {
        similar.push_back(row);
      } else if (key.startsWith(QStringLiteral("div|"))) {
        divergent.push_back(row);
      } else {
        deduplicated.push_back(row);
      }
    }
    annualTransactionGroups_.insert(qstr(kDeduplicated), deduplicated);
    annualTransactionGroups_.insert(qstr(kSimilar), similar);
    annualTransactionGroups_.insert(qstr(kDivergent), divergent);
    annualTransactionGroups_.insert(qstr(kWorkspaceOnly), workspaceOnly);
  }
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

int AnnualViewModel::groupedCount() const {
  return annualTransactionGroups_.value(qstr(kDeduplicated)).toList().size() +
         annualTransactionGroups_.value(qstr(kSimilar)).toList().size() +
         annualTransactionGroups_.value(qstr(kDivergent)).toList().size() +
         annualTransactionGroups_.value(qstr(kWorkspaceOnly)).toList().size();
}

} // namespace ui






namespace ui {

using namespace annual_view_model;

QVariantList AnnualViewModel::rowsFromResultBucket(const QVariantList &rows) const {
  QVariantList out;
  out.reserve(rows.size());
  for (const QVariant &value : rows) {
    out.push_back(transactionRow(value.toMap()));
  }
  return out;
}

QVariantList AnnualViewModel::missingLiveRows() const {
  QVariantList out;
  QSet<QString> seen;
  const QStringList buckets{qstr(kDeduplicated), qstr(kSimilar),
                            qstr(kDivergent)};
  for (const QString &bucket : buckets) {
    for (const QVariant &value :
         annualTransactionGroups_.value(bucket).toList()) {
      const QVariantMap row = value.toMap();
      if (!row.value(QStringLiteral("isMissingLive")).toBool()) {
        continue;
      }
      const QString key =
          nonEmptyString(row, QStringLiteral("key"),
                         row.value(QStringLiteral("id")).toString());
      if (key.isEmpty() || seen.contains(key)) {
        continue;
      }
      seen.insert(key);
      out.push_back(row);
    }
  }
  return out;
}

QVariantMap AnnualViewModel::transactionRow(const QVariantMap &source) const {
  QVariantMap row = source;
  const QVariantList names =
      source.value(QStringLiteral("sourceAnalysisNames")).toList();
  QStringList nameStrings;
  nameStrings.reserve(names.size());
  for (const QVariant &value : names) {
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
  row.insert(
      QStringLiteral("statusText"),
      nonEmptyString(source, QStringLiteral("statusText"), tr("Neutral")));
  row.insert(QStringLiteral("statusTone"),
             status == 3   ? QStringLiteral("success")
             : status == 2 ? QStringLiteral("info")
             : status == 1 ? QStringLiteral("warning")
                           : QStringLiteral("primary"));
  return row;
}

QVariantMap AnnualViewModel::transactionSection(const QString &key,
                                            const QString &title,
                                            const QVariantList &rows) const {
  return {{QStringLiteral("key"), key},
          {QStringLiteral("title"), title},
          {QStringLiteral("rows"), rows},
          {QStringLiteral("expanded"), isTransactionSectionExpanded(key)},
          {QStringLiteral("visible"), !rows.isEmpty()}};
}

} // namespace ui
