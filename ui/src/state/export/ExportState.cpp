/**
 * @file ui/src/state/export/ExportState.cpp
 * @brief Implements the UI state adapter for the Export view.
 */

#include "ui/state/export/ExportState.h"

#include <algorithm>

#include <QJsonDocument>
#include <QLocale>
#include <QVariant>

#include "ui/platform/filesystem/FileSystemBrowser.h"
#include "ui/shell/AppActions.h"
#include "ui/viewmodels/export/ExportRunListModel.h"
#include "ui/viewmodels/system/SettingsViewModel.h"
#include "ui/workflows/export/ExportWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

namespace {

constexpr auto kAnnual = "annual";
constexpr auto kAnalysis = "analysis";
constexpr auto kAnnualObject = "Annual";
constexpr auto kAnalysisObject = "Analysis";
constexpr auto kTab = "tab";
constexpr auto kPlot = "plot";
constexpr auto kCsv = "CSV";
constexpr auto kXlsx = "XLSX";
constexpr auto kPng = "PNG";
constexpr auto kJpg = "JPG";
constexpr int kCreateMode = 0;
constexpr int kProgressMode = 1;

QString qstr(const char *value) { return QString::fromLatin1(value); }

QString rowText(const QVariantMap &row, const QString &key,
                const QString &fallback = {}) {
  const QString value = row.value(key).toString();
  return value.isEmpty() ? fallback : value;
}

} // namespace

ExportState::ExportState(QObject *parent) : QObject(parent) {}

void ExportState::setWorkspace(WorkspaceFacade *value) {
  if (workspace_ == value) {
    return;
  }
  bindWorkspace(value);
  refreshFromWorkspace();
}

void ExportState::setExportWorkflow(ExportWorkflow *value) {
  if (exportWorkflow_ == value) {
    return;
  }
  bindWorkflow(value);
  refreshRuns();
  emitChanged();
}

void ExportState::setActions(Actions *value) {
  if (actions_ == value) {
    return;
  }
  bindActions(value);
  emitChanged();
}

void ExportState::setFileSystemBrowser(FileSystemBrowser *value) {
  if (fileSystemBrowser_ == value) {
    return;
  }
  fileSystemBrowser_ = value;
  if (targetDirectory_.isEmpty()) {
    targetDirectory_ = defaultTargetDirectory();
    appliedDefaultTargetDirectory_ = targetDirectory_;
  }
  emitChanged();
}

void ExportState::setSettings(SettingsViewModel *value) {
  if (settings_ == value) {
    return;
  }
  bindSettings(value);
  if (targetDirectory_.isEmpty() ||
      targetDirectory_ == appliedDefaultTargetDirectory_) {
    targetDirectory_ = defaultTargetDirectory();
    appliedDefaultTargetDirectory_ = targetDirectory_;
  }
  packageFormatIndex_ = settings_ ? settings_->exportArchiveFormat() : 0;
  emitChanged();
}

void ExportState::setTargetDirectory(const QString &value) {
  if (targetDirectory_ == value) {
    return;
  }
  targetDirectory_ = value;
  emitChanged();
}

void ExportState::setPackageFormatIndex(int value) {
  const int next = std::clamp(value, 0, 1);
  if (packageFormatIndex_ == next) {
    return;
  }
  packageFormatIndex_ = next;
  emitChanged();
}

void ExportState::setAddMode(const QString &value) {
  const QString normalized =
      value.trimmed().toLower() == qstr(kAnalysis) ? qstr(kAnalysis) : qstr(kAnnual);
  if (addMode_ == normalized) {
    return;
  }
  addMode_ = normalized;
  ensurePendingSelection();
  emitChanged();
}

QVariantList ExportState::addRows() const {
  return isAnnualMode() ? annualRows() : analysisRows();
}

QString ExportState::addTextRole() const {
  return isAnnualMode() ? QStringLiteral("display") : QStringLiteral("name");
}

