/**
 * @file ui/src/workspace/WorkspaceFacadeReporting.cpp
 * @brief Implements analysis and annual commands for the UI workspace facade.
 */

#include "ui/workspace/WorkspaceFacade.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "core/application/analysis/AnalysisWorkflowSupport.h"
#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/shared/util/StringConversions.h"

namespace ui {
namespace {

core::ports::workspace::AnalysisCommand makeAnalysisCommand(
    const QString &id, const QString &name, const QString &type,
    const QString &configJson, const QString &filterSpec,
    const QString &exportFormat, bool includeCalculationAdjustments,
    const QString &exportStateJson, const QString &snapshotTransactionsJson,
    const QString &adjustmentsJson) {
  core::ports::workspace::AnalysisCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.type = strings::toStdString(type);
  command.configJson = strings::toStdString(configJson);
  command.filterSpec = strings::toStdString(filterSpec);
  command.exportFormat = strings::toStdString(exportFormat);
  command.includeCalculationAdjustments = includeCalculationAdjustments;
  command.exportStateJson = strings::toStdString(exportStateJson);
  command.snapshotTransactionsJson =
      strings::toStdString(snapshotTransactionsJson);
  command.adjustments = core::application::analysis::parseAnalysisAdjustmentsJson(
      adjustmentsJson.toStdString());
  return command;
}

core::ports::workspace::AnalysisCommand makeAnalysisCommand(
    const core::ports::workspace::AnalysisSnapshot &analysis,
    const QString &exportFormat) {
  core::ports::workspace::AnalysisCommand command;
  command.id = analysis.id;
  command.name = analysis.name;
  command.type = analysis.type;
  command.configJson = analysis.configJson;
  command.filterSpec = analysis.filterSpec;
  command.exportFormat = strings::toStdString(exportFormat);
  command.includeCalculationAdjustments =
      analysis.includeCalculationAdjustments;
  command.exportStateJson = analysis.exportStateJson;
  command.snapshotTransactionsJson = analysis.snapshotTransactionsJson;
  command.adjustments = analysis.adjustments;
  return command;
}

core::ports::workspace::AnnualCommand
makeAnnualCommand(const QString &id, const QString &name, int year,
                  const QStringList &analysisIds) {
  core::ports::workspace::AnnualCommand command;
  command.id = strings::toStdString(id);
  command.name = strings::toStdString(name);
  command.year = year;
  command.assignedAnalysisIds = strings::toStdList(analysisIds);
  return command;
}

} // namespace

QString WorkspaceFacade::addAnalysis(
    const QString &name, const QString &type,
    const QString &configJson, const QString &filterSpec,
    const QString &exportFormat, bool includeCalcAdjustments,
    const QString &exportStateJson, const QString &snapshotTransactionsJson,
    const QString &adjustmentsJson) {
  if (!workspaceWriter_) {
    return {};
  }
  return QString::fromStdString(workspaceWriter_->addAnalysis(
      makeAnalysisCommand({}, name, type, configJson, filterSpec, exportFormat,
                          includeCalcAdjustments, exportStateJson,
                          snapshotTransactionsJson, adjustmentsJson)));
}

void WorkspaceFacade::updateAnalysis(
    const QString &id, const QString &name, const QString &type,
    const QString &configJson, const QString &filterSpec,
    const QString &exportFormat, bool includeCalcAdjustments,
    const QString &exportStateJson, const QString &snapshotTransactionsJson,
    const QString &adjustmentsJson) {
  if (!workspaceWriter_ || id.trimmed().isEmpty()) {
    return;
  }
  workspaceWriter_->updateAnalysis(makeAnalysisCommand(
      id, name, type, configJson, filterSpec, exportFormat,
      includeCalcAdjustments, exportStateJson, snapshotTransactionsJson,
      adjustmentsJson));
}

void WorkspaceFacade::deleteAnalysis(const QString &id) {
  if (workspaceWriter_) {
    workspaceWriter_->deleteAnalysis(strings::toStdString(id));
  }
}

QString WorkspaceFacade::addAnnual(const QString &name, int year,
                                   const QStringList &analysisIds) {
  if (!workspaceWriter_) {
    return {};
  }
  return QString::fromStdString(
      workspaceWriter_->addAnnual(makeAnnualCommand({}, name, year, analysisIds)));
}

void WorkspaceFacade::updateAnnual(const QString &id, const QString &name,
                                   int year,
                                   const QStringList &analysisIds) {
  if (!workspaceWriter_ || id.trimmed().isEmpty()) {
    return;
  }
  workspaceWriter_->updateAnnual(makeAnnualCommand(id, name, year, analysisIds));
}

void WorkspaceFacade::deleteAnnual(const QString &id) {
  if (workspaceWriter_) {
    workspaceWriter_->deleteAnnual(strings::toStdString(id));
  }
}

void WorkspaceFacade::updateAnalysisExportFormat(const QString &analysisId,
                                                 const QString &exportFormat) {
  if (!workspaceWriter_ || !workspaceReader_) {
    return;
  }
  const std::string targetId = strings::toStdString(analysisId.trimmed());
  if (targetId.empty()) {
    return;
  }
  const auto analyses = workspaceReader_->workspaceSnapshot().analyses;
  const auto it =
      std::find_if(analyses.begin(), analyses.end(),
                   [&](const auto &item) { return item.id == targetId; });
  if (it == analyses.end()) {
    return;
  }
  workspaceWriter_->updateAnalysis(makeAnalysisCommand(*it, exportFormat));
}

} // namespace ui
