/**
 * @file ui/src/viewmodels/ExportViewModel.cpp
 * @brief Implements the QML-facing ExportViewModel API.
 */

#include "ui/viewmodels/ExportViewModel.h"

#include <algorithm>
#include <cstddef>

#include <QLocale>

#include "ui/observability/Trace.h"
#include "ui/platform/FileSystemBrowser.h"
#include "ui/presentation/PayloadKeys.h"
#include "ui/shell/AppActions.h"
#include "ui/shell/Settings.h"
#include "ui/workflows/ExportWorkflow.h"
#include "ui/workspace/WorkspaceCommands.h"
#include "ui/workspace/WorkspaceSelectors.h"
#include "ui/workspace/WorkspaceStore.h"

namespace ui {
namespace exportKeys = ui::payload::keys::exportSelection;

namespace {

inline constexpr auto kPlot = "plot";
inline constexpr auto kTab = "tab";
inline constexpr auto kCsv = "CSV";
inline constexpr auto kXlsx = "XLSX";
inline constexpr auto kPng = "PNG";
inline constexpr auto kJpg = "JPG";
inline constexpr int kCreateMode = 0;
inline constexpr int kProgressMode = 1;

QString qstr(const char* value) {
  return QString::fromLatin1(value);
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

QString nonEmptyString(const QVariantMap& map, const QString& key,
                       const QString& defaultValue = {}) {
  const QString value = map.value(key).toString();
  return value.isEmpty() ? defaultValue : value;
}
} // namespace

ExportViewModel::ExportViewModel(QObject* parent) : QObject(parent) {}

void ExportViewModel::setWorkspaceRoles(WorkspaceStore* store,
                                        WorkspaceCommands* commands,
                                        WorkspaceSelectors* selectors) {
  if (store_ == store && commands_ == commands && selectors_ == selectors) {
    return;
  }
  bindWorkspaceRoles(store, commands, selectors);
  configureExportLogSink();
  refreshFromWorkspace();
}

void ExportViewModel::setExportWorkflow(ExportWorkflow* value) {
  if (exportWorkflow_ == value) {
    return;
  }
  bindWorkflow(value);
  configureExportLogSink();
  emitChanged();
}

void ExportViewModel::setActions(Actions* value) {
  if (actions_ == value) {
    return;
  }
  bindActions(value);
  emitChanged();
}

void ExportViewModel::setFileSystemBrowser(FileSystemBrowser* value) {
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

void ExportViewModel::setSettings(Settings* value) {
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

void ExportViewModel::bindWorkspaceRoles(WorkspaceStore* store,
                                         WorkspaceCommands* commands,
                                         WorkspaceSelectors* selectors) {
  if (store_) {
    disconnect(store_, nullptr, this, nullptr);
  }
  store_ = store;
  commands_ = commands;
  selectors_ = selectors;
  if (store_) {
    connect(store_, &WorkspaceStore::dataRevisionChanged, this,
            &ExportViewModel::refreshFromWorkspace);
  }
}

void ExportViewModel::bindActions(Actions* value) {
  if (actions_) {
    disconnect(actions_, nullptr, this, nullptr);
  }
  actions_ = value;
  if (actions_) {
    connect(actions_, &Actions::exportDirectorySelected, this,
            &ExportViewModel::setTargetDirectory);
  }
}

void ExportViewModel::bindSettings(Settings* value) {
  if (settings_) {
    disconnect(settings_, nullptr, this, nullptr);
  }
  settings_ = value;
  if (!settings_) {
    return;
  }
  connect(settings_, &Settings::exportDefaultDirectoryChanged, this, [this]() {
    if (targetDirectory_.isEmpty() ||
        targetDirectory_ == appliedDefaultTargetDirectory_) {
      targetDirectory_ = defaultTargetDirectory();
      appliedDefaultTargetDirectory_ = targetDirectory_;
      emitChanged();
    }
  });
  connect(settings_, &Settings::exportArchiveFormatChanged, this, [this]() {
    packageFormatIndex_ = settings_ ? settings_->exportArchiveFormat() : 0;
    emitChanged();
  });
  connect(settings_, &Settings::exportIncludeFormulasChanged, this,
          &ExportViewModel::emitChanged);
}

void ExportViewModel::bindWorkflow(ExportWorkflow* value) {
  if (exportWorkflow_) {
    disconnect(exportWorkflow_, nullptr, this, nullptr);
  }
  exportWorkflow_ = value;
  if (exportWorkflow_) {
    connect(exportWorkflow_, &ExportWorkflow::stateChanged, this,
            &ExportViewModel::emitChanged);
  }
}

void ExportViewModel::configureExportLogSink() {
  if (!exportWorkflow_) {
    return;
  }
  if (!commands_) {
    exportWorkflow_->setExportLogSink({});
    return;
  }
  exportWorkflow_->setExportLogSink([this](const auto& log) {
    if (commands_) {
      commands_->saveExportLog(log);
    }
  });
}

bool ExportViewModel::isAnnualMode() const {
  return addMode_ == exportKeys::kAnnual;
}

QVariantList ExportViewModel::annualRows() const {
  return selectors_ ? selectors_->annualRows() : QVariantList();
}

QVariantList ExportViewModel::analysisRows() const {
  return selectors_ ? selectors_->analysisRows() : QVariantList();
}

QVariantMap ExportViewModel::annualRowById(const QString& id) const {
  return rowById(annualRows(), id);
}

QVariantMap ExportViewModel::analysisRowById(const QString& id) const {
  return rowById(analysisRows(), id);
}

QString ExportViewModel::selectedAddObjectId() const {
  return isAnnualMode() ? selectedAddAnnualId_ : selectedAddAnalysisId_;
}

void ExportViewModel::ensureAddSelection() {
  const QVariantList rows = addRows();
  if (rows.isEmpty()) {
    if (isAnnualMode()) {
      selectedAddAnnualId_.clear();
    } else {
      selectedAddAnalysisId_.clear();
    }
    return;
  }
  if (indexOfId(rows, selectedAddObjectId()) >= 0) {
    return;
  }
  const QString firstId =
      rows.first().toMap().value(QStringLiteral("id")).toString();
  if (isAnnualMode()) {
    selectedAddAnnualId_ = firstId;
  } else {
    selectedAddAnalysisId_ = firstId;
  }
}

void ExportViewModel::setTargetDirectory(const QString& value) {
  if (targetDirectory_ == value) {
    return;
  }
  targetDirectory_ = value;
  emitChanged();
}

void ExportViewModel::setPackageFormatIndex(int value) {
  const int next = std::clamp(value, 0, 1);
  if (packageFormatIndex_ == next) {
    return;
  }
  packageFormatIndex_ = next;
  emitChanged();
}

void ExportViewModel::setAddMode(const QString& value) {
  const QString normalized = value.trimmed().toLower() == exportKeys::kAnalysis
                                 ? exportKeys::kAnalysis
                                 : exportKeys::kAnnual;
  if (addMode_ == normalized) {
    return;
  }
  addMode_ = normalized;
  ensureAddSelection();
  emitChanged();
}

QVariantList ExportViewModel::addRows() const {
  return isAnnualMode() ? annualRows() : analysisRows();
}

QString ExportViewModel::addTextRole() const {
  return isAnnualMode() ? QStringLiteral("display") : QStringLiteral("name");
}

int ExportViewModel::selectedAddIndex() const {
  return indexOfId(addRows(), selectedAddObjectId());
}

bool ExportViewModel::canAddEntry() const {
  return !selectedAddObjectId().isEmpty();
}

void ExportViewModel::refreshFromWorkspace() {
  if (targetDirectory_.isEmpty()) {
    targetDirectory_ = defaultTargetDirectory();
    appliedDefaultTargetDirectory_ = targetDirectory_;
  }
  if (settings_) {
    packageFormatIndex_ = settings_->exportArchiveFormat();
  }
  ensureAddSelection();
  refreshEntriesFromWorkspace();
  emitChanged();
}

void ExportViewModel::clearForm() {
  targetDirectory_ = defaultTargetDirectory();
  appliedDefaultTargetDirectory_ = targetDirectory_;
  packageFormatIndex_ = settings_ ? settings_->exportArchiveFormat() : 0;
  exportEntries_.clear();
  ensureAddSelection();
  if (exportWorkflow_) {
    exportWorkflow_->clearActiveExportLog();
  }
  emitChanged();
}

QString ExportViewModel::defaultTargetDirectory() const {
  if (settings_ && !settings_->exportDefaultDirectory().isEmpty()) {
    return settings_->exportDefaultDirectory();
  }
  if (fileSystemBrowser_) {
    return fileSystemBrowser_->appDir();
  }
  return {};
}

void ExportViewModel::refreshEntriesFromWorkspace() {
  QVariantList refreshed;
  refreshed.reserve(exportEntries_.size());
  for (const QVariant& value : exportEntries_) {
    QVariantMap entry = value.toMap();
    if (entry.value(QStringLiteral("kind")).toString() == exportKeys::kAnnual) {
      const QString id = entry.value(QStringLiteral("objectId")).toString();
      const QVariantMap annual = annualRowById(id);
      entry.insert(
          QStringLiteral("objectName"),
          nonEmptyString(annual, QStringLiteral("name"),
                         entry.value(QStringLiteral("objectName")).toString()));
      entry.insert(QStringLiteral("analyses"),
                   analysesForAnnual(
                       id, entry.value(QStringLiteral("analyses")).toList()));
      refreshed.push_back(entry);
      continue;
    }
    const QString id = entry.value(QStringLiteral("objectId")).toString();
    const QVariantMap analysis = analysisRowById(id);
    refreshed.push_back(createAnalysisEntry(
        id,
        nonEmptyString(analysis, QStringLiteral("name"),
                       entry.value(QStringLiteral("objectName")).toString()),
        nonEmptyString(analysis, QStringLiteral("type"),
                       entry.value(QStringLiteral("analysisType")).toString()),
        entry.value(QStringLiteral("exportType")).toString()));
  }
  exportEntries_ = refreshed;
}

QVariantMap
ExportViewModel::createAnnualEntry(const QString& id, const QString& name,
                                   const QVariantList& analyses) const {
  return {{QStringLiteral("kind"), exportKeys::kAnnual},
          {QStringLiteral("isAnnual"), true},
          {QStringLiteral("objectId"), id},
          {QStringLiteral("objectName"), name},
          {QStringLiteral("annualIndex"), indexOfId(annualRows(), id)},
          {QStringLiteral("collapsed"), false},
          {QStringLiteral("analyses"), analyses}};
}

QVariantMap
ExportViewModel::createAnalysisEntry(const QString& id, const QString& name,
                                     const QString& type,
                                     const QString& exportType) const {
  const QString normalizedType = type.trimmed().isEmpty()
                                     ? analysisTypeById(id)
                                     : type.trimmed().toLower();
  const QVariantList options = exportOptionsForAnalysisType(normalizedType);
  const QString selectedExportType =
      normalizedExportType(exportType, normalizedType);
  return {
      {QStringLiteral("kind"), exportKeys::kAnalysis},
      {QStringLiteral("isAnnual"), false},
      {QStringLiteral("objectId"), id},
      {QStringLiteral("objectName"), name},
      {QStringLiteral("analysisIndex"), indexOfId(analysisRows(), id)},
      {QStringLiteral("analysisType"), normalizedType},
      {QStringLiteral("exportType"), selectedExportType},
      {QStringLiteral("exportTypeOptions"), options},
      {QStringLiteral("exportTypeIndex"), options.indexOf(selectedExportType)}};
}

QVariantList
ExportViewModel::analysesForAnnual(const QString& annualId,
                                   const QVariantList& currentAnalyses) const {
  const QVariantMap annual = annualRowById(annualId);
  const QVariantList ids =
      annual.value(QStringLiteral("analysisIds")).toList().isEmpty()
          ? annual.value(QStringLiteral("assignedAnalysisIds")).toList()
          : annual.value(QStringLiteral("analysisIds")).toList();
  QVariantMap currentTypeById;
  for (const QVariant& value : currentAnalyses) {
    const QVariantMap row = value.toMap();
    const QString id = row.value(QStringLiteral("objectId")).toString();
    if (!id.isEmpty()) {
      currentTypeById.insert(id, row.value(QStringLiteral("exportType")));
    }
  }
  QVariantList out;
  for (const QVariant& idValue : ids) {
    const QString analysisId = idValue.toString();
    const QVariantMap row = analysisRowById(analysisId);
    out.push_back(createAnalysisEntry(
        analysisId, nonEmptyString(row, QStringLiteral("name")),
        nonEmptyString(row, QStringLiteral("type"),
                       analysisTypeById(analysisId)),
        currentTypeById.value(analysisId).toString()));
  }
  return out;
}

QVariantList
ExportViewModel::exportOptionsForAnalysisType(const QString& type) const {
  return type.trimmed().toLower() == qstr(kPlot)
             ? QVariantList{qstr(kPng), qstr(kJpg)}
             : QVariantList{qstr(kCsv), qstr(kXlsx)};
}

QString ExportViewModel::normalizedExportType(const QString& exportType,
                                              const QString& type) const {
  const QString upper = exportType.trimmed().toUpper();
  const QVariantList options = exportOptionsForAnalysisType(type);
  for (const QVariant& option : options) {
    if (option.toString() == upper) {
      return upper;
    }
  }
  return defaultExportType(type);
}

QString ExportViewModel::defaultExportType(const QString& type) const {
  return exportOptionsForAnalysisType(type).value(0).toString();
}

QString ExportViewModel::analysisTypeById(const QString& id) const {
  const QVariantMap row = analysisRowById(id);
  return nonEmptyString(row, QStringLiteral("type"), qstr(kTab)).toLower();
}

void ExportViewModel::selectAddRow(int index) {
  const QVariantList rows = addRows();
  const QVariantMap row = index >= 0 && index < rows.size()
                              ? rows.at(index).toMap()
                              : QVariantMap();
  const QString id = row.value(QStringLiteral("id")).toString();
  if (isAnnualMode()) {
    selectedAddAnnualId_ = id;
  } else {
    selectedAddAnalysisId_ = id;
  }
  emitChanged();
}

void ExportViewModel::addSelectedEntry() {
  const QString objectId = selectedAddObjectId();
  if (objectId.isEmpty()) {
    return;
  }
  if (isAnnualMode()) {
    const QVariantMap annual = annualRowById(objectId);
    exportEntries_.push_back(createAnnualEntry(
        objectId, nonEmptyString(annual, QStringLiteral("name")),
        analysesForAnnual(objectId, {})));
  } else {
    const QVariantMap analysis = analysisRowById(objectId);
    const QString type = nonEmptyString(analysis, QStringLiteral("type"),
                                        analysisTypeById(objectId));
    exportEntries_.push_back(createAnalysisEntry(
        objectId, nonEmptyString(analysis, QStringLiteral("name")), type,
        nonEmptyString(analysis, QStringLiteral("exportFormat"))));
  }
  emitChanged();
}

void ExportViewModel::removeEntry(int index) {
  if (index < 0 || index >= exportEntries_.size()) {
    return;
  }
  exportEntries_.removeAt(index);
  emitChanged();
}

void ExportViewModel::updateAnnualEntryAtIndex(int entryIndex,
                                               int annualIndex) {
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
  entry.insert(QStringLiteral("objectName"),
               nonEmptyString(annual, QStringLiteral("name")));
  entry.insert(QStringLiteral("annualIndex"), annualIndex);
  entry.insert(QStringLiteral("analyses"),
               analysesForAnnual(id, existingAnalyses));
  exportEntries_[entryIndex] = entry;
  emitChanged();
}

void ExportViewModel::updateAnnualCollapsed(int entryIndex, bool collapsed) {
  if (entryIndex < 0 || entryIndex >= exportEntries_.size()) {
    return;
  }
  QVariantMap entry = exportEntries_.at(entryIndex).toMap();
  entry.insert(QStringLiteral("collapsed"), collapsed);
  exportEntries_[entryIndex] = entry;
  emitChanged();
}

void ExportViewModel::updateStandaloneAnalysisAtIndex(int entryIndex,
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
  const QString type =
      nonEmptyString(row, QStringLiteral("type"), qstr(kTab)).toLower();
  exportEntries_[entryIndex] = createAnalysisEntry(
      id, nonEmptyString(row, QStringLiteral("name")), type, {});
  emitChanged();
}

void ExportViewModel::updateStandaloneAnalysisExportType(
    int entryIndex, const QString& exportType) {
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

void ExportViewModel::updateAnnualAnalysisExportType(
    int entryIndex, int analysisIndex, const QString& exportType) {
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

QVariantList ExportViewModel::exportItems() const {
  QVariantList out;
  for (const QVariant& value : exportEntries_) {
    const QVariantMap entry = value.toMap();
    const QString kind = entry.value(QStringLiteral("kind")).toString();
    if (kind == exportKeys::kAnnual) {
      const QString annualId =
          entry.value(QStringLiteral("objectId")).toString();
      out.push_back(
          QVariantMap{{exportKeys::kObjectType, exportKeys::kAnnual},
                      {exportKeys::kObjectId, annualId},
                      {exportKeys::kObjectName,
                       entry.value(QStringLiteral("objectName")).toString()},
                      {exportKeys::kExportType, QString()}});
      for (const QVariant& analysisValue :
           entry.value(QStringLiteral("analyses")).toList()) {
        const QVariantMap analysis = analysisValue.toMap();
        out.push_back(QVariantMap{
            {exportKeys::kObjectType, exportKeys::kAnalysis},
            {exportKeys::kAnnualId, annualId},
            {exportKeys::kObjectId,
             analysis.value(QStringLiteral("objectId")).toString()},
            {exportKeys::kObjectName,
             analysis.value(QStringLiteral("objectName")).toString()},
            {exportKeys::kExportType,
             analysis.value(QStringLiteral("exportType")).toString()}});
      }
      continue;
    }
    out.push_back(
        QVariantMap{{exportKeys::kObjectType, exportKeys::kAnalysis},
                    {exportKeys::kAnnualId, QString()},
                    {exportKeys::kObjectId,
                     entry.value(QStringLiteral("objectId")).toString()},
                    {exportKeys::kObjectName,
                     entry.value(QStringLiteral("objectName")).toString()},
                    {exportKeys::kExportType,
                     entry.value(QStringLiteral("exportType")).toString()}});
  }
  return out;
}

bool ExportViewModel::canStart() const {
  return exportWorkflow_ && workflowMode() == kCreateMode &&
         !targetDirectory_.isEmpty() && !exportItems().isEmpty();
}

bool ExportViewModel::showClear() const {
  return exportWorkflow_ && workflowMode() == kCreateMode;
}

bool ExportViewModel::showCancel() const {
  return exportWorkflow_ && workflowMode() == kProgressMode;
}

bool ExportViewModel::showPause() const {
  return showCancel();
}

bool ExportViewModel::isPaused() const {
  return exportWorkflow_ && exportWorkflow_->isPaused();
}

QString ExportViewModel::pauseText() const {
  return isPaused() ? tr("Resume") : tr("Pause");
}

double ExportViewModel::progress() const {
  return exportWorkflow_ && workflowMode() != kCreateMode
             ? exportWorkflow_->progress()
             : 0.0;
}

QString ExportViewModel::statusText() const {
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

bool ExportViewModel::hasError() const {
  return exportWorkflow_ && !exportWorkflow_->error().isEmpty();
}

QVariantList ExportViewModel::exportLogs() const {
  return selectors_ ? selectors_->exportLogRows() : QVariantList{};
}

void ExportViewModel::browseDirectory() {
  if (actions_) {
    actions_->browseExportDirectory();
  }
}

void ExportViewModel::startExport() {
  if (!canStart()) {
    return;
  }
  observability::traceViewModel(
      "ExportViewModel::startExport", "Export start submitted",
      {{observability::context::kPath, targetDirectory_.toStdString()},
       {observability::context::kCount,
        std::to_string(std::max(1, static_cast<int>(exportItems().size())))}});
  const bool includeFormulas =
      settings_ ? settings_->exportIncludeFormulas() : true;
  const int itemCount = static_cast<int>(exportItems().size());
  exportWorkflow_->exportDataWithPayload(0, targetDirectory_, includeFormulas,
                                         defaultLocale(), payload(),
                                         std::max(1, itemCount));
}

void ExportViewModel::cancelExport() {
  if (exportWorkflow_) {
    observability::traceViewModel("ExportViewModel::cancelExport",
                                  "Export cancel submitted");
    exportWorkflow_->cancelExport();
  }
}

void ExportViewModel::pauseExport() {
  if (exportWorkflow_) {
    exportWorkflow_->pauseExport();
  }
}

void ExportViewModel::resumeExport() {
  if (exportWorkflow_) {
    exportWorkflow_->resumeExport();
  }
}

void ExportViewModel::openExportLogLocation(const QString& logId) {
  if (!selectors_ || !fileSystemBrowser_) {
    return;
  }

  const QString targetPath = selectors_->exportLogTargetPath(logId);
  fileSystemBrowser_->openLocation(targetPath);
}

void ExportViewModel::deleteExportLog(const QString& logId) {
  if (!commands_) {
    return;
  }
  observability::traceViewModel(
      "ExportViewModel::deleteExportLog", "Export log delete submitted",
      {{observability::context::kId, logId.toStdString()}});
  const QString trimmed = logId.trimmed();
  if (!trimmed.isEmpty()) {
    commands_->deleteExportLog(trimmed);
  }
}

QVariantMap ExportViewModel::payload() const {
  return {{exportKeys::kTargetDirectory, targetDirectory_},
          {exportKeys::kPackageFormatIndex, packageFormatIndex_},
          {exportKeys::kItems, exportItems()}};
}

QString ExportViewModel::defaultLocale() const {
  return QLocale::system().name().replace(QLatin1Char('_'), QLatin1Char('-'));
}

int ExportViewModel::workflowMode() const {
  return exportWorkflow_ ? exportWorkflow_->currentMode() : kCreateMode;
}

void ExportViewModel::emitChanged() {
  emit changed();
}

} // namespace ui
