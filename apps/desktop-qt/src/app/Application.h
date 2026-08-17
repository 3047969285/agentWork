#pragma once

#include <QJsonObject>
#include <QObject>
#include <QTimer>
#include <QUrl>

class HostProcess;
class RpcClient;
class ConnectionHook;
class StudyHook;
class WindowController;

class Application : public QObject {
  Q_OBJECT

 public:
  explicit Application(QObject *parent = nullptr);
  ~Application() override;

  bool init();
  void start();
  void stop();

 private slots:
  void onHostStarted();
  void onHostReady(quint16 port);
  void onHostStopped();
  void onHostError(const QString &error);
  void onRetryRequested();
  void doHandshake();
  void scheduleRetry(const QString &reason);
  void loadStudy();
  void onSelectRequested(const QString &sessionId);
  void onCreateRequested();
  void onSendRequested(const QString &text);
  void onRefreshRequested();

 private:
  void applyStudyLists(const QJsonObject &workspaceList, const QJsonObject &sessionList);
  void loadHistory(const QString &sessionId);
  void beginHistoryPoll(const QString &sessionId);
  void pollHistoryTick(const QString &sessionId);
  QString titleForSession(const QString &sessionId) const;

  HostProcess *m_hostProcess = nullptr;
  RpcClient *m_rpcClient = nullptr;
  ConnectionHook *m_connectionHook = nullptr;
  StudyHook *m_studyHook = nullptr;
  WindowController *m_windowController = nullptr;

  quint16 m_port = 0;
  int m_retryCount = 0;
  bool m_isStopping = false;
  int m_studyGeneration = 0;
  int m_historyGeneration = 0;
  int m_pollRemaining = 0;
  int m_pollAfterSeq = 0;
};
