#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class QAbstractListModel;
class SessionListModel;
class TranscriptModel;

class StudyHook : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString workspaceTitle READ workspaceTitle NOTIFY workspaceTitleChanged)
  Q_PROPERTY(QString workspaceId READ workspaceId NOTIFY workspaceIdChanged)
  Q_PROPERTY(QVariantList workspaces READ workspaces NOTIFY workspacesChanged)
  Q_PROPERTY(QAbstractListModel *sessions READ sessions CONSTANT)
  Q_PROPERTY(QAbstractListModel *transcript READ transcript CONSTANT)
  Q_PROPERTY(QString selectedSessionId READ selectedSessionId NOTIFY selectedSessionIdChanged)
  Q_PROPERTY(QString selectedTitle READ selectedTitle NOTIFY selectedTitleChanged)
  Q_PROPERTY(bool sending READ sending NOTIFY sendingChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
  Q_PROPERTY(bool streaming READ streaming NOTIFY streamingChanged)
  Q_PROPERTY(QString noticeText READ noticeText NOTIFY noticeTextChanged)
  Q_PROPERTY(QString modelLabel READ modelLabel NOTIFY modelLabelChanged)
  Q_PROPERTY(QVariantList modelOptions READ modelOptions NOTIFY modelOptionsChanged)
  Q_PROPERTY(bool modelsOpen READ modelsOpen NOTIFY modelsOpenChanged)
  Q_PROPERTY(QVariantMap pendingApproval READ pendingApproval NOTIFY pendingApprovalChanged)
  Q_PROPERTY(QVariantMap pendingQuestion READ pendingQuestion NOTIFY pendingQuestionChanged)
  Q_PROPERTY(bool settingsOpen READ settingsOpen NOTIFY settingsOpenChanged)
  Q_PROPERTY(bool settingsWritable READ settingsWritable NOTIFY settingsWritableChanged)
  Q_PROPERTY(bool settingsHasDocument READ settingsHasDocument NOTIFY settingsHasDocumentChanged)
  Q_PROPERTY(QVariantList settingsNamespaces READ settingsNamespaces NOTIFY settingsNamespacesChanged)
  Q_PROPERTY(bool streamOpen READ streamOpen NOTIFY streamOpenChanged)

 public:
  explicit StudyHook(QObject *parent = nullptr);

  QString workspaceTitle() const;
  QString workspaceId() const;
  QVariantList workspaces() const;
  QAbstractListModel *sessions() const;
  QAbstractListModel *transcript() const;
  SessionListModel *sessionList() const;
  TranscriptModel *transcriptModel() const;
  QString selectedSessionId() const;
  QString selectedTitle() const;
  bool sending() const;
  bool busy() const;
  bool streaming() const;
  QString noticeText() const;
  QString modelLabel() const;
  QVariantList modelOptions() const;
  bool modelsOpen() const;
  QVariantMap pendingApproval() const;
  QVariantMap pendingQuestion() const;
  bool settingsOpen() const;
  bool settingsWritable() const;
  bool settingsHasDocument() const;
  QVariantList settingsNamespaces() const;
  bool streamOpen() const;

  void setWorkspaceTitle(const QString &title);
  void setWorkspaceId(const QString &id);
  void setWorkspaces(const QVariantList &workspaces);
  void setSelectedSessionId(const QString &id);
  void setSelectedTitle(const QString &title);
  void setSending(bool sending);
  void setBusy(bool busy);
  void setStreaming(bool streaming);
  void setNoticeText(const QString &text);
  void setModelLabel(const QString &label);
  void setModelOptions(const QVariantList &options);
  void setModelsOpen(bool open);
  void setPendingApproval(const QVariantMap &approval);
  void setPendingQuestion(const QVariantMap &question);
  void setSettingsOpen(bool open);
  void setSettingsWritable(bool writable);
  void setSettingsHasDocument(bool hasDocument);
  void setSettingsNamespaces(const QVariantList &namespaces);
  void setStreamOpen(bool open);

  Q_INVOKABLE void selectSession(const QString &sessionId);
  Q_INVOKABLE void createSession();
  Q_INVOKABLE void sendPrompt(const QString &text);
  Q_INVOKABLE void refresh();
  Q_INVOKABLE void selectWorkspace(const QString &workspaceId);
  Q_INVOKABLE void selectModel(const QString &provider, const QString &model);
  Q_INVOKABLE void toggleModels();
  Q_INVOKABLE void cancelTurn();
  Q_INVOKABLE void answerApproval(const QString &outcome);
  Q_INVOKABLE void pickQuestionOption(const QString &label);
  Q_INVOKABLE void submitQuestionCustom(const QString &text);
  Q_INVOKABLE void openSettings();
  Q_INVOKABLE void closeSettings();
  Q_INVOKABLE void openSettingsDocument();
  Q_INVOKABLE void dismissNotice();

 signals:
  void workspaceTitleChanged();
  void workspaceIdChanged();
  void workspacesChanged();
  void selectedSessionIdChanged();
  void selectedTitleChanged();
  void sendingChanged();
  void busyChanged();
  void streamingChanged();
  void noticeTextChanged();
  void modelLabelChanged();
  void modelOptionsChanged();
  void modelsOpenChanged();
  void pendingApprovalChanged();
  void pendingQuestionChanged();
  void settingsOpenChanged();
  void settingsWritableChanged();
  void settingsHasDocumentChanged();
  void settingsNamespacesChanged();
  void streamOpenChanged();
  void selectRequested(const QString &sessionId);
  void createRequested();
  void sendRequested(const QString &text);
  void refreshRequested();
  void workspaceRequested(const QString &workspaceId);
  void modelRequested(const QString &provider, const QString &model);
  void cancelRequested();
  void approvalAnswerRequested(const QString &outcome);
  void questionOptionRequested(const QString &label);
  void questionCustomRequested(const QString &text);
  void settingsOpenRequested();
  void settingsDocumentRequested();

 private:
  SessionListModel *m_sessions = nullptr;
  TranscriptModel *m_transcript = nullptr;
  QString m_workspaceTitle;
  QString m_workspaceId;
  QVariantList m_workspaces;
  QString m_selectedSessionId;
  QString m_selectedTitle;
  bool m_sending = false;
  bool m_busy = false;
  bool m_streaming = false;
  QString m_noticeText;
  QString m_modelLabel;
  QVariantList m_modelOptions;
  bool m_modelsOpen = false;
  QVariantMap m_pendingApproval;
  QVariantMap m_pendingQuestion;
  bool m_settingsOpen = false;
  bool m_settingsWritable = false;
  bool m_settingsHasDocument = false;
  QVariantList m_settingsNamespaces;
  bool m_streamOpen = false;
};
