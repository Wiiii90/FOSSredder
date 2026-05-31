/**
 * @file ui/src/models/ExportRunList.cpp
 * @brief Implementation of the UI ExportRunList component.
 */

#include "ui/viewmodels/export/ExportRunListModel.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QVariant>

#include "ui/shared/payload/PayloadKeys.h"

namespace ui {

namespace {

QString fileName(const QString& path)
{
    const QFileInfo info(path);
    const QString name = info.fileName();
    return name.isEmpty() ? path : name;
}

QJsonArray exportItems(const QString& payloadText)
{
    if (payloadText.trimmed().isEmpty()) return {};
    const QJsonDocument document = QJsonDocument::fromJson(payloadText.toUtf8());
    if (!document.isObject()) return {};
    return document.object().value(QStringLiteral("items")).toArray();
}

QString exportTitle(const QString& file, const QString& payloadText)
{
    const QJsonArray items = exportItems(payloadText);
    if (items.isEmpty()) return fileName(file);

    QStringList analysisNames;
    for (const QJsonValue& value : items) {
        const QJsonObject item = value.toObject();
        if (item.value(QStringLiteral("objectType")).toString().compare(QStringLiteral("analysis"), Qt::CaseInsensitive) != 0) {
            continue;
        }
        const QString name = item.value(QStringLiteral("objectName")).toString().trimmed();
        if (!name.isEmpty()) analysisNames.push_back(name);
    }

    if (analysisNames.isEmpty()) return QObject::tr("Export");
    if (analysisNames.size() == 1) return QObject::tr("Export '%1'").arg(analysisNames.front());
    for (QString& name : analysisNames) {
        name = QObject::tr("'%1'").arg(name);
    }
    return QObject::tr("Export %1").arg(analysisNames.join(QStringLiteral(", ")));
}

QString exportStatusDetail(const QString& message, const QString& status,
                           const QString& payloadText)
{
    const QString messageText = message.trimmed();
    if (!messageText.isEmpty()) return messageText;
    if (exportItems(payloadText).isEmpty()) return {};

    const QString normalized = status.toLower();
    if (normalized == QStringLiteral("success")) return QObject::tr("Export completed successfully.");
    if (normalized == QStringLiteral("running")) return QObject::tr("Starting export...");
    return {};
}

} // namespace

ExportRunList::ExportRunList(QObject* parent)
    : Base(parent)
{
}

QVariant ExportRunList::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    const auto* r = rowPtr(index.row());
    if (!r) return {};

    switch (role) {
    case LogIdRole: return r->logId;
    case TimeRole: return r->time;
    case FileRole: return r->file;
    case StatusRole: return r->status;
    case MessageRole: return r->message;
    case PayloadRole: return r->payload;
    case DraftAttachedRole: return r->draftAttached;
    case DraftIdRole: return r->draftId;
    case StatementIdRole: return r->statementId;
    case DisplayTimeRole: return r->time;
    case DisplayTitleRole: return exportTitle(r->file, r->payload);
    case DisplayStatusDetailRole: return exportStatusDetail(r->message, r->status, r->payload);
    default: return {};
    }
}

QHash<int, QByteArray> ExportRunList::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[LogIdRole] = ui::payload::keys::exportRun::kLogId.toUtf8();
    roles[TimeRole] = ui::payload::keys::exportRun::kTime.toUtf8();
    roles[FileRole] = ui::payload::keys::exportRun::kFile.toUtf8();
    roles[StatusRole] = ui::payload::keys::common::kStatus.toUtf8();
    roles[MessageRole] = ui::payload::keys::exportRun::kMessage.toUtf8();
    roles[PayloadRole] = ui::payload::keys::exportRun::kPayload.toUtf8();
    roles[DraftAttachedRole] = ui::payload::keys::importRun::kDraftAttached.toUtf8();
    roles[DraftIdRole] = ui::payload::keys::importRun::kDraftId.toUtf8();
    roles[StatementIdRole] = ui::payload::keys::importRun::kStatementId.toUtf8();
    roles[DisplayTimeRole] = ui::payload::keys::common::kDisplayTime.toUtf8();
    roles[DisplayTitleRole] = ui::payload::keys::common::kDisplayTitle.toUtf8();
    roles[DisplayStatusDetailRole] = ui::payload::keys::common::kDisplayStatusDetail.toUtf8();
    return roles;
}

bool ExportRunList::upsertRun(const ExportRunRow& row)
{
    const int idx = findByLogId(row.logId);
    if (idx < 0) {
        appendRow(row);
        return true;
    }
    replaceRow(idx, row);
    emitRowChanged(idx, {LogIdRole, TimeRole, FileRole, StatusRole, MessageRole, PayloadRole,
                         DraftAttachedRole, DraftIdRole, StatementIdRole,
                         DisplayTimeRole, DisplayTitleRole, DisplayStatusDetailRole});
    return false;
}

int ExportRunList::findByLogId(const QString& logId) const
{
    if (logId.isEmpty()) return -1;
    const auto& items = rows();
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        if (items[static_cast<size_t>(i)].logId == logId) return i;
    }
    return -1;
}

ExportRunRow ExportRunList::at(int index) const
{
    const auto* row = rowPtr(index);
    return row ? *row : ExportRunRow{};
}

std::vector<ExportRunRow> ExportRunList::snapshot() const
{
    return rows();
}

void ExportRunList::setRuns(std::vector<ExportRunRow> runs)
{
    setRows(std::move(runs));
}

void ExportRunList::removeAt(int index)
{
    removeRow(index);
}

void ExportRunList::clear()
{
    clearRows();
}

}
