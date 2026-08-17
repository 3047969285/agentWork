#pragma once

#include <QObject>
#include <QQmlApplicationEngine>

class ConnectionHook;

class WindowController : public QObject {
  Q_OBJECT

 public:
  explicit WindowController(QObject *parent = nullptr);
  ~WindowController() override;

  bool init(ConnectionHook *connectionHook);
  QQmlApplicationEngine *engine() const;

 private:
  QQmlApplicationEngine *m_engine = nullptr;
};
