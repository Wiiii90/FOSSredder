/**
 * @file ui/include/ui/viewmodels/TransactionDraftViewModel.h
 * @brief Declares transaction draft UI state used by the import review flow.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlintegration.h>

#include "ui/adapters/ImportAdapter.h"
#include "ui/shared/payload/PayloadMapper.h"

namespace ui {

class ImportWorkflow;
class WorkspaceFacade;

/**
 * @brief QML API for reviewing and editing a single imported transaction draft.
 */
class TransactionDraftViewModel : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(TransactionDraftViewModel)

  Q_PROPERTY(ui::ImportWorkflow *importWorkflow READ importWorkflow WRITE
                 setImportWorkflow NOTIFY changed)
  Q_PROPERTY(ui::WorkspaceFacade *workspace READ workspace WRITE setWorkspace
                 NOTIFY changed)
  Q_PROPERTY(QString nameText READ nameText WRITE setNameText NOTIFY changed)
  Q_PROPERTY(QString bookingDateText READ bookingDateText WRITE
                 setBookingDateText NOTIFY changed)
  Q_PROPERTY(
      QString valutaText READ valutaText WRITE setValutaText NOTIFY changed)
  Q_PROPERTY(
      QString amountText READ amountText WRITE setAmountText NOTIFY changed)
  Q_PROPERTY(QVariantList statusOptions READ statusOptions NOTIFY changed)
  Q_PROPERTY(int statusIndex READ statusIndex NOTIFY changed)
  Q_PROPERTY(QString metadataText READ metadataText NOTIFY changed)
  Q_PROPERTY(QString proofSource READ proofSource NOTIFY changed)
  Q_PROPERTY(bool effectiveAllocatable READ effectiveAllocatable NOTIFY changed)
  Q_PROPERTY(QVariantList actorOptions READ actorOptions NOTIFY changed)
  Q_PROPERTY(int selectedActorOptionIndex READ selectedActorOptionIndex NOTIFY
                 changed)
  Q_PROPERTY(QString actorName READ actorName WRITE setActorName NOTIFY changed)
  Q_PROPERTY(bool canAddActor READ canAddActor NOTIFY changed)
  Q_PROPERTY(QVariantList contractOptions READ contractOptions NOTIFY changed)
  Q_PROPERTY(
      int selectedContractOptionIndex READ selectedContractOptionIndex NOTIFY changed)
  Q_PROPERTY(QString contractName READ contractName WRITE setContractName NOTIFY
                 changed)
  Q_PROPERTY(QString contractType READ contractType WRITE setContractType NOTIFY
                 changed)
  Q_PROPERTY(QString contractNamePlaceholder READ contractNamePlaceholder NOTIFY
                 changed)
  Q_PROPERTY(QVariantList contractAllocatableModes READ contractAllocatableModes
                 CONSTANT)
  Q_PROPERTY(int contractAllocatableModeIndex READ contractAllocatableModeIndex
                 WRITE setContractAllocatableModeIndex NOTIFY changed)
  Q_PROPERTY(
      QString selectedContractType READ selectedContractType NOTIFY changed)
  Q_PROPERTY(bool canAddContract READ canAddContract NOTIFY changed)
  Q_PROPERTY(QVariantList propertyOptions READ propertyOptions NOTIFY changed)
  Q_PROPERTY(QString propertyName READ propertyName WRITE setPropertyName NOTIFY
                 changed)
  Q_PROPERTY(bool canAddProperty READ canAddProperty NOTIFY changed)
  Q_PROPERTY(double actorSuggestionConfidence READ actorSuggestionConfidence
                 NOTIFY changed)
  Q_PROPERTY(
      QString actorSuggestionSummary READ actorSuggestionSummary NOTIFY changed)
  Q_PROPERTY(double propertySuggestionConfidence READ
                 propertySuggestionConfidence NOTIFY changed)
  Q_PROPERTY(QString propertySuggestionSummary READ propertySuggestionSummary
                 NOTIFY changed)
  Q_PROPERTY(double contractSuggestionConfidence READ
                 contractSuggestionConfidence NOTIFY changed)
  Q_PROPERTY(QString contractSuggestionSummary READ contractSuggestionSummary
                 NOTIFY changed)
  Q_PROPERTY(double allocatableSuggestionConfidence READ
                 allocatableSuggestionConfidence NOTIFY changed)
  Q_PROPERTY(QString allocatableSuggestionText READ allocatableSuggestionText
                 NOTIFY changed)

