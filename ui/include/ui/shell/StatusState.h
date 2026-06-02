/**
 * @file ui/include/ui/shell/StatusState.h
 * @brief Declarations for the UI StatusState component.
 */

#pragma once

#include <QObject>
#include <QString>

namespace ui {

class StatusState : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
  explicit StatusState(QObject *parent = nullptr) : QObject(parent) {}

  QString text() const { return text_; }

  void setText(const QString &value) {
    if (text_ == value)
      return;
    text_ = value;
    emit textChanged();
  }

signals:
  void textChanged();

private:
  QString text_;
};

} // namespace ui
