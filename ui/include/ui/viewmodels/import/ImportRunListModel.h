/**
 * @file ui/include/ui/models/ImportRunList.h
 * @brief Declarations for the UI ImportRunList component.
 */

#pragma once

#include <qqmlintegration.h>

#include <QString>

#include "ui/viewmodels/base/RowListModel.h"

namespace ui {

struct ImportRunRow {
    QString logId;
    QString time;
    QString type;
    QString file;
    QString status;
    QString message;
    bool draftAttached = false;
    QString draftId;
    QString statementId;
};

class ImportRunList : public models::RowListModel<ImportRunRow> {
    Q_OBJECT
    QML_NAMED_ELEMENT(ImportRunList)
    QML_UNCREATABLE("ImportRunList is exposed by the application context")
public:
    enum Roles {
        TimeRole = Qt::UserRole + 1,
        LogIdRole,
        TypeRole,
        FileRole,
        StatusRole,
        MessageRole,
        DraftAttachedRole,
        DraftIdRole,
        StatementIdRole,
        DisplayTimeRole,
        DisplayTitleRole,
        DisplayStatusDetailRole
    };

private:
    using Base = models::RowListModel<ImportRunRow>;

public:

    explicit ImportRunList(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addRun(QString time, QString type, QString file, QString status, QString message, bool draftAttached = false,
                QString statementId = {}, QString logId = {});
    bool upsertRun(const ImportRunRow& row);
    int findByLogId(const QString& logId) const;
    ImportRunRow at(int index) const;
    std::vector<ImportRunRow> snapshot() const;
    void setRuns(std::vector<ImportRunRow> runs);

    Q_INVOKABLE void removeAt(int index);
    Q_INVOKABLE void clear();
};

}