public:
  explicit TransactionDraftViewModel(QObject *parent = nullptr);

  ImportWorkflow *importWorkflow() const noexcept { return importWorkflow_; }
  WorkspaceFacade *workspace() const noexcept { return workspace_; }
  void setImportWorkflow(ImportWorkflow *value);
  void setWorkspace(WorkspaceFacade *value);

  QString nameText() const { return nameText_; }
  void setNameText(const QString &value);
  QString bookingDateText() const { return bookingDateText_; }
  void setBookingDateText(const QString &value);
  QString valutaText() const { return valutaText_; }
  void setValutaText(const QString &value);
  QString amountText() const { return amountText_; }
  void setAmountText(const QString &value);

  QVariantList statusOptions() const;
  int statusIndex() const;
  QString metadataText() const;
  QString proofSource() const;
  bool effectiveAllocatable() const;
  QVariantList actorOptions() const;
  int selectedActorOptionIndex() const;
  QString actorName() const { return actorName_; }
  void setActorName(const QString &value);
  bool canAddActor() const;
  QVariantList contractOptions() const;
  int selectedContractOptionIndex() const;
  QString contractName() const { return contractName_; }
  void setContractName(const QString &value);
  QString contractType() const { return contractType_; }
  void setContractType(const QString &value);
  QString contractNamePlaceholder() const;
  QVariantList contractAllocatableModes() const;
  int contractAllocatableModeIndex() const;
  void setContractAllocatableModeIndex(int index);
  QString selectedContractType() const;
  bool canAddContract() const;
  QVariantList propertyOptions() const;
  QString propertyName() const { return propertyName_; }
  void setPropertyName(const QString &value);
  bool canAddProperty() const;

  Q_INVOKABLE void commitNameText();
  Q_INVOKABLE void commitBookingDateText();
  Q_INVOKABLE void commitValutaText();
  Q_INVOKABLE void commitAmountText();
  Q_INVOKABLE void selectStatusAtIndex(int index);
  Q_INVOKABLE int suggestionTone(double confidence) const;
  Q_INVOKABLE void selectActorAtIndex(int index);
  Q_INVOKABLE void addActor();
  Q_INVOKABLE void selectContractAtIndex(int index);
  Q_INVOKABLE void addContract();
  Q_INVOKABLE bool isPropertySelected(const QString &propertyId) const;
  Q_INVOKABLE void setPropertySelected(const QString &propertyId,
                                       bool selected);
  Q_INVOKABLE void addProperty();
  Q_INVOKABLE void toggleAllocatable();

  double actorSuggestionConfidence() const;
  QString actorSuggestionSummary() const;
  double propertySuggestionConfidence() const;
  QString propertySuggestionSummary() const;
  double contractSuggestionConfidence() const;
  QString contractSuggestionSummary() const;
  double allocatableSuggestionConfidence() const;
  QString allocatableSuggestionText() const;

signals:
  void changed();

private:
  void bindWorkspace(WorkspaceFacade *value);
  void handleWorkflowStateChanged();
  void resetFormFieldsFromCurrentTransaction();
  void loadCurrentTransactionView();
  void notifyCurrentTransactionViewChanged();
  QString suggestionText(const QString &value) const;
  QString currentTransactionId() const;
  QStringList currentTransactionPropertyIds() const;
  QString currentTransactionActorId() const;

  ImportWorkflow *importWorkflow_ = nullptr;
  WorkspaceFacade *workspace_ = nullptr;
  adapters::TransactionDraftView currentView_;
  QString lastTransactionId_;
  QString nameText_;
  QString bookingDateText_;
  QString valutaText_;
  QString amountText_;
  QString actorName_;
  QString contractName_;
  QString contractType_;
  QString contractAllocatableMode_ = payload::contract_allocatable_mode::kMixed;
  QString propertyName_;
};

} // namespace ui
