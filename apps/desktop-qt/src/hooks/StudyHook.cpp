#include "hooks/StudyHook.h"

StudyHook::StudyHook(QObject *parent)
    : QObject(parent), m_workspaceTitle(QStringLiteral("未入席")) {}

QString StudyHook::workspaceTitle() const { return m_workspaceTitle; }
QString StudyHook::workspaceId() const { return m_workspaceId; }
QVariantList StudyHook::sessions() const { return m_sessions; }
QString StudyHook::selectedSessionId() const { return m_selectedSessionId; }
QString StudyHook::selectedTitle() const { return m_selectedTitle; }
QVariantList StudyHook::messages() const { return m_messages; }
bool StudyHook::sending() const { return m_sending; }
bool StudyHook::busy() const { return m_busy; }
QString StudyHook::noticeText() const { return m_noticeText; }

void StudyHook::setWorkspaceTitle(const QString &title) {
  if (m_workspaceTitle != title) {
    m_workspaceTitle = title;
    emit workspaceTitleChanged();
  }
}

void StudyHook::setWorkspaceId(const QString &id) {
  if (m_workspaceId != id) {
    m_workspaceId = id;
    emit workspaceIdChanged();
  }
}

void StudyHook::setSessions(const QVariantList &sessions) {
  if (m_sessions != sessions) {
    m_sessions = sessions;
    emit sessionsChanged();
  }
}

void StudyHook::setSelectedSessionId(const QString &id) {
  if (m_selectedSessionId != id) {
    m_selectedSessionId = id;
    emit selectedSessionIdChanged();
  }
}

void StudyHook::setSelectedTitle(const QString &title) {
  if (m_selectedTitle != title) {
    m_selectedTitle = title;
    emit selectedTitleChanged();
  }
}

void StudyHook::setMessages(const QVariantList &messages) {
  if (m_messages != messages) {
    m_messages = messages;
    emit messagesChanged();
  }
}

void StudyHook::setSending(bool sending) {
  if (m_sending != sending) {
    m_sending = sending;
    emit sendingChanged();
  }
}

void StudyHook::setBusy(bool busy) {
  if (m_busy != busy) {
    m_busy = busy;
    emit busyChanged();
  }
}

void StudyHook::setNoticeText(const QString &text) {
  if (m_noticeText != text) {
    m_noticeText = text;
    emit noticeTextChanged();
  }
}

void StudyHook::selectSession(const QString &sessionId) {
  if (sessionId.trimmed().isEmpty()) {
    return;
  }
  emit selectRequested(sessionId);
}

void StudyHook::createSession() { emit createRequested(); }

void StudyHook::sendPrompt(const QString &text) {
  const QString trimmed = text.trimmed();
  if (trimmed.isEmpty()) {
    return;
  }
  emit sendRequested(trimmed);
}

void StudyHook::refresh() { emit refreshRequested(); }
