#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class StudyHook : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString workspaceTitle READ workspaceTitle NOTIFY workspaceTitleChanged)
  Q_PROPERTY(QString workspaceId READ workspaceId NOTIFY workspaceIdChanged)
  Q_PROPERTY(QVariantList sessions READ sessions NOTIFY sessionsChanged)
  Q_PROPERTY(QString selectedSessionId READ selectedSessionId NOTIFY selectedSessionIdChanged)
  Q_PROPERTY(QString selectedTitle READ selectedTitle NOTIFY selectedTitleChanged)
  Q_PROPERTY(QVariantList messages READ messages NOTIFY messagesChanged)
  Q_PROPERTY(bool sending READ sending NOTIFY sendingChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
  Q_PROPERTY(QString noticeText READ noticeText NOTIFY noticeTextChanged)

 public:
  explicit StudyHook(QObject *parent = nullptr);

  QString workspaceTitle() const;
  QString workspaceId() const;
  QVariantList sessions() const;
  QString selectedSessionId() const;
  QString selectedTitle() const;
  QVariantList messages() const;
  bool sending() const;
  bool busy() const;
  QString noticeText() const;

  void setWorkspaceTitle(const QString &title);
  void setWorkspaceId(const QString &id);
  void setSessions(const QVariantList &sessions);
  void setSelectedSessionId(const QString &id);
  void setSelectedTitle(const QString &title);
  void setMessages(const QVariantList &messages);
  void setSending(bool sending);
  void setBusy(bool busy);
  void setNoticeText(const QString &text);

  Q_INVOKABLE void selectSession(const QString &sessionId);
  Q_INVOKABLE void createSession();
  Q_INVOKABLE void sendPrompt(const QString &text);
  Q_INVOKABLE void refresh();

 signals:
  void workspaceTitleChanged();
  void workspaceIdChanged();
  void sessionsChanged();
  void selectedSessionIdChanged();
  void selectedTitleChanged();
  void messagesChanged();
  void sendingChanged();
  void busyChanged();
  void noticeTextChanged();
  void selectRequested(const QString &sessionId);
  void createRequested();
  void sendRequested(const QString &text);
  void refreshRequested();

 private:
  QString m_workspaceTitle;
  QString m_workspaceId;
  QVariantList m_sessions;
  QString m_selectedSessionId;
  QString m_selectedTitle;
  QVariantList m_messages;
  bool m_sending = false;
  bool m_busy = false;
  QString m_noticeText;
};
