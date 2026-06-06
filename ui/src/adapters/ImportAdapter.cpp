/**
 * @file ui/src/adapters/ImportAdapter.cpp
 * @brief Implements the import adapter used by UI import workflows.
 */

#include "ui/adapters/ImportAdapter.h"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "ui/observability/Trace.h"
#include "ui/util/StringConversions.h"
#include "ui/workflows/ImportWorkflow.h"

namespace ui::adapters {

namespace core_importing = core::ports::importing::draft;

namespace {

QStringList toQStringList(const std::vector<std::string>& values) {
  QStringList out;
  out.reserve(static_cast<int>(values.size()));
  for (const auto& value : values) {
    out.push_back(QString::fromStdString(value));
  }
  return out;
}

QString proofImageDataUrl(const std::vector<std::uint8_t>& bytes) {
  if (bytes.empty()) {
    return {};
  }
  const QByteArray data(reinterpret_cast<const char*>(bytes.data()),
                        static_cast<int>(bytes.size()));
  return QString::fromLatin1(data.toBase64());
}

} // namespace

ImportAdapter::ImportAdapter(
    std::shared_ptr<core::ports::importing::IImportRunner> runner)
    : runner_(std::move(runner)) {}

core::ports::importing::StatementImportHandle
ImportAdapter::startStatementImport(
    const core::ports::importing::ImportRequest& request,
    core::ports::importing::StatementImportEventCallback callback) {
  observability::traceAdapter(
      "ImportAdapter::startStatementImport", "Import runner invoked",
      {{observability::context::kFile, request.sourcePath}});
  return runner_ ? runner_->startStatementImport(request, std::move(callback))
                 : core::ports::importing::StatementImportHandle{};
}

void ImportAdapter::unsubscribe(
    const core::ports::importing::StatementImportHandle& handle) {
  if (runner_)
    runner_->unsubscribe(handle);
}

void ImportAdapter::cancel(
    const core::ports::importing::StatementImportHandle& handle) {
  if (runner_)
    runner_->cancel(handle);
}

void ImportAdapter::pause(
    const core::ports::importing::StatementImportHandle& handle) {
  if (runner_)
    runner_->pause(handle);
}

void ImportAdapter::resume(
    const core::ports::importing::StatementImportHandle& handle) {
  if (runner_)
    runner_->resume(handle);
}

core::ports::importing::ImportResult ImportAdapter::importResult(
    const core::ports::importing::StatementImportHandle& handle) {
  return runner_ ? runner_->importResult(handle)
                 : core::ports::importing::ImportResult{};
}

core::ports::importing::draft::DraftDerivedState
ImportAdapter::buildDraftDerivedState(
    const core::ports::workspace::WorkspaceSnapshot& state,
    const core::ports::importing::draft::DraftLinkSelection& selection) const {
  return runner_ ? runner_->buildDraftDerivedState(state, selection)
                 : core::ports::importing::draft::DraftDerivedState{};
}

core::ports::workspace::WorkspaceSnapshot ImportAdapter::mergeWorkspaceState(
    core::ports::workspace::WorkspaceSnapshot primary,
    const core::ports::workspace::WorkspaceSnapshot& secondary) const {
  return runner_ ? runner_->mergeWorkspaceState(std::move(primary), secondary)
                 : std::move(primary);
}

TransactionDraftView ImportAdapter::transactionDraftView(
    const core_importing::TransactionDraft& draft,
    const core::ports::workspace::WorkspaceSnapshot& state) const {
  const auto derived = buildDraftDerivedState(state, toCoreSelection(draft));
  return toTransactionDraftView(draft, derived);
}

core_importing::DraftLinkSelection
ImportAdapter::toCoreSelection(const core_importing::TransactionDraft& draft) {
  core_importing::DraftLinkSelection out;
  out.name = draft.name;
  out.metadata = draft.metadata;
  out.actorText = draft.actorText;
  out.propertyText = draft.propertyText;
  out.actorId = draft.actorId;
  out.actorSelected = draft.actorSelected;
  out.contractId = draft.contractId;
  out.contractSelected = draft.contractSelected;
  out.type = draft.type;
  out.allocatable = draft.allocatable;
  out.allocatableSelected = draft.allocatableSelected;
  out.propertyIds = draft.propertyIds;
  return out;
}

bool ImportAdapter::updateTransactionDraft(
    core_importing::TransactionDraft& draft,
    const core::ports::workspace::WorkspaceSnapshot& state,
    const core_importing::TransactionDraftEdit& edit) const {
  return runner_ && runner_->updateTransactionDraft(draft, state, edit);
}

core_importing::StatementDraftEditResult ImportAdapter::updateStatementDraft(
    core_importing::StatementDraft& draft,
    const core_importing::StatementDraftEdit& edit) const {
  return runner_ ? runner_->updateStatementDraft(draft, edit)
                 : core_importing::StatementDraftEditResult{};
}

core_importing::StatementDraft ImportAdapter::buildStatementDraft(
    const std::string& sourceFile,
    const core::ports::workspace::StatementSnapshot& statement,
    const core::ports::workspace::WorkspaceSnapshot& state,
    const std::vector<core_importing::TransactionDraft>& transactions,
    const std::string& draftId) const {
  return runner_ ? runner_->buildStatementDraft(sourceFile, statement, state,
                                                transactions, draftId)
                 : core_importing::StatementDraft{};
}

core_importing::StatementDraft ImportAdapter::restoreStatementDraft(
    const core::ports::workspace::StatementDraftSnapshot& draft) const {
  return runner_ ? runner_->restoreStatementDraft(draft)
                 : core_importing::StatementDraft{};
}

core::ports::workspace::StatementDraftSnapshot
ImportAdapter::buildStatementDraftSnapshot(
    const core_importing::StatementDraft& draft,
    const core::ports::workspace::WorkspaceSnapshot& state) const {
  return runner_ ? runner_->buildStatementDraftSnapshot(draft, state)
                 : core::ports::workspace::StatementDraftSnapshot{};
}

TransactionDraftView ImportAdapter::toTransactionDraftView(
    const core_importing::TransactionDraft& draft,
    const core_importing::DraftDerivedState& derived) {
  TransactionDraftView view;
  view.id = QString::fromStdString(draft.id);
  view.name = QString::fromStdString(draft.name);
  view.bookingDate = QString::fromStdString(draft.bookingDate);
  view.valuta = QString::fromStdString(draft.valuta);
  view.amountText = QVariant(draft.amount).toString();
  view.metadata = QString::fromStdString(draft.metadata);
  const QString proofData = proofImageDataUrl(draft.proofImageData);
  view.proofSource =
      proofData.isEmpty()
          ? QString{}
          : QStringLiteral("data:image/jpeg;base64,") + proofData;
  view.actorId = QString::fromStdString(draft.actorId);
  view.contractId = QString::fromStdString(draft.contractId);
  view.propertyIds = toQStringList(draft.propertyIds);
  view.status = draft.status;
  view.effectiveAllocatable = draft.allocatableSelected
                                  ? draft.allocatable
                                  : derived.effectiveAllocatable;
  view.actorSuggestionConfidence = derived.actorSuggestionConfidence;
  view.actorSuggestionSummary =
      QString::fromStdString(derived.actorSuggestionSummary);
  view.propertySuggestionConfidence = derived.propertySuggestionConfidence;
  view.propertySuggestionSummary =
      QString::fromStdString(derived.propertySuggestionSummary);
  view.contractSuggestionConfidence = derived.contractSuggestionConfidence;
  view.contractSuggestionSummary =
      QString::fromStdString(derived.contractSuggestionSummary);
  view.allocatableSuggestionConfidence =
      derived.allocatableSuggestionConfidence;
  view.allocatableSuggestionSummary =
      QString::fromStdString(derived.allocatableSuggestionSummary);
  return view;
}

} // namespace ui::adapters
