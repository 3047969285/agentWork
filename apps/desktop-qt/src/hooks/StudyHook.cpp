#include "hooks/StudyHook.h"

#include "models/SessionListModel.h"
#include "models/TranscriptModel.h"

StudyHook::StudyHook(QObject *parent)
    : QObject(parent),
      m_sessions(new SessionListModel(this)),
      m_transcript(new TranscriptModel(this)),
      m_workspaceTitle(QStringLiteral("未入席")),
      m_modelLabel(QStringLiteral("模型")) {}

QString StudyHook::workspaceTitle() const { return m_workspaceTitle; }
QString StudyHook::workspaceId() const { return m_workspaceId; }
QVariantList StudyHook::workspaces() const { return m_workspaces; }
QAbstractListModel *StudyHook::sessions() const { return m_sessions; }
QAbstractListModel *StudyHook::transcript() const { return m_transcript; }
SessionListModel *StudyHook::sessionList() const { return m_sessions; }
TranscriptModel *StudyHook::transcriptModel() const { return m_transcript; }
QString StudyHook::selectedSessionId() const { return m_selectedSessionId; }
QString StudyHook::selectedTitle() const { return m_selectedTitle; }
bool StudyHook::sending() const { return m_sending; }
bool StudyHook::busy() const { return m_busy; }
bool StudyHook::streaming() const { return m_streaming; }
QString StudyHook::noticeText() const { return m_noticeText; }
QString StudyHook::modelLabel() const { return m_modelLabel; }
QVariantList StudyHook::modelOptions() const { return m_modelOptions; }
bool StudyHook::modelsOpen() const { return m_modelsOpen; }
QVariantMap StudyHook::pendingApproval() const { return m_pendingApproval; }
QVariantMap StudyHook::pendingQuestion() const { return m_pendingQuestion; }
bool StudyHook::settingsOpen() const { return m_settingsOpen; }
bool StudyHook::settingsWritable() const { return m_settingsWritable; }
bool StudyHook::settingsHasDocument() const { return m_settingsHasDocument; }
QVariantList StudyHook::settingsNamespaces() const { return m_settingsNamespaces; }
bool StudyHook::streamOpen() const { return m_streamOpen; }

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

void StudyHook::setWorkspaces(const QVariantList &workspaces) {
  if (m_workspaces != workspaces) {
    m_workspaces = workspaces;
    emit workspacesChanged();
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

void StudyHook::setStreaming(bool streaming) {
  if (m_streaming != streaming) {
    m_streaming = streaming;
    emit streamingChanged();
  }
}

void StudyHook::setNoticeText(const QString &text) {
  if (m_noticeText != text) {
    m_noticeText = text;
    emit noticeTextChanged();
  }
}

void StudyHook::setModelLabel(const QString &label) {
  if (m_modelLabel != label) {
    m_modelLabel = label;
    emit modelLabelChanged();
  }
}

void StudyHook::setModelOptions(const QVariantList &options) {
  if (m_modelOptions != options) {
    m_modelOptions = options;
    emit modelOptionsChanged();
  }
}

void StudyHook::setModelsOpen(bool open) {
  if (m_modelsOpen != open) {
    m_modelsOpen = open;
    emit modelsOpenChanged();
  }
}

void StudyHook::setPendingApproval(const QVariantMap &approval) {
  if (m_pendingApproval != approval) {
    m_pendingApproval = approval;
    emit pendingApprovalChanged();
  }
}

void StudyHook::setPendingQuestion(const QVariantMap &question) {
  if (m_pendingQuestion != question) {
    m_pendingQuestion = question;
    emit pendingQuestionChanged();
  }
}

void StudyHook::setSettingsOpen(bool open) {
  if (m_settingsOpen != open) {
    m_settingsOpen = open;
    emit settingsOpenChanged();
  }
}

void StudyHook::setSettingsWritable(bool writable) {
  if (m_settingsWritable != writable) {
    m_settingsWritable = writable;
    emit settingsWritableChanged();
  }
}

void StudyHook::setSettingsHasDocument(bool hasDocument) {
  if (m_settingsHasDocument != hasDocument) {
    m_settingsHasDocument = hasDocument;
    emit settingsHasDocumentChanged();
  }
}

void StudyHook::setSettingsNamespaces(const QVariantList &namespaces) {
  if (m_settingsNamespaces != namespaces) {
    m_settingsNamespaces = namespaces;
    emit settingsNamespacesChanged();
  }
}

void StudyHook::setStreamOpen(bool open) {
  if (m_streamOpen != open) {
    m_streamOpen = open;
    emit streamOpenChanged();
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

void StudyHook::selectWorkspace(const QString &workspaceId) {
  if (workspaceId.trimmed().isEmpty()) {
    return;
  }
  emit workspaceRequested(workspaceId);
}

void StudyHook::selectModel(const QString &provider, const QString &model) {
  if (provider.isEmpty() || model.isEmpty()) {
    return;
  }
  setModelsOpen(false);
  emit modelRequested(provider, model);
}

void StudyHook::toggleModels() { setModelsOpen(!m_modelsOpen); }

void StudyHook::cancelTurn() { emit cancelRequested(); }

void StudyHook::answerApproval(const QString &outcome) { emit approvalAnswerRequested(outcome); }

void StudyHook::pickQuestionOption(const QString &label) { emit questionOptionRequested(label); }

void StudyHook::submitQuestionCustom(const QString &text) {
  const QString trimmed = text.trimmed();
  if (trimmed.isEmpty()) {
    return;
  }
  emit questionCustomRequested(trimmed);
}

void StudyHook::openSettings() {
  setSettingsOpen(true);
  emit settingsOpenRequested();
}

void StudyHook::closeSettings() { setSettingsOpen(false); }

void StudyHook::openSettingsDocument() { emit settingsDocumentRequested(); }

void StudyHook::dismissNotice() { setNoticeText({}); }
