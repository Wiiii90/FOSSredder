/**
 * @file ui/include/ui/state/shell/ShellNavigationState.h
 * @brief Declares the shell-level navigation state used by layout QML.
 */

#pragma once

#include <QObject>
#include <qqmlintegration.h>

namespace ui {

class ImportWorkflow;
class NavigationState;
class WorkspaceFacade;

/**
 * @brief Owns shell navigation side effects and lazy-loaded section flags.
 */
class ShellNavigationState : public QObject {
  Q_OBJECT
  QML_NAMED_ELEMENT(ShellNavigationState)

  Q_PROPERTY(ui::NavigationState *navigation READ navigation WRITE setNavigation
                 NOTIFY navigationChanged)
  Q_PROPERTY(ui::WorkspaceFacade *workspace READ workspace WRITE setWorkspace
                 NOTIFY workspaceChanged)
  Q_PROPERTY(ui::ImportWorkflow *importWorkflow READ importWorkflow WRITE
                 setImportWorkflow NOTIFY importWorkflowChanged)

  Q_PROPERTY(int activeSection READ activeSection NOTIFY activeSectionChanged)
  Q_PROPERTY(bool actorLoaded READ actorLoaded NOTIFY loadedSectionsChanged)
  Q_PROPERTY(bool propertyLoaded READ propertyLoaded NOTIFY loadedSectionsChanged)
  Q_PROPERTY(bool contractLoaded READ contractLoaded NOTIFY loadedSectionsChanged)
  Q_PROPERTY(bool bookingLoaded READ bookingLoaded NOTIFY loadedSectionsChanged)
  Q_PROPERTY(bool importLoaded READ importLoaded NOTIFY loadedSectionsChanged)
  Q_PROPERTY(bool exportLoaded READ exportLoaded NOTIFY loadedSectionsChanged)
  Q_PROPERTY(bool settingsLoaded READ settingsLoaded NOTIFY loadedSectionsChanged)
  Q_PROPERTY(bool analysisLoaded READ analysisLoaded NOTIFY loadedSectionsChanged)
  Q_PROPERTY(bool annualLoaded READ annualLoaded NOTIFY loadedSectionsChanged)
  Q_PROPERTY(bool placeholderLoaded READ placeholderLoaded NOTIFY loadedSectionsChanged)

  Q_PROPERTY(int actorSection READ actorSection CONSTANT)
  Q_PROPERTY(int propertySection READ propertySection CONSTANT)
  Q_PROPERTY(int contractSection READ contractSection CONSTANT)
  Q_PROPERTY(int bookingSection READ bookingSection CONSTANT)
  Q_PROPERTY(int importSection READ importSection CONSTANT)
  Q_PROPERTY(int exportSection READ exportSection CONSTANT)
  Q_PROPERTY(int settingsSection READ settingsSection CONSTANT)
  Q_PROPERTY(int analysisSection READ analysisSection CONSTANT)
  Q_PROPERTY(int annualSection READ annualSection CONSTANT)

public:
  explicit ShellNavigationState(QObject *parent = nullptr);

  NavigationState *navigation() const noexcept { return navigation_; }
  WorkspaceFacade *workspace() const noexcept { return workspace_; }
  ImportWorkflow *importWorkflow() const noexcept { return importWorkflow_; }

  void setNavigation(NavigationState *navigation);
  void setWorkspace(WorkspaceFacade *workspace);
  void setImportWorkflow(ImportWorkflow *importWorkflow);

  int activeSection() const noexcept { return activeSection_; }
  bool actorLoaded() const noexcept { return actorLoaded_; }
  bool propertyLoaded() const noexcept { return propertyLoaded_; }
  bool contractLoaded() const noexcept { return contractLoaded_; }
  bool bookingLoaded() const noexcept { return bookingLoaded_; }
  bool importLoaded() const noexcept { return importLoaded_; }
  bool exportLoaded() const noexcept { return exportLoaded_; }
  bool settingsLoaded() const noexcept { return settingsLoaded_; }
  bool analysisLoaded() const noexcept { return analysisLoaded_; }
  bool annualLoaded() const noexcept { return annualLoaded_; }
  bool placeholderLoaded() const noexcept { return placeholderLoaded_; }

  int actorSection() const noexcept;
  int propertySection() const noexcept;
  int contractSection() const noexcept;
  int bookingSection() const noexcept;
  int importSection() const noexcept;
  int exportSection() const noexcept;
  int settingsSection() const noexcept;
  int analysisSection() const noexcept;
  int annualSection() const noexcept;

  Q_INVOKABLE void activate();
  Q_INVOKABLE void navigateToSection(int section,
                                     bool resetWorkspaceSelection = false);
  Q_INVOKABLE void navigateToImportHome();
  Q_INVOKABLE void navigateToBookingCreate();
  Q_INVOKABLE bool isKnownSection(int section) const noexcept;

signals:
  void navigationChanged();
  void workspaceChanged();
  void importWorkflowChanged();
  void activeSectionChanged();
  void loadedSectionsChanged();

private:
  void syncFromNavigation();
  void setActiveSection(int section);
  void rememberSection(int section);
  void clearWorkspaceSelection();
  void clearBookingSelection();

  NavigationState *navigation_ = nullptr;
  WorkspaceFacade *workspace_ = nullptr;
  ImportWorkflow *importWorkflow_ = nullptr;
  int activeSection_ = 0;
  bool actorLoaded_ = false;
  bool propertyLoaded_ = false;
  bool contractLoaded_ = false;
  bool bookingLoaded_ = false;
  bool importLoaded_ = false;
  bool exportLoaded_ = false;
  bool settingsLoaded_ = false;
  bool analysisLoaded_ = false;
  bool annualLoaded_ = false;
  bool placeholderLoaded_ = false;
};

} // namespace ui
