/**
 * @file ui/src/workspace/WorkspaceFacadeReporting.cpp
 * @brief Implements analysis and annual commands for the UI workspace facade.
 */

#include "ui/workspace/WorkspaceFacade.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "core/ports/workspace/WorkspaceCommands.h"
#include "ui/observability/Trace.h"
#include "ui/util/StringConversions.h"

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
  command.adjustmentsJson = strings::toStdString(adjustmentsJson);
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
    observability::traceWorkspace("WorkspaceFacade::addAnalysis",
                                  "Analysis create ignored; no writer bound");
    return {};
  }
  const auto command =
      makeAnalysisCommand({}, name, type, configJson, filterSpec, exportFormat,
                          includeCalcAdjustments, exportStateJson,
                          snapshotTransactionsJson, adjustmentsJson);
  if (rejectInvalidCommand("WorkspaceFacade::addAnalysis",
                           workspaceWriter_->validateAnalysis(command))) {
    return {};
  }
  const QString createdId =
      QString::fromStdString(workspaceWriter_->addAnalysis(command));
  observability::traceWorkspace(
      "WorkspaceFacade::addAnalysis", "Analysis created",
      {{observability::context::kId, createdId.toStdString()},
       {observability::context::kName, name.toStdString()},
       {"type", type.toStdString()},
       {observability::context::kFormat, exportFormat.toStdString()}});
  return createdId;
}

QVariantMap WorkspaceFacade::validateAnalysis(
    const QString &id, const QString &name, const QString &type,
    const QString &configJson, const QString &filterSpec,
    const QString &exportFormat, bool includeCalcAdjustments,
    const QString &exportStateJson, const QString &snapshotTransactionsJson,
    const QString &adjustmentsJson) const {
  if (!workspaceWriter_) {
    return validationResultToMap({});
  }
  return validationResultToMap(workspaceWriter_->validateAnalysis(
      makeAnalysisCommand(id, name, type, configJson, filterSpec, exportFormat,
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
  const auto command = makeAnalysisCommand(
      id, name, type, configJson, filterSpec, exportFormat,
      includeCalcAdjustments, exportStateJson, snapshotTransactionsJson,
      adjustmentsJson);
  if (rejectInvalidCommand("WorkspaceFacade::updateAnalysis",
                           workspaceWriter_->validateAnalysis(command))) {
    return;
  }
  workspaceWriter_->updateAnalysis(command);
  observability::traceWorkspace(
      "WorkspaceFacade::updateAnalysis", "Analysis updated",
      {{observability::context::kId, id.toStdString()},
       {observability::context::kName, name.toStdString()},
       {"type", type.toStdString()},
       {observability::context::kFormat, exportFormat.toStdString()}});
}

void WorkspaceFacade::deleteAnalysis(const QString &id) {
  if (workspaceWriter_) {
    observability::traceWorkspace(
        "WorkspaceFacade::deleteAnalysis", "Analysis delete requested",
        {{observability::context::kId, id.toStdString()}});
    workspaceWriter_->deleteAnalysis(strings::toStdString(id));
  }
}

QString WorkspaceFacade::addAnnual(const QString &name, int year,
                                   const QStringList &analysisIds) {
  if (!workspaceWriter_) {
    observability::traceWorkspace("WorkspaceFacade::addAnnual",
                                  "Annual create ignored; no writer bound");
    return {};
  }
  const auto command = makeAnnualCommand({}, name, year, analysisIds);
  if (rejectInvalidCommand("WorkspaceFacade::addAnnual",
                           workspaceWriter_->validateAnnual(command))) {
    return {};
  }
  const QString createdId =
      QString::fromStdString(workspaceWriter_->addAnnual(command));
  observability::traceWorkspace(
      "WorkspaceFacade::addAnnual", "Annual created",
      {{observability::context::kId, createdId.toStdString()},
       {observability::context::kName, name.toStdString()},
       {"year", std::to_string(year)},
       {"analysisCount", std::to_string(analysisIds.size())}});
  return createdId;
}

QVariantMap WorkspaceFacade::validateAnnual(
    const QString &id, const QString &name, int year,
    const QStringList &analysisIds) const {
  if (!workspaceWriter_) {
    return validationResultToMap({});
  }
  return validationResultToMap(workspaceWriter_->validateAnnual(
      makeAnnualCommand(id, name, year, analysisIds)));
}

void WorkspaceFacade::updateAnnual(const QString &id, const QString &name,
                                   int year,
                                   const QStringList &analysisIds) {
  if (!workspaceWriter_ || id.trimmed().isEmpty()) {
    return;
  }
  const auto command = makeAnnualCommand(id, name, year, analysisIds);
  if (rejectInvalidCommand("WorkspaceFacade::updateAnnual",
                           workspaceWriter_->validateAnnual(command))) {
    return;
  }
  workspaceWriter_->updateAnnual(command);
  observability::traceWorkspace(
      "WorkspaceFacade::updateAnnual", "Annual updated",
      {{observability::context::kId, id.toStdString()},
       {observability::context::kName, name.toStdString()},
       {"year", std::to_string(year)},
       {"analysisCount", std::to_string(analysisIds.size())}});
}

void WorkspaceFacade::deleteAnnual(const QString &id) {
  if (workspaceWriter_) {
    observability::traceWorkspace(
        "WorkspaceFacade::deleteAnnual", "Annual delete requested",
        {{observability::context::kId, id.toStdString()}});
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
  const auto command = makeAnalysisCommand(*it, exportFormat);
  if (rejectInvalidCommand("WorkspaceFacade::updateAnalysisExportFormat",
                           workspaceWriter_->validateAnalysis(command))) {
    return;
  }
  workspaceWriter_->updateAnalysis(command);
  observability::traceWorkspace(
      "WorkspaceFacade::updateAnalysisExportFormat",
      "Analysis export format updated",
      {{observability::context::kId, analysisId.toStdString()},
       {observability::context::kFormat, exportFormat.toStdString()}});
}

} // namespace ui
