#pragma once

#include <QObject>
#include <QTimer>
#include <QUrl>

class HostProcess;
class RpcClient;
class ConnectionHook;
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

 private:
  HostProcess *m_hostProcess = nullptr;
  RpcClient *m_rpcClient = nullptr;
  ConnectionHook *m_connectionHook = nullptr;
  WindowController *m_windowController = nullptr;

  quint16 m_port = 0;
  int m_retryCount = 0;
  bool m_isStopping = false;
};
