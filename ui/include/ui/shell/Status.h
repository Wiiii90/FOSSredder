/**
 * @file ui/include/ui/shell/Status.h
 * @brief Declarations for the UI Status component.
 */

#pragma once

#include <QObject>
#include <QString>

namespace ui {

/**
 * @brief Exposes the shell status text shown by the main window.
 */
class Status : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
  /**
   * @brief Creates the status state object.
   * @param parent Optional QObject parent.
   */
  explicit Status(QObject* parent = nullptr) : QObject(parent) {}

  /**
   * @brief Returns the current status text.
   * @return Current shell status text.
   */
  QString text() const {
    return text_;
  }

  /**
   * @brief Updates the current status text.
   * @param value New status text.
   */
  void setText(const QString& value) {
    if (text_ == value)
      return;
    text_ = value;
    emit textChanged();
  }

signals:
  /**
   * @brief Emitted when the status text changes.
   */
  void textChanged();

private:
  QString text_;
};

} // namespace ui