int ExportState::pendingIndex() const {
  return indexForId(addRows(), pendingObjectId());
}

bool ExportState::canAddEntry() const { return !pendingObjectId().isEmpty(); }

bool ExportState::canStart() const {
  return exportWorkflow_ && workflowMode() == kCreateMode &&
         !targetDirectory_.isEmpty() && !exportItems().isEmpty();
}

bool ExportState::showClear() const {
  return exportWorkflow_ && workflowMode() == kCreateMode;
}

bool ExportState::showCancel() const {
  return exportWorkflow_ && workflowMode() == kProgressMode;
}

bool ExportState::showPause() const { return showCancel(); }

QString ExportState::pauseText() const {
  return exportWorkflow_ && exportWorkflow_->isPaused() ? tr("Resume")
                                                        : tr("Pause");
}

double ExportState::progress() const {
  return exportWorkflow_ && workflowMode() != kCreateMode ? exportWorkflow_->progress()
                                                          : 0.0;
}

QString ExportState::statusText() const {
  if (!exportWorkflow_) {
    return tr("Ready");
  }
  if (!exportWorkflow_->error().isEmpty()) {
    return exportWorkflow_->error();
  }
  if (!exportWorkflow_->phase().isEmpty()) {
    return exportWorkflow_->phase();
  }
  return tr("Ready");
}

bool ExportState::hasError() const {
  return exportWorkflow_ && !exportWorkflow_->error().isEmpty();
}

ExportRunList *ExportState::runs() const {
  return exportWorkflow_ ? exportWorkflow_->runs() : nullptr;
}

void ExportState::refreshFromWorkspace() {
  if (targetDirectory_.isEmpty()) {
    targetDirectory_ = defaultTargetDirectory();
    appliedDefaultTargetDirectory_ = targetDirectory_;
  }
  if (settings_) {
    packageFormatIndex_ = settings_->exportArchiveFormat();
  }
  ensurePendingSelection();
  refreshEntriesFromWorkspace();
  emitChanged();
}

void ExportState::browseDirectory() {
  if (actions_) {
    actions_->browseExportDirectory();
  }
}

void ExportState::clearForm() {
  targetDirectory_ = defaultTargetDirectory();
  appliedDefaultTargetDirectory_ = targetDirectory_;
  packageFormatIndex_ = settings_ ? settings_->exportArchiveFormat() : 0;
  exportEntries_.clear();
  ensurePendingSelection();
  if (exportWorkflow_) {
    exportWorkflow_->clearActiveRun();
  }
  emitChanged();
}

void ExportState::startExport() {
  if (!canStart()) {
    return;
  }
  const bool includeFormulas = settings_ ? settings_->exportIncludeFormulas() : true;
  const int itemCount = static_cast<int>(exportItems().size());
  exportWorkflow_->exportDataWithPayload(0, targetDirectory_, includeFormulas,
                                         defaultLocale(), payloadJson(),
                                         std::max(1, itemCount));
}

void ExportState::cancelExport() {
  if (exportWorkflow_) {
    exportWorkflow_->cancelExport();
  }
}

void ExportState::togglePause() {
  if (exportWorkflow_) {
    exportWorkflow_->togglePause();
  }
}

void ExportState::refreshRuns() {
  if (exportWorkflow_) {
    exportWorkflow_->refreshFromStateSnapshot();
  }
}

void ExportState::openRunLocationAt(int index) {
  if (exportWorkflow_) {
    exportWorkflow_->openRunLocationAt(index);
  }
}

void ExportState::removeRunAt(int index) {
  if (exportWorkflow_) {
    exportWorkflow_->removeRunAt(index);
  }
}

void ExportState::selectPendingRow(int index) {
  const QVariantList rows = addRows();
  const QVariantMap row =
      index >= 0 && index < rows.size() ? rows.at(index).toMap() : QVariantMap();
  const QString id = row.value(QStringLiteral("id")).toString();
  if (isAnnualMode()) {
    pendingAnnualId_ = id;
  } else {
    pendingAnalysisId_ = id;
  }
  emitChanged();
}

