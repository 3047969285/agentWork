#pragma once

#include <QQmlApplicationEngine>
#include <QObject>

class ConnectionHook;
class StudyHook;

class WindowController : public QObject {
  Q_OBJECT

 public:
  explicit WindowController(QObject *parent = nullptr);
  ~WindowController() override;

  bool init(ConnectionHook *connectionHook, StudyHook *studyHook);
  QQmlApplicationEngine *engine() const;

 private:
  QQmlApplicationEngine *m_engine = nullptr;
};
