#include "hooks/StudyHook.h"

#include "models/SessionListModel.h"
#include "models/TranscriptModel.h"

StudyHook::StudyHook(QObject *parent)
    : QObject(parent),
      m_sessions(new SessionListModel(this)),
      m_transcript(new TranscriptModel(this)),
      m_workspaceTitle(QStringLiteral("未入席")),
      m_modelLabel(QStringLiteral("模型")),
      m_settingsTab(QStringLiteral("overview")),
      m_permissionLabel(QStringLiteral("权限")),
      m_onboardingKeyRef(QStringLiteral("DEEPSEEK_API_KEY")) {}

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
QVariantList StudyHook::settingsFields() const { return m_settingsFields; }
QString StudyHook::settingsTab() const { return m_settingsTab; }
QVariantList StudyHook::providerRows() const { return m_providerRows; }
QVariantList StudyHook::skills() const { return m_skills; }
QVariantList StudyHook::subagents() const { return m_subagents; }
QVariantList StudyHook::agentPresets() const { return m_agentPresets; }
QVariantList StudyHook::permissionOptions() const { return m_permissionOptions; }
QString StudyHook::permissionLabel() const { return m_permissionLabel; }
bool StudyHook::permissionsOpen() const { return m_permissionsOpen; }
bool StudyHook::planActive() const { return m_planActive; }
bool StudyHook::planKnown() const { return m_planKnown; }
QVariantList StudyHook::jobs() const { return m_jobs; }
QVariantList StudyHook::attachments() const { return m_attachments; }
QVariantList StudyHook::slashItems() const { return m_slashItems; }
bool StudyHook::onboardingOpen() const { return m_onboardingOpen; }
QString StudyHook::onboardingKeyRef() const { return m_onboardingKeyRef; }
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

void StudyHook::setSettingsFields(const QVariantList &fields) {
  if (m_settingsFields != fields) {
    m_settingsFields = fields;
    emit settingsFieldsChanged();
  }
}

void StudyHook::setSettingsTab(const QString &tab) {
  if (m_settingsTab != tab) {
    m_settingsTab = tab;
    emit settingsTabChanged();
  }
}

void StudyHook::setProviderRows(const QVariantList &rows) {
  if (m_providerRows != rows) {
    m_providerRows = rows;
    emit providerRowsChanged();
  }
}

void StudyHook::setSkills(const QVariantList &skills) {
  if (m_skills != skills) {
    m_skills = skills;
    emit skillsChanged();
  }
}

void StudyHook::setSubagents(const QVariantList &subagents) {
  if (m_subagents != subagents) {
    m_subagents = subagents;
    emit subagentsChanged();
  }
}

void StudyHook::setAgentPresets(const QVariantList &presets) {
  if (m_agentPresets != presets) {
    m_agentPresets = presets;
    emit agentPresetsChanged();
  }
}

void StudyHook::setPermissionOptions(const QVariantList &options) {
  if (m_permissionOptions != options) {
    m_permissionOptions = options;
    emit permissionOptionsChanged();
  }
}

void StudyHook::setPermissionLabel(const QString &label) {
  if (m_permissionLabel != label) {
    m_permissionLabel = label;
    emit permissionLabelChanged();
  }
}

void StudyHook::setPermissionsOpen(bool open) {
  if (m_permissionsOpen != open) {
    m_permissionsOpen = open;
    emit permissionsOpenChanged();
  }
}

void StudyHook::setPlanActive(bool active) {
  if (m_planActive != active) {
    m_planActive = active;
    emit planActiveChanged();
  }
}

void StudyHook::setPlanKnown(bool known) {
  if (m_planKnown != known) {
    m_planKnown = known;
    emit planKnownChanged();
  }
}

void StudyHook::setJobs(const QVariantList &jobs) {
  if (m_jobs != jobs) {
    m_jobs = jobs;
    emit jobsChanged();
  }
}

void StudyHook::setAttachments(const QVariantList &attachments) {
  if (m_attachments != attachments) {
    m_attachments = attachments;
    emit attachmentsChanged();
  }
}

void StudyHook::setSlashItems(const QVariantList &items) {
  if (m_slashItems != items) {
    m_slashItems = items;
    emit slashItemsChanged();
  }
}

void StudyHook::setOnboardingOpen(bool open) {
  if (m_onboardingOpen != open) {
    m_onboardingOpen = open;
    emit onboardingOpenChanged();
  }
}

void StudyHook::setOnboardingKeyRef(const QString &ref) {
  if (m_onboardingKeyRef != ref) {
    m_onboardingKeyRef = ref;
    emit onboardingKeyRefChanged();
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

void StudyHook::sendPrompt(const QString &text) { emit sendRequested(text); }

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

void StudyHook::setSettingsSection(const QString &tab) {
  if (!tab.isEmpty()) {
    setSettingsTab(tab);
  }
}

void StudyHook::updateSetting(const QString &ns, const QString &key, const QString &kind, const QVariant &value) {
  if (ns.isEmpty() || key.isEmpty()) {
    return;
  }
  emit settingsUpdateRequested(ns, key, kind, value);
}

void StudyHook::setCredential(const QString &ref, const QString &value) {
  if (ref.isEmpty() || value.isEmpty()) {
    return;
  }
  emit credentialSetRequested(ref, value);
}

void StudyHook::selectPermission(const QString &preset) {
  if (preset.isEmpty()) {
    return;
  }
  setPermissionsOpen(false);
  emit permissionRequested(preset);
}

void StudyHook::togglePermissions() { setPermissionsOpen(!m_permissionsOpen); }

void StudyHook::togglePlan() { emit planToggleRequested(); }

void StudyHook::selectPreset(const QString &id) {
  if (id.isEmpty()) {
    return;
  }
  emit presetRequested(id);
}

void StudyHook::attachFromUrl(const QUrl &url) {
  if (!url.isValid()) {
    return;
  }
  emit attachRequested(url);
}

void StudyHook::removeAttachment(int index) {
  if (index < 0) {
    return;
  }
  emit attachmentRemoveRequested(index);
}

void StudyHook::pickSlash(const QString &line) {
  if (line.trimmed().isEmpty()) {
    return;
  }
  emit slashPicked(line);
}

void StudyHook::interruptSubagent(const QString &childId) {
  if (childId.isEmpty()) {
    return;
  }
  emit subagentInterruptRequested(childId);
}

void StudyHook::submitApiKey(const QString &key) { emit onboardingKeyRequested(key); }

void StudyHook::dismissOnboarding() {
  setOnboardingOpen(false);
  emit onboardingDismissRequested();
}