void ExportState::addPendingEntry() {
  const QString objectId = pendingObjectId();
  if (objectId.isEmpty()) {
    return;
  }
  if (isAnnualMode()) {
    const QVariantMap annual = annualRowById(objectId);
    exportEntries_.push_back(createAnnualEntry(
        objectId, rowText(annual, QStringLiteral("name")),
        analysesForAnnual(objectId, {})));
  } else {
    const QVariantMap analysis = analysisRowById(objectId);
    const QString type =
        rowText(analysis, QStringLiteral("type"), analysisTypeById(objectId));
    exportEntries_.push_back(createAnalysisEntry(
        objectId, rowText(analysis, QStringLiteral("name")), type,
        rowText(analysis, QStringLiteral("exportFormat"))));
  }
  emitChanged();
}

void ExportState::removeEntry(int index) {
  if (index < 0 || index >= exportEntries_.size()) {
    return;
  }
  exportEntries_.removeAt(index);
  emitChanged();
}

void ExportState::updateAnnualEntryAtIndex(int entryIndex, int annualIndex) {
  if (entryIndex < 0 || entryIndex >= exportEntries_.size()) {
    return;
  }
  const QVariantList rows = annualRows();
  if (annualIndex < 0 || annualIndex >= rows.size()) {
    return;
  }
  QVariantMap entry = exportEntries_.at(entryIndex).toMap();
  const QVariantList existingAnalyses =
      entry.value(QStringLiteral("analyses")).toList();
  const QVariantMap annual = rows.at(annualIndex).toMap();
  const QString id = annual.value(QStringLiteral("id")).toString();
  entry.insert(QStringLiteral("objectId"), id);
  entry.insert(QStringLiteral("objectName"), rowText(annual, QStringLiteral("name")));
  entry.insert(QStringLiteral("annualIndex"), annualIndex);
  entry.insert(QStringLiteral("analyses"), analysesForAnnual(id, existingAnalyses));
  exportEntries_[entryIndex] = entry;
  emitChanged();
}

void ExportState::updateAnnualCollapsed(int entryIndex, bool collapsed) {
  if (entryIndex < 0 || entryIndex >= exportEntries_.size()) {
    return;
  }
  QVariantMap entry = exportEntries_.at(entryIndex).toMap();
  entry.insert(QStringLiteral("collapsed"), collapsed);
  exportEntries_[entryIndex] = entry;
  emitChanged();
}

void ExportState::updateStandaloneAnalysisAtIndex(int entryIndex,
                                                  int analysisIndex) {
  if (entryIndex < 0 || entryIndex >= exportEntries_.size()) {
    return;
  }
  const QVariantList rows = analysisRows();
  if (analysisIndex < 0 || analysisIndex >= rows.size()) {
    return;
  }
  const QVariantMap row = rows.at(analysisIndex).toMap();
  const QString id = row.value(QStringLiteral("id")).toString();
  const QString type = rowText(row, QStringLiteral("type"), qstr(kTab)).toLower();
  exportEntries_[entryIndex] =
      createAnalysisEntry(id, rowText(row, QStringLiteral("name")), type, {});
  emitChanged();
}

void ExportState::updateStandaloneAnalysisExportType(int entryIndex,
                                                     const QString &exportType) {
  if (entryIndex < 0 || entryIndex >= exportEntries_.size()) {
    return;
  }
  const QVariantMap current = exportEntries_.at(entryIndex).toMap();
  exportEntries_[entryIndex] = createAnalysisEntry(
      current.value(QStringLiteral("objectId")).toString(),
      current.value(QStringLiteral("objectName")).toString(),
      current.value(QStringLiteral("analysisType")).toString(), exportType);
  emitChanged();
}

