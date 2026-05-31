/**
 * @file ui/src/state/shell/ShellNavigationState.cpp
 * @brief Implements the shell-level navigation state used by layout QML.
 */

#include "ui/state/shell/ShellNavigationState.h"

#include "ui/state/navigation/NavigationState.h"
#include "ui/workflows/import/ImportWorkflow.h"
#include "ui/workspace/WorkspaceFacade.h"

namespace ui {

namespace {

constexpr int sectionValue(NavigationState::Section section) noexcept
{
  return static_cast<int>(section);
}

} // namespace

ShellNavigationState::ShellNavigationState(QObject *parent) : QObject(parent)
{
  activeSection_ = importSection();
  rememberSection(activeSection_);
}

void ShellNavigationState::setNavigation(NavigationState *navigation)
{
  if (navigation_ == navigation) {
    return;
  }

  if (navigation_) {
    disconnect(navigation_, nullptr, this, nullptr);
  }

  navigation_ = navigation;
  if (navigation_) {
    connect(navigation_, &NavigationState::sectionChanged, this,
            &ShellNavigationState::syncFromNavigation);
  }

  emit navigationChanged();
  syncFromNavigation();
}

void ShellNavigationState::setWorkspace(WorkspaceFacade *workspace)
{
  if (workspace_ == workspace) {
    return;
  }

  workspace_ = workspace;
  emit workspaceChanged();
}

void ShellNavigationState::setImportWorkflow(ImportWorkflow *importWorkflow)
{
  if (importWorkflow_ == importWorkflow) {
    return;
  }

  importWorkflow_ = importWorkflow;
  emit importWorkflowChanged();
}

int ShellNavigationState::actorSection() const noexcept
{
  return sectionValue(NavigationState::Section::Actors);
}

int ShellNavigationState::propertySection() const noexcept
{
  return sectionValue(NavigationState::Section::Properties);
}

int ShellNavigationState::contractSection() const noexcept
{
  return sectionValue(NavigationState::Section::Contracts);
}

int ShellNavigationState::bookingSection() const noexcept
{
  return sectionValue(NavigationState::Section::Booking);
}

int ShellNavigationState::importSection() const noexcept
{
  return sectionValue(NavigationState::Section::Import);
}

int ShellNavigationState::exportSection() const noexcept
{
  return sectionValue(NavigationState::Section::Export);
}

int ShellNavigationState::settingsSection() const noexcept
{
  return sectionValue(NavigationState::Section::Settings);
}

int ShellNavigationState::analysisSection() const noexcept
{
  return sectionValue(NavigationState::Section::Analysis);
}

int ShellNavigationState::annualSection() const noexcept
{
  return sectionValue(NavigationState::Section::Annual);
}

void ShellNavigationState::activate()
{
  syncFromNavigation();
}

void ShellNavigationState::navigateToSection(int section,
                                             bool resetWorkspaceSelection)
{
  if (resetWorkspaceSelection) {
    clearWorkspaceSelection();
  }

  if (section != bookingSection()) {
    clearBookingSelection();
  }

  if (navigation_) {
    navigation_->setSectionValue(section);
  }
  setActiveSection(section);
  rememberSection(section);
}

void ShellNavigationState::navigateToImportHome()
{
  if (importWorkflow_) {
    importWorkflow_->clearDraft();
  }
  navigateToSection(importSection());
}

void ShellNavigationState::navigateToBookingCreate()
{
  clearBookingSelection();
  navigateToSection(bookingSection());
}

bool ShellNavigationState::isKnownSection(int section) const noexcept
{
  return section == actorSection() || section == propertySection() ||
         section == contractSection() || section == bookingSection() ||
         section == importSection() || section == exportSection() ||
         section == settingsSection() || section == analysisSection() ||
         section == annualSection();
}

void ShellNavigationState::syncFromNavigation()
{
  const int section = navigation_ ? navigation_->sectionValue() : importSection();
  if (section != bookingSection()) {
    clearBookingSelection();
  }
  setActiveSection(section);
  rememberSection(section);
}

void ShellNavigationState::setActiveSection(int section)
{
  if (activeSection_ == section) {
    return;
  }

  activeSection_ = section;
  emit activeSectionChanged();
}

void ShellNavigationState::rememberSection(int section)
{
  bool *loaded = nullptr;
  if (section == actorSection()) {
    loaded = &actorLoaded_;
  } else if (section == propertySection()) {
    loaded = &propertyLoaded_;
  } else if (section == contractSection()) {
    loaded = &contractLoaded_;
  } else if (section == bookingSection()) {
    loaded = &bookingLoaded_;
  } else if (section == importSection()) {
    loaded = &importLoaded_;
  } else if (section == exportSection()) {
    loaded = &exportLoaded_;
  } else if (section == settingsSection()) {
    loaded = &settingsLoaded_;
  } else if (section == analysisSection()) {
    loaded = &analysisLoaded_;
  } else if (section == annualSection()) {
    loaded = &annualLoaded_;
  } else {
    loaded = &placeholderLoaded_;
  }

  if (*loaded) {
    return;
  }
  *loaded = true;
  emit loadedSectionsChanged();
}

void ShellNavigationState::clearWorkspaceSelection()
{
  if (!workspace_) {
    return;
  }

  workspace_->setSelectedActorId({});
  workspace_->setSelectedPropertyId({});
  workspace_->setSelectedContractId({});
  workspace_->setSelectedAnalysisId({});
  workspace_->setSelectedAnnualId({});
  clearBookingSelection();
}

void ShellNavigationState::clearBookingSelection()
{
  if (!workspace_) {
    return;
  }

  workspace_->setSelectedStatementId({});
  workspace_->setSelectedTransactionId({});
}

} // namespace ui
