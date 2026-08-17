#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QVariantMap>

class HostProcess;
class RpcClient;
class EventStream;
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
  void onWorkspaceRequested(const QString &workspaceId);
  void onModelRequested(const QString &provider, const QString &model);
  void onCancelRequested();
  void onApprovalAnswerRequested(const QString &outcome);
  void onQuestionOptionRequested(const QString &label);
  void onQuestionCustomRequested(const QString &text);
  void onSettingsOpenRequested();
  void onSettingsDocumentRequested();
  void onMuxFrame(const QString &rpcId, const QJsonObject &payload);
  void onHostFrame(const QString &rpcId, const QJsonObject &payload);
  void onStreamOpenChanged(bool open);
  void flushStreamDelta();

 private:
  void applyStudyLists(const QJsonObject &workspaceList, const QJsonObject &sessionList);
  void loadHistory(const QString &sessionId);
  void loadModels(const QString &sessionId);
  void connectStreams();
  void applySelectedSession(const QString &sessionId);
  QString titleForSession(const QString &sessionId) const;
  void publishQuestion();
  void submitQuestionBatch();
  void appendQuestionAnswer(const QJsonObject &answer);

  HostProcess *m_hostProcess = nullptr;
  RpcClient *m_rpcClient = nullptr;
  EventStream *m_eventStream = nullptr;
  ConnectionHook *m_connectionHook = nullptr;
  StudyHook *m_studyHook = nullptr;
  WindowController *m_windowController = nullptr;
  QTimer *m_streamFlush = nullptr;

  quint16 m_port = 0;
  int m_retryCount = 0;
  bool m_isStopping = false;
  int m_studyGeneration = 0;
  int m_historyGeneration = 0;
  QJsonObject m_workspaceList;
  QJsonObject m_sessionList;
  QString m_pendingDelta;
  QJsonArray m_questionItems;
  QJsonArray m_questionAnswers;
  int m_questionIndex = 0;
  QString m_questionRpcId;
  QString m_questionSessionId;
};