void ExportState::updateAnnualAnalysisExportType(int entryIndex,
                                                 int analysisIndex,
                                                 const QString &exportType) {
  if (entryIndex < 0 || entryIndex >= exportEntries_.size()) {
    return;
  }
  QVariantMap entry = exportEntries_.at(entryIndex).toMap();
  QVariantList analyses = entry.value(QStringLiteral("analyses")).toList();
  if (analysisIndex < 0 || analysisIndex >= analyses.size()) {
    return;
  }
  const QVariantMap analysis = analyses.at(analysisIndex).toMap();
  analyses[analysisIndex] = createAnalysisEntry(
      analysis.value(QStringLiteral("objectId")).toString(),
      analysis.value(QStringLiteral("objectName")).toString(),
      analysis.value(QStringLiteral("analysisType")).toString(), exportType);
  entry.insert(QStringLiteral("analyses"), analyses);
  exportEntries_[entryIndex] = entry;
  emitChanged();
}

QVariantList ExportState::exportItems() const {
  QVariantList out;
  for (const QVariant &value : exportEntries_) {
    const QVariantMap entry = value.toMap();
    const QString kind = entry.value(QStringLiteral("kind")).toString();
    if (kind == qstr(kAnnual)) {
      const QString annualId = entry.value(QStringLiteral("objectId")).toString();
      out.push_back(QVariantMap{{QStringLiteral("objectType"), qstr(kAnnualObject)},
                                {QStringLiteral("objectId"), annualId},
                                {QStringLiteral("objectName"),
                                 entry.value(QStringLiteral("objectName")).toString()},
                                {QStringLiteral("exportType"), QString()}});
      for (const QVariant &analysisValue :
           entry.value(QStringLiteral("analyses")).toList()) {
        const QVariantMap analysis = analysisValue.toMap();
        out.push_back(QVariantMap{
            {QStringLiteral("objectType"), qstr(kAnalysisObject)},
            {QStringLiteral("annualId"), annualId},
            {QStringLiteral("objectId"),
             analysis.value(QStringLiteral("objectId")).toString()},
            {QStringLiteral("objectName"),
             analysis.value(QStringLiteral("objectName")).toString()},
            {QStringLiteral("exportType"),
             analysis.value(QStringLiteral("exportType")).toString()}});
      }
      continue;
    }
    out.push_back(QVariantMap{
        {QStringLiteral("objectType"), qstr(kAnalysisObject)},
        {QStringLiteral("annualId"), QString()},
        {QStringLiteral("objectId"), entry.value(QStringLiteral("objectId")).toString()},
        {QStringLiteral("objectName"),
         entry.value(QStringLiteral("objectName")).toString()},
        {QStringLiteral("exportType"),
         entry.value(QStringLiteral("exportType")).toString()}});
  }
  return out;
}

void ExportState::clearAll() {
  exportEntries_.clear();
  ensurePendingSelection();
  emitChanged();
}

