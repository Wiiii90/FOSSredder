/**
 * @file ui/include/ui/workspace/WorkspaceRowProjector.h
 * @brief Declares helpers that project workspace state into QML-friendly rows.
 */

#pragma once

#include <QVariantList>

#include <QString>

namespace ui {

class WorkspaceCache;

QVariantList buildActorRows(const WorkspaceCache &cache);
QVariantList buildPropertyRows(const WorkspaceCache &cache);
QVariantList buildContractRows(const WorkspaceCache &cache);
QVariantList buildAnalysisRows(const WorkspaceCache &cache);
QVariantList buildAnnualRows(const WorkspaceCache &cache);
QVariantList buildStatementRows(const WorkspaceCache &cache);
QVariantList buildStatementTransactionRows(const WorkspaceCache &cache,
                                           const QString &statementId);

} // namespace ui
