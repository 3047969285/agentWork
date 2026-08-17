#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QVariant>
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
  void onSettingsUpdateRequested(const QString &ns, const QString &key, const QString &kind, const QVariant &value);
  void onCredentialSetRequested(const QString &ref, const QString &value);
  void onPermissionRequested(const QString &preset);
  void onPlanToggleRequested();
  void onPresetRequested(const QString &id);
  void onAttachRequested(const QUrl &url);
  void onAttachmentRemoveRequested(int index);
  void onSlashPicked(const QString &line);
  void onSubagentInterruptRequested(const QString &childId);
  void onOnboardingKeyRequested(const QString &key);
  void onMuxFrame(const QString &rpcId, const QJsonObject &payload);
  void onHostFrame(const QString &rpcId, const QJsonObject &payload);
  void onStreamOpenChanged(bool open);
  void flushStreamDelta();

 private:
  void applyStudyLists(const QJsonObject &workspaceList, const QJsonObject &sessionList);
  void loadHistory(const QString &sessionId);
  void loadModels(const QString &sessionId);
  void loadHostCatalog();
  void loadSessionExtras(const QString &sessionId);
  void refreshSlashItems();
  void applyProjection(const QString &key, const QJsonValue &value);
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
  int m_catalogGeneration = 0;
  QJsonObject m_workspaceList;
  QJsonObject m_sessionList;
  QJsonObject m_settingsDescribe;
  QJsonValue m_permissionProjection;
  QVariantMap m_imageLimits;
  QString m_pendingDelta;
  QJsonArray m_questionItems;
  QJsonArray m_questionAnswers;
  int m_questionIndex = 0;
  QString m_questionRpcId;
  QString m_questionSessionId;
};