void ExportState::loadItems(const QVariantList &items) {
  QVariantList loadedEntries;
  for (const QVariant &value : items) {
    const QVariantMap item = value.toMap();
    const QString objectType = item.value(QStringLiteral("objectType")).toString();
    if (objectType == qstr(kAnnualObject)) {
      const QString annualId = item.value(QStringLiteral("objectId")).toString();
      loadedEntries.push_back(createAnnualEntry(
          annualId, item.value(QStringLiteral("objectName")).toString(),
          analysesForAnnual(annualId, {})));
      continue;
    }
    if (objectType != qstr(kAnalysisObject)) {
      continue;
    }
    const QString annualId = item.value(QStringLiteral("annualId")).toString();
    bool mappedToAnnual = false;
    if (!annualId.isEmpty()) {
      for (int i = loadedEntries.size() - 1; i >= 0; --i) {
        QVariantMap entry = loadedEntries.at(i).toMap();
        if (entry.value(QStringLiteral("kind")).toString() != qstr(kAnnual) ||
            entry.value(QStringLiteral("objectId")).toString() != annualId) {
          continue;
        }
        QVariantList analyses = entry.value(QStringLiteral("analyses")).toList();
        const QString analysisId = item.value(QStringLiteral("objectId")).toString();
        const QVariantMap loadedAnalysis = createAnalysisEntry(
            analysisId, item.value(QStringLiteral("objectName")).toString(),
            analysisTypeById(analysisId),
            item.value(QStringLiteral("exportType")).toString());
        bool replacedAnalysis = false;
        for (int analysisRow = 0; analysisRow < analyses.size(); ++analysisRow) {
          if (analyses.at(analysisRow)
                  .toMap()
                  .value(QStringLiteral("objectId"))
                  .toString() != analysisId) {
            continue;
          }
          analyses[analysisRow] = loadedAnalysis;
          replacedAnalysis = true;
          break;
        }
        if (!replacedAnalysis) {
          analyses.push_back(loadedAnalysis);
        }
        entry.insert(QStringLiteral("analyses"), analyses);
        loadedEntries[i] = entry;
        mappedToAnnual = true;
        break;
      }
    }
    if (!mappedToAnnual) {
      const QString analysisId = item.value(QStringLiteral("objectId")).toString();
      const QVariantMap row = analysisRowById(analysisId);
      loadedEntries.push_back(createAnalysisEntry(
          analysisId, item.value(QStringLiteral("objectName")).toString(),
          rowText(row, QStringLiteral("type"), analysisTypeById(analysisId)),
          item.value(QStringLiteral("exportType")).toString()));
    }
  }
  exportEntries_ = loadedEntries;
  ensurePendingSelection();
  emitChanged();
}

bool ExportState::isAnnualMode() const { return addMode_ == qstr(kAnnual); }

QVariantList ExportState::annualRows() const {
  return workspace_ ? workspace_->annualRows() : QVariantList();
}

QVariantList ExportState::analysisRows() const {
  return workspace_ ? workspace_->analysisRows() : QVariantList();
}

QVariantMap ExportState::annualRowById(const QString &id) const {
  for (const QVariant &value : annualRows()) {
    const QVariantMap row = value.toMap();
    if (row.value(QStringLiteral("id")).toString() == id) {
      return row;
    }
  }
  return {};
}

QVariantMap ExportState::analysisRowById(const QString &id) const {
  for (const QVariant &value : analysisRows()) {
    const QVariantMap row = value.toMap();
    if (row.value(QStringLiteral("id")).toString() == id) {
      return row;
    }
  }
  return {};
}

QString ExportState::pendingObjectId() const {
  return isAnnualMode() ? pendingAnnualId_ : pendingAnalysisId_;
}

int ExportState::indexForId(const QVariantList &rows, const QString &id) const {
  if (id.isEmpty()) {
    return -1;
  }
  for (int i = 0; i < rows.size(); ++i) {
    if (rows.at(i).toMap().value(QStringLiteral("id")).toString() == id) {
      return i;
    }
  }
  return -1;
}

void ExportState::ensurePendingSelection() {
  const QVariantList rows = addRows();
  if (rows.isEmpty()) {
    if (isAnnualMode()) {
      pendingAnnualId_.clear();
    } else {
      pendingAnalysisId_.clear();
    }
    return;
  }
  if (indexForId(rows, pendingObjectId()) >= 0) {
    return;
  }
  const QString firstId = rows.first().toMap().value(QStringLiteral("id")).toString();
  if (isAnnualMode()) {
    pendingAnnualId_ = firstId;
  } else {
    pendingAnalysisId_ = firstId;
  }
}

