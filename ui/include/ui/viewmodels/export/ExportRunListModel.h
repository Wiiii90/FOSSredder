/**
 * @file ui/include/ui/models/ExportRunList.h
 * @brief Declarations for the UI ExportRunList component.
 */

#pragma once

#include <qqmlintegration.h>

#include <QString>

#include "ui/viewmodels/base/RowListModel.h"

namespace ui {

struct ExportRunRow {
    QString logId;
    QString time;
    QString file;
    QString status;
    QString message;
    QString payload;
    bool draftAttached = false;
    QString draftId;
    QString statementId;
};

class ExportRunList : public models::RowListModel<ExportRunRow> {
    Q_OBJECT
    QML_NAMED_ELEMENT(ExportRunList)
    QML_UNCREATABLE("ExportRunList is exposed by the application context")
public:
    enum Roles {
        TimeRole = Qt::UserRole + 1,
        LogIdRole,
        FileRole,
        StatusRole,
        MessageRole,
        PayloadRole,
        DraftAttachedRole,
        DraftIdRole,
        StatementIdRole,
        DisplayTimeRole,
        DisplayTitleRole,
        DisplayStatusDetailRole
    };

private:
    using Base = models::RowListModel<ExportRunRow>;

public:
    explicit ExportRunList(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool upsertRun(const ExportRunRow& row);
    int findByLogId(const QString& logId) const;
    ExportRunRow at(int index) const;
    std::vector<ExportRunRow> snapshot() const;
    void setRuns(std::vector<ExportRunRow> runs);

    Q_INVOKABLE void removeAt(int index);
    Q_INVOKABLE void clear();
};

}
