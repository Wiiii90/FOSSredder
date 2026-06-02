/**
 * @file ui/tests/qml/qmltests.cpp
 * @brief Boots the QML-near Quick Test suite for the UI module.
 */

#include <QtQuickTest/quicktest.h>

#include <QQmlEngine>
#include <QObject>
#include <QQuickStyle>
#include <QString>

#include "ui/shared/config/Defaults.h"
#include "ui/shell/QmlRuntime.h"

class UiQmlTestSetup : public QObject {
    Q_OBJECT
public slots:
    void applicationAvailable()
    {
        QQuickStyle::setStyle(ui::config::kQtStyle);
        ui::bootstrap::registerTypes();
    }

    void qmlEngineAvailable(QQmlEngine* engine)
    {
        ui::bootstrap::registerTypes();
        ui::bootstrap::configureRuntime(engine);
        engine->addImportPath(QStringLiteral(FOSSREDDER_UI_QML_IMPORT_DIR));
    }
};

QUICK_TEST_MAIN_WITH_SETUP(ui_qml_tests, UiQmlTestSetup)

#include "qmltests.moc"