void ExportState::refreshEntriesFromWorkspace() {
  QVariantList refreshed;
  refreshed.reserve(exportEntries_.size());
  for (const QVariant &value : exportEntries_) {
    QVariantMap entry = value.toMap();
    if (entry.value(QStringLiteral("kind")).toString() == qstr(kAnnual)) {
      const QString id = entry.value(QStringLiteral("objectId")).toString();
      const QVariantMap annual = annualRowById(id);
      entry.insert(QStringLiteral("objectName"),
                   rowText(annual, QStringLiteral("name"),
                           entry.value(QStringLiteral("objectName")).toString()));
      entry.insert(QStringLiteral("analyses"),
                   analysesForAnnual(id, entry.value(QStringLiteral("analyses")).toList()));
      refreshed.push_back(entry);
      continue;
    }
    const QString id = entry.value(QStringLiteral("objectId")).toString();
    const QVariantMap analysis = analysisRowById(id);
    refreshed.push_back(createAnalysisEntry(
        id,
        rowText(analysis, QStringLiteral("name"),
                entry.value(QStringLiteral("objectName")).toString()),
        rowText(analysis, QStringLiteral("type"),
                entry.value(QStringLiteral("analysisType")).toString()),
        entry.value(QStringLiteral("exportType")).toString()));
  }
  exportEntries_ = refreshed;
}

QVariantMap ExportState::createAnnualEntry(const QString &id, const QString &name,
                                           const QVariantList &analyses) const {
  return {{QStringLiteral("kind"), qstr(kAnnual)},
          {QStringLiteral("isAnnual"), true},
          {QStringLiteral("objectId"), id},
          {QStringLiteral("objectName"), name},
          {QStringLiteral("annualIndex"), indexForId(annualRows(), id)},
          {QStringLiteral("collapsed"), false},
          {QStringLiteral("analyses"), analyses}};
}

QVariantMap ExportState::createAnalysisEntry(const QString &id, const QString &name,
                                             const QString &type,
                                             const QString &exportType) const {
  const QString normalizedType =
      type.trimmed().isEmpty() ? analysisTypeById(id) : type.trimmed().toLower();
  const QVariantList options = exportOptionsForAnalysisType(normalizedType);
  const QString selectedExportType =
      normalizedExportType(exportType, normalizedType);
  return {{QStringLiteral("kind"), qstr(kAnalysis)},
          {QStringLiteral("isAnnual"), false},
          {QStringLiteral("objectId"), id},
          {QStringLiteral("objectName"), name},
          {QStringLiteral("analysisIndex"), indexForId(analysisRows(), id)},
          {QStringLiteral("analysisType"), normalizedType},
          {QStringLiteral("exportType"), selectedExportType},
          {QStringLiteral("exportTypeOptions"), options},
          {QStringLiteral("exportTypeIndex"),
           options.indexOf(selectedExportType)}};
}

QVariantList ExportState::analysesForAnnual(
    const QString &annualId, const QVariantList &currentAnalyses) const {
  const QVariantMap annual = annualRowById(annualId);
  const QVariantList ids = annual.value(QStringLiteral("analysisIds")).toList().isEmpty()
                               ? annual.value(QStringLiteral("assignedAnalysisIds")).toList()
                               : annual.value(QStringLiteral("analysisIds")).toList();
  QVariantMap currentTypeById;
  for (const QVariant &value : currentAnalyses) {
    const QVariantMap row = value.toMap();
    const QString id = row.value(QStringLiteral("objectId")).toString();
    if (!id.isEmpty()) {
      currentTypeById.insert(id, row.value(QStringLiteral("exportType")));
    }
  }
  QVariantList out;
  for (const QVariant &idValue : ids) {
    const QString analysisId = idValue.toString();
    const QVariantMap row = analysisRowById(analysisId);
    out.push_back(createAnalysisEntry(
        analysisId, rowText(row, QStringLiteral("name")),
        rowText(row, QStringLiteral("type"), analysisTypeById(analysisId)),
        currentTypeById.value(analysisId).toString()));
  }
  return out;
}

