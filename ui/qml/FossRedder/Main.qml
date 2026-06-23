/**
 * @file ui/qml/FossRedder/Main.qml
 * @brief Bootstraps the QML shell with shared app context and theme singletons.
 */

import QtQuick 2.15
import FossRedder 1.0
import FossRedder.Components 1.0 as Components

Components.Shell {
    // qmllint disable unqualified
    appContext: AppContext
    theme: Theme
    // qmllint enable unqualified
}

