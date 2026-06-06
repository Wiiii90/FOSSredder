/**
 * @file ui/include/ui/workspace/WorkspacePayloads.h
 * @brief Declares workspace-specific QML payload builders.
 */

#pragma once

#include <vector>

#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include "core/ports/workspace/WorkspaceCommands.h"
#include "core/ports/workspace/WorkspaceSnapshot.h"

namespace ui::workspace_payloads {

/**
 * @brief Maps validation results to a QML-friendly payload.
 * @param result Core workspace validation result.
 * @return Payload with valid and issues fields.
 */
QVariantMap
validationResult(const core::ports::workspace::ValidationResult& result);

/**
 * @brief Builds an actor row.
 * @param actor Actor snapshot.
 * @return QML-friendly actor row.
 */
QVariantMap actorRow(const core::ports::workspace::ActorSnapshot& actor);

/**
 * @brief Builds a property row.
 * @param property Property snapshot.
 * @return QML-friendly property row.
 */
QVariantMap
propertyRow(const core::ports::workspace::PropertySnapshot& property);

/**
 * @brief Builds a contract row.
 * @param contract Contract snapshot.
 * @return QML-friendly contract row.
 */
QVariantMap
contractRow(const core::ports::workspace::ContractSnapshot& contract);

/**
 * @brief Builds an analysis row.
 * @param analysis Analysis snapshot.
 * @return QML-friendly analysis row.
 */
QVariantMap
analysisRow(const core::ports::workspace::AnalysisSnapshot& analysis);

/**
 * @brief Builds an analysis config row.
 * @param config Analysis config DTO.
 * @return QML-friendly config row.
 */
QVariantMap
analysisConfigRow(const core::ports::analysis::AnalysisConfigInput& config);

/**
 * @brief Builds an analysis filter row.
 * @param filter Analysis filter DTO.
 * @return QML-friendly filter row.
 */
QVariantMap
analysisFilterRow(const core::ports::analysis::AnalysisFilterSelection& filter);

/**
 * @brief Builds an annual row.
 * @param annual Annual snapshot.
 * @return QML-friendly annual row.
 */
QVariantMap annualRow(const core::ports::workspace::AnnualSnapshot& annual);

/**
 * @brief Builds a statement row.
 * @param statement Statement snapshot.
 * @return QML-friendly statement row.
 */
QVariantMap
statementRow(const core::ports::workspace::StatementSnapshot& statement);

/**
 * @brief Builds a transaction row.
 * @param transaction Transaction snapshot.
 * @param contractType Contract type resolved from the transaction contract id.
 * @return QML-friendly transaction row.
 */
QVariantMap
transactionRow(const core::ports::workspace::TransactionSnapshot& transaction,
               const QString& contractType = {});

/**
 * @brief Builds transaction rows.
 * @param transactions Transaction snapshots.
 * @return QML-friendly transaction rows.
 */
QVariantList
transactionRows(const std::vector<core::ports::workspace::TransactionSnapshot>&
                    transactions);

/**
 * @brief Builds a lightweight statement transaction row.
 * @param transaction Transaction snapshot.
 * @return QML-friendly transaction row used in statement navigation.
 */
QVariantMap statementTransactionRow(
    const core::ports::workspace::TransactionSnapshot& transaction);

/**
 * @brief Builds an import log row.
 * @param log Import log snapshot.
 * @return QML-friendly import log row.
 */
QVariantMap importLogRow(const core::ports::workspace::ImportLogSnapshot& log);

/**
 * @brief Builds an export log row.
 * @param log Export log snapshot.
 * @return QML-friendly export log row.
 */
QVariantMap exportLogRow(const core::ports::workspace::ExportLogSnapshot& log);

/**
 * @brief Builds a dropdown row.
 * @param id Row id.
 * @param display Display text.
 * @param name Optional name value.
 * @param type Optional type value.
 * @param actorIds Optional linked actor ids.
 * @param propertyIds Optional linked property ids.
 * @param allocatableMode Optional allocatable mode.
 * @return QML-friendly dropdown row.
 */
QVariantMap
dropdownRow(const QString& id, const QString& display, const QString& name = {},
            const QString& type = {}, const QVariantList& actorIds = {},
            const QVariantList& propertyIds = {},
            const QString& allocatableMode = QStringLiteral("mixed"));

} // namespace ui::workspace_payloads