QVariantList ExportState::exportOptionsForAnalysisType(const QString &type) const {
  return type.trimmed().toLower() == qstr(kPlot)
             ? QVariantList{qstr(kPng), qstr(kJpg)}
             : QVariantList{qstr(kCsv), qstr(kXlsx)};
}

QString ExportState::normalizedExportType(const QString &exportType,
                                          const QString &type) const {
  const QString upper = exportType.trimmed().toUpper();
  const QVariantList options = exportOptionsForAnalysisType(type);
  for (const QVariant &option : options) {
    if (option.toString() == upper) {
      return upper;
    }
  }
  return defaultExportType(type);
}

QString ExportState::defaultExportType(const QString &type) const {
  return exportOptionsForAnalysisType(type).value(0).toString();
}

QString ExportState::analysisTypeById(const QString &id) const {
  const QVariantMap row = analysisRowById(id);
  return rowText(row, QStringLiteral("type"), qstr(kTab)).toLower();
}

QString ExportState::defaultTargetDirectory() const {
  if (settings_ && !settings_->exportDefaultDirectory().isEmpty()) {
    return settings_->exportDefaultDirectory();
  }
  if (fileSystemBrowser_) {
    return fileSystemBrowser_->appDir();
  }
  return {};
}

QString ExportState::payloadJson() const {
  const QVariantMap payload{{QStringLiteral("targetDirectory"), targetDirectory_},
                            {QStringLiteral("packageFormatIndex"),
                             packageFormatIndex_},
                            {QStringLiteral("items"), exportItems()}};
  return QString::fromUtf8(QJsonDocument::fromVariant(payload).toJson(
      QJsonDocument::Compact));
}

QString ExportState::defaultLocale() const {
  return QLocale::system().name().replace(QLatin1Char('_'), QLatin1Char('-'));
}

int ExportState::workflowMode() const {
  return exportWorkflow_ ? exportWorkflow_->currentMode() : kCreateMode;
}

void ExportState::bindWorkspace(WorkspaceFacade *value) {
  if (workspace_) {
    disconnect(workspace_, nullptr, this, nullptr);
  }
  workspace_ = value;
  if (workspace_) {
    connect(workspace_, &WorkspaceFacade::dataRevisionChanged, this,
            &ExportState::refreshFromWorkspace);
  }
}

void ExportState::bindActions(Actions *value) {
  if (actions_) {
    disconnect(actions_, nullptr, this, nullptr);
  }
  actions_ = value;
  if (actions_) {
    connect(actions_, &Actions::exportDirectorySelected, this,
            &ExportState::setTargetDirectory);
  }
}

void ExportState::bindSettings(SettingsViewModel *value) {
  if (settings_) {
    disconnect(settings_, nullptr, this, nullptr);
  }
  settings_ = value;
  if (!settings_) {
    return;
  }
  connect(settings_, &SettingsViewModel::exportDefaultDirectoryChanged, this,
          [this]() {
            if (targetDirectory_.isEmpty() ||
                targetDirectory_ == appliedDefaultTargetDirectory_) {
              targetDirectory_ = defaultTargetDirectory();
              appliedDefaultTargetDirectory_ = targetDirectory_;
              emitChanged();
            }
          });
  connect(settings_, &SettingsViewModel::exportArchiveFormatChanged, this,
          [this]() {
            packageFormatIndex_ = settings_ ? settings_->exportArchiveFormat() : 0;
            emitChanged();
          });
  connect(settings_, &SettingsViewModel::exportIncludeFormulasChanged, this,
          &ExportState::emitChanged);
}

void ExportState::bindWorkflow(ExportWorkflow *value) {
  if (exportWorkflow_) {
    disconnect(exportWorkflow_, nullptr, this, nullptr);
  }
  exportWorkflow_ = value;
  if (exportWorkflow_) {
    connect(exportWorkflow_, &ExportWorkflow::stateChanged, this,
            &ExportState::emitChanged);
  }
}

void ExportState::emitChanged() { emit changed(); }

} // namespace ui
