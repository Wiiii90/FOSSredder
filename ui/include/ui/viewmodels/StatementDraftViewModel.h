/**
 * @file ui/include/ui/viewmodels/StatementDraftViewModel.h
 * @brief Declares statement draft review state used by the import view.
 */

#pragma once

#include <QObject>
#include <QString>
#include <qqmlintegration.h>

namespace ui {

class ImportWorkflow;
class WorkspaceFacade;

/**
 * @brief QML API for reviewing an imported statement draft.
 */
class StatementDraftViewModel : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(StatementDraftViewModel)

  Q_PROPERTY(ui::ImportWorkflow *importWorkflow READ importWorkflow WRITE
                 setImportWorkflow NOTIFY changed)
  Q_PROPERTY(ui::WorkspaceFacade *workspace READ workspace WRITE setWorkspace
                 NOTIFY changed)
  Q_PROPERTY(bool hasDraft READ hasDraft NOTIFY changed)
  Q_PROPERTY(QString statementName READ statementName WRITE setStatementName
                 NOTIFY changed)
  Q_PROPERTY(int currentTransactionNumber READ currentTransactionNumber NOTIFY
                 changed)
  Q_PROPERTY(int transactionCount READ transactionCount NOTIFY changed)
  Q_PROPERTY(bool canDeleteTransaction READ canDeleteTransaction NOTIFY changed)
  Q_PROPERTY(bool canSelectPreviousTransactionDraft READ
                 canSelectPreviousTransactionDraft NOTIFY changed)
  Q_PROPERTY(bool canSelectNextTransactionDraft READ
                 canSelectNextTransactionDraft NOTIFY changed)

public:
  explicit StatementDraftViewModel(QObject *parent = nullptr);

  ImportWorkflow *importWorkflow() const noexcept { return importWorkflow_; }
  WorkspaceFacade *workspace() const noexcept { return workspace_; }
  void setImportWorkflow(ImportWorkflow *value);
  void setWorkspace(WorkspaceFacade *value);

  bool hasDraft() const noexcept;
  QString statementName() const;
  void setStatementName(const QString &value);
  int currentTransactionNumber() const noexcept;
  int transactionCount() const noexcept;
  bool canDeleteTransaction() const noexcept;
  bool canSelectPreviousTransactionDraft() const noexcept;
  bool canSelectNextTransactionDraft() const noexcept;

  Q_INVOKABLE void returnToImport();
  Q_INVOKABLE void discard();
  Q_INVOKABLE void addTransactionAfterCurrent();
  Q_INVOKABLE void deleteCurrentTransaction();
  Q_INVOKABLE void finalize();
  Q_INVOKABLE void selectPreviousTransactionDraft();
  Q_INVOKABLE void selectNextTransactionDraft();

signals:
  void changed();

private:
  ImportWorkflow *importWorkflow_ = nullptr;
  WorkspaceFacade *workspace_ = nullptr;
};

} // namespace ui
