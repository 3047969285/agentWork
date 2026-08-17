#include "app/Application.h"

#include "app/WindowController.h"
#include "constants/AppConstants.h"
#include "hooks/ConnectionHook.h"
#include "hooks/StudyHook.h"
#include "models/SessionListModel.h"
#include "models/TranscriptModel.h"
#include "services/host/HostProcess.h"
#include "services/rpc/EventStream.h"
#include "services/rpc/RpcClient.h"
#include "services/rpc/RpcTypes.h"
#include "utils/StudyJson.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QSet>
#include <QtGlobal>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>

namespace {

constexpr int kStreamFlushMs = 16;

QString chunkDelta(const QJsonObject &data) {
  const QJsonObject chunk = data.value(QStringLiteral("chunk")).toObject();
  if (chunk.value(QStringLiteral("type")).toString() != QLatin1String("text-delta")) {
    return {};
  }
  return chunk.value(QStringLiteral("text")).toString();
}

}  // namespace

Application::Application(QObject *parent)
    : QObject(parent),
      m_hostProcess(new HostProcess(this)),
      m_rpcClient(new RpcClient(this)),
      m_eventStream(new EventStream(this)),
      m_connectionHook(new ConnectionHook(this)),
      m_studyHook(new StudyHook(this)),
      m_windowController(new WindowController(this)),
      m_streamFlush(new QTimer(this)) {
  m_streamFlush->setSingleShot(true);
  m_streamFlush->setInterval(kStreamFlushMs);
  connect(m_streamFlush, &QTimer::timeout, this, &Application::flushStreamDelta);

  connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &Application::stop);

  connect(m_hostProcess, &HostProcess::started, this, &Application::onHostStarted);
  connect(m_hostProcess, &HostProcess::ready, this, &Application::onHostReady);
  connect(m_hostProcess, &HostProcess::stopped, this, &Application::onHostStopped);
  connect(m_hostProcess, &HostProcess::errorOccurred, this, &Application::onHostError);

  connect(m_connectionHook, &ConnectionHook::retryRequested, this, &Application::onRetryRequested);
  connect(m_studyHook, &StudyHook::selectRequested, this, &Application::onSelectRequested);
  connect(m_studyHook, &StudyHook::createRequested, this, &Application::onCreateRequested);
  connect(m_studyHook, &StudyHook::sendRequested, this, &Application::onSendRequested);
  connect(m_studyHook, &StudyHook::refreshRequested, this, &Application::onRefreshRequested);
  connect(m_studyHook, &StudyHook::workspaceRequested, this, &Application::onWorkspaceRequested);
  connect(m_studyHook, &StudyHook::modelRequested, this, &Application::onModelRequested);
  connect(m_studyHook, &StudyHook::cancelRequested, this, &Application::onCancelRequested);
  connect(m_studyHook, &StudyHook::approvalAnswerRequested, this, &Application::onApprovalAnswerRequested);
  connect(m_studyHook, &StudyHook::questionOptionRequested, this, &Application::onQuestionOptionRequested);
  connect(m_studyHook, &StudyHook::questionCustomRequested, this, &Application::onQuestionCustomRequested);
  connect(m_studyHook, &StudyHook::settingsOpenRequested, this, &Application::onSettingsOpenRequested);
  connect(m_studyHook, &StudyHook::settingsDocumentRequested, this, &Application::onSettingsDocumentRequested);
  connect(m_studyHook, &StudyHook::settingsUpdateRequested, this, &Application::onSettingsUpdateRequested);
  connect(m_studyHook, &StudyHook::credentialSetRequested, this, &Application::onCredentialSetRequested);
  connect(m_studyHook, &StudyHook::permissionRequested, this, &Application::onPermissionRequested);
  connect(m_studyHook, &StudyHook::planToggleRequested, this, &Application::onPlanToggleRequested);
  connect(m_studyHook, &StudyHook::presetRequested, this, &Application::onPresetRequested);
  connect(m_studyHook, &StudyHook::attachRequested, this, &Application::onAttachRequested);
  connect(m_studyHook, &StudyHook::attachmentRemoveRequested, this, &Application::onAttachmentRemoveRequested);
  connect(m_studyHook, &StudyHook::slashPicked, this, &Application::onSlashPicked);
  connect(m_studyHook, &StudyHook::subagentInterruptRequested, this, &Application::onSubagentInterruptRequested);
  connect(m_studyHook, &StudyHook::onboardingKeyRequested, this, &Application::onOnboardingKeyRequested);

  connect(m_eventStream, &EventStream::muxFrame, this, &Application::onMuxFrame);
  connect(m_eventStream, &EventStream::hostFrame, this, &Application::onHostFrame);
  connect(m_eventStream, &EventStream::openChanged, this, &Application::onStreamOpenChanged);
  connect(m_eventStream, &EventStream::errorOccurred, this, [this](const QString &message) {
    if (!m_isStopping) {
      m_studyHook->setNoticeText(message);
    }
  });
}

Application::~Application() { stop(); }

bool Application::init() { return m_windowController->init(m_connectionHook, m_studyHook); }

void Application::start() {
  if (m_isStopping) {
    return;
  }

  m_connectionHook->setConnecting(true);
  m_connectionHook->setConnected(false);
  m_connectionHook->setHasError(false);
  m_connectionHook->setStatusText(QStringLiteral("正在启动宿主进程…"));

  if (m_hostProcess->isRunning()) {
    if (m_port > 0) {
      doHandshake();
    }
    return;
  }

  QTimer::singleShot(0, this, [this]() {
    if (m_isStopping) {
      return;
    }
    if (m_hostProcess->isRunning()) {
      if (m_port > 0) {
        doHandshake();
      }
      return;
    }
    QString errorMessage;
    if (!m_hostProcess->start(&errorMessage)) {
      scheduleRetry(errorMessage);
    }
  });
}

void Application::stop() {
  m_isStopping = true;
  m_streamFlush->stop();
  m_pendingDelta.clear();
  if (m_eventStream != nullptr) {
    m_eventStream->disconnectFromHost();
  }
  if (m_hostProcess != nullptr) {
    m_hostProcess->stop();
  }
}

void Application::onHostStarted() {
  m_connectionHook->setStatusText(QStringLiteral("宿主进程已启动，等待就绪…"));
}

void Application::onHostReady(quint16 port) {
  m_port = port;
  m_connectionHook->setStatusText(QStringLiteral("宿主已就绪，正在握手…"));
  const QUrl baseUrl(QStringLiteral("http://%1:%2").arg(AppConstants::kDefaultHost).arg(port));
  m_rpcClient->setBaseUrl(baseUrl);
  doHandshake();
}

void Application::onHostStopped() {
  if (!m_isStopping) {
    m_eventStream->disconnectFromHost();
    m_studyHook->setStreamOpen(false);
    m_connectionHook->setConnected(false);
    m_connectionHook->setConnecting(false);
    m_connectionHook->setHasError(true);
    m_connectionHook->setHostPort(0);
    m_connectionHook->setStatusText(QStringLiteral("宿主进程已停止"));
  }
}

void Application::onHostError(const QString &error) {
  if (!m_isStopping) {
    scheduleRetry(error);
  }
}

void Application::onRetryRequested() {
  m_retryCount = 0;
  start();
}

void Application::doHandshake() {
  if (m_isStopping) {
    return;
  }

  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodHostDescribe), QJsonObject{},
                         [this](bool ok, QJsonValue resultOrError) {
                           if (m_isStopping) {
                             return;
                           }

                           if (ok) {
                             m_retryCount = 0;
                             m_connectionHook->setConnecting(false);
                             m_connectionHook->setConnected(true);
                             m_connectionHook->setHasError(false);

                             QString version = QStringLiteral("开发版");
                             if (resultOrError.isObject()) {
                               const QJsonObject obj = resultOrError.toObject();
                               const QString reported = obj.value(QStringLiteral("version")).toString().trimmed();
                               if (!reported.isEmpty() && reported != QLatin1String("dev")) {
                                 version = reported;
                               }
                             }

                             m_connectionHook->setHostVersion(version);
                             m_connectionHook->setHostPort(m_port);
                             m_connectionHook->setStatusText(
                                 QStringLiteral("已连接 · %1 · 端口 %2").arg(version).arg(m_port));
                             connectStreams();
                             loadStudy();
                             loadHostCatalog();
                           } else {
                             scheduleRetry(dsh::study::rpcErrorMessage(resultOrError));
                           }
                         });
}

void Application::scheduleRetry(const QString &reason) {
  if (m_isStopping) {
    return;
  }

  if (m_retryCount < AppConstants::kConnectRetryCount) {
    m_retryCount++;
    m_connectionHook->setConnecting(true);
    m_connectionHook->setConnected(false);
    m_connectionHook->setHasError(false);
    m_connectionHook->setStatusText(
        QStringLiteral("%1，%2 秒后重试 (%3/%4)…")
            .arg(reason)
            .arg(AppConstants::kConnectRetryMs / 1000)
            .arg(m_retryCount)
            .arg(AppConstants::kConnectRetryCount));
    QTimer::singleShot(AppConstants::kConnectRetryMs, this, [this]() {
      if (!m_isStopping && !m_connectionHook->connected()) {
        start();
      }
    });
  } else {
    m_connectionHook->setConnecting(false);
    m_connectionHook->setConnected(false);
    m_connectionHook->setHasError(true);
    m_connectionHook->setStatusText(QStringLiteral("连接失败: %1").arg(reason));
  }
}

void Application::connectStreams() {
  m_eventStream->connectTo(m_rpcClient->baseUrl());
}

void Application::onStreamOpenChanged(bool open) {
  m_studyHook->setStreamOpen(open);
  if (!open && m_connectionHook->connected() && !m_isStopping) {
    m_studyHook->setNoticeText(QStringLiteral("流未通 · 点刷新"));
  }
}

void Application::loadStudy() {
  if (m_isStopping || !m_connectionHook->connected()) {
    return;
  }
  if (!m_eventStream->isOpen()) {
    connectStreams();
  }

  const int gen = ++m_studyGeneration;
  m_studyHook->setBusy(true);
  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodWorkspaceList), QJsonObject{},
                         [this, gen](bool workspaceOk, QJsonValue workspaceValue) {
                           if (m_isStopping || gen != m_studyGeneration) {
                             return;
                           }
                           const QJsonObject workspaceList =
                               workspaceOk && workspaceValue.isObject() ? workspaceValue.toObject() : QJsonObject{};
                           if (!workspaceOk) {
                             m_studyHook->setNoticeText(dsh::study::rpcErrorMessage(workspaceValue));
                           }

                           m_rpcClient->callUnary(
                               QString::fromLatin1(dsh::rpc::kMethodSessionList), QJsonObject{},
                               [this, gen, workspaceList](bool sessionOk, QJsonValue sessionValue) {
                                 if (m_isStopping || gen != m_studyGeneration) {
                                   return;
                                 }
                                 m_studyHook->setBusy(false);
                                 if (!sessionOk) {
                                   m_studyHook->setNoticeText(dsh::study::rpcErrorMessage(sessionValue));
                                   return;
                                 }
                                 if (sessionValue.isObject()) {
                                   applyStudyLists(workspaceList, sessionValue.toObject());
                                 }
                               });
                         });
}

void Application::applyStudyLists(const QJsonObject &workspaceList, const QJsonObject &sessionList) {
  m_workspaceList = workspaceList;
  m_sessionList = sessionList;
  m_studyHook->setWorkspaces(dsh::study::workspaceRows(workspaceList));

  const QJsonObject workspace = dsh::study::workspaceById(workspaceList, m_studyHook->workspaceId());
  const QString workspaceId = workspace.value(QStringLiteral("workspaceId")).toString();
  m_studyHook->setWorkspaceId(workspaceId);
  m_studyHook->setWorkspaceTitle(workspace.isEmpty() ? QStringLiteral("未入席")
                                                     : dsh::study::workspaceTitle(workspace));

  QSet<QString> allowIds;
  const QSet<QString> *allowPtr = nullptr;
  if (!workspace.isEmpty()) {
    const QJsonArray sessionIds = workspace.value(QStringLiteral("sessionIds")).toArray();
    for (const QJsonValue &value : sessionIds) {
      const QString id = value.toString();
      if (!id.isEmpty()) {
        allowIds.insert(id);
      }
    }
    allowPtr = &allowIds;
  }

  const QVariantList rows = dsh::study::sessionRows(sessionList.value(QStringLiteral("items")).toArray(),
                                                    allowPtr, dsh::study::archivedSessionIds(workspaceList));
  m_studyHook->sessionList()->replaceAll(rows);

  QString selected = m_studyHook->selectedSessionId();
  if (!m_studyHook->sessionList()->contains(selected)) {
    selected = m_studyHook->sessionList()->firstSessionId();
  }
  applySelectedSession(selected);
}

void Application::applySelectedSession(const QString &sessionId) {
  m_streamFlush->stop();
  m_pendingDelta.clear();
  m_studyHook->setSelectedSessionId(sessionId);
  m_studyHook->setSelectedTitle(titleForSession(sessionId));
  m_studyHook->setModelsOpen(false);
  m_studyHook->setPermissionsOpen(false);
  m_studyHook->setPendingApproval({});
  m_studyHook->setPendingQuestion({});
  m_studyHook->setJobs({});
  m_studyHook->setAttachments({});
  m_studyHook->setPlanKnown(false);
  m_studyHook->setPlanActive(false);
  m_permissionProjection = QJsonValue();
  m_questionItems = QJsonArray();
  m_questionAnswers = QJsonArray();
  m_questionIndex = 0;
  m_questionRpcId.clear();
  m_questionSessionId.clear();
  if (sessionId.isEmpty()) {
    m_studyHook->transcriptModel()->clear();
    m_studyHook->setSending(false);
    m_studyHook->setStreaming(false);
    return;
  }
  loadHistory(sessionId);
  loadModels(sessionId);
  loadSessionExtras(sessionId);
}

QString Application::titleForSession(const QString &sessionId) const {
  return m_studyHook->sessionList()->titleFor(sessionId);
}

void Application::onSelectRequested(const QString &sessionId) {
  if (!m_connectionHook->connected()) {
    m_studyHook->setNoticeText(QStringLiteral("尚未连接"));
    return;
  }
  m_studyHook->setSending(false);
  m_studyHook->setNoticeText({});
  applySelectedSession(sessionId);
}

void Application::onWorkspaceRequested(const QString &workspaceId) {
  if (!m_connectionHook->connected()) {
    m_studyHook->setNoticeText(QStringLiteral("尚未连接"));
    return;
  }
  m_studyHook->setWorkspaceId(workspaceId);
  applyStudyLists(m_workspaceList, m_sessionList);
}

void Application::onCreateRequested() {
  if (!m_connectionHook->connected()) {
    m_studyHook->setNoticeText(QStringLiteral("尚未连接"));
    return;
  }
  const QString reusable = m_studyHook->sessionList()->blankSessionId();
  if (!reusable.isEmpty()) {
    onSelectRequested(reusable);
    return;
  }

  m_studyHook->setBusy(true);
  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodSessionCreate),
                         dsh::study::createPayload(m_studyHook->workspaceId()),
                         [this](bool ok, QJsonValue resultOrError) {
                           m_studyHook->setBusy(false);
                           if (!ok) {
                             m_studyHook->setNoticeText(dsh::study::rpcErrorMessage(resultOrError));
                             return;
                           }
                           m_studyHook->setNoticeText({});
                           const QString createdId = resultOrError.toObject().value(QStringLiteral("sessionId")).toString();
                           if (!createdId.isEmpty()) {
                             m_studyHook->setSelectedSessionId(createdId);
                           }
                           loadStudy();
                         });
}

void Application::onSendRequested(const QString &text) {
  const QString sessionId = m_studyHook->selectedSessionId();
  if (!m_connectionHook->connected()) {
    m_studyHook->setNoticeText(QStringLiteral("尚未连接"));
    return;
  }
  if (sessionId.isEmpty()) {
    m_studyHook->setNoticeText(QStringLiteral("请先选择或新建会话"));
    return;
  }
  if (m_studyHook->sending()) {
    return;
  }
  const QString trimmed = text.trimmed();
  const QVariantList images = m_studyHook->attachments();
  if (trimmed.isEmpty() && images.isEmpty()) {
    return;
  }

  m_studyHook->setSending(true);
  m_studyHook->setNoticeText({});
  if (!trimmed.isEmpty()) {
    m_studyHook->transcriptModel()->appendUser(trimmed);
  } else {
    m_studyHook->transcriptModel()->appendUser(QStringLiteral("附页"));
  }
  const QJsonObject payload = dsh::study::promptPayload(sessionId, trimmed, images);
  m_studyHook->setAttachments({});

  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodSessionPrompt), payload,
                         [this, sessionId](bool ok, QJsonValue resultOrError) {
                           if (m_studyHook->selectedSessionId() != sessionId) {
                             return;
                           }
                           if (!ok) {
                             m_studyHook->setSending(false);
                             m_studyHook->setNoticeText(dsh::study::rpcErrorMessage(resultOrError));
                             loadHistory(sessionId);
                             return;
                           }
                           const QJsonObject command = resultOrError.toObject().value(QStringLiteral("command")).toObject();
                           if (command.value(QStringLiteral("kind")).toString() == QLatin1String("success") &&
                               !command.value(QStringLiteral("text")).toString().isEmpty()) {
                             m_studyHook->setNoticeText(command.value(QStringLiteral("text")).toString());
                             m_studyHook->setSending(false);
                           }
                         });
}

void Application::onRefreshRequested() {
  loadStudy();
  loadHostCatalog();
  loadSessionExtras(m_studyHook->selectedSessionId());
}

void Application::onModelRequested(const QString &provider, const QString &model) {
  const QString sessionId = m_studyHook->selectedSessionId();
  if (!m_connectionHook->connected() || sessionId.isEmpty()) {
    m_studyHook->setNoticeText(QStringLiteral("尚未连接"));
    return;
  }
  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodSessionSelectModel),
                         dsh::study::selectModelPayload(sessionId, provider, model),
                         [this, sessionId, model](bool ok, QJsonValue resultOrError) {
                           if (m_studyHook->selectedSessionId() != sessionId) {
                             return;
                           }
                           if (!ok) {
                             m_studyHook->setNoticeText(dsh::study::rpcErrorMessage(resultOrError));
                             return;
                           }
                           const QJsonObject selected = resultOrError.toObject().value(QStringLiteral("selected")).toObject();
                           const QString label = selected.value(QStringLiteral("model")).toString();
                           m_studyHook->setModelLabel(label.isEmpty() ? model : label);
                           m_studyHook->setNoticeText({});
                         });
}

void Application::onCancelRequested() {
  const QString sessionId = m_studyHook->selectedSessionId();
  if (!m_connectionHook->connected() || sessionId.isEmpty()) {
    return;
  }
  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodSessionCancel), dsh::study::cancelPayload(sessionId),
                         [this, sessionId](bool ok, QJsonValue resultOrError) {
                           if (m_studyHook->selectedSessionId() != sessionId) {
                             return;
                           }
                           if (!ok) {
                             m_studyHook->setNoticeText(dsh::study::rpcErrorMessage(resultOrError));
                             return;
                           }
                           m_studyHook->setSending(false);
                           m_studyHook->setStreaming(false);
                           flushStreamDelta();
                           m_studyHook->transcriptModel()->finishStreaming();
                         });
}

void Application::onApprovalAnswerRequested(const QString &outcome) {
  const QVariantMap pending = m_studyHook->pendingApproval();
  const QString rpcId = pending.value(QStringLiteral("rpcId")).toString();
  const QString sessionId = pending.value(QStringLiteral("sessionId")).toString();
  const QString approvalId = pending.value(QStringLiteral("approvalId")).toString();
  if (rpcId.isEmpty() || sessionId.isEmpty() || approvalId.isEmpty()) {
    return;
  }
  if (outcome != QLatin1String("allowed-once") && outcome != QLatin1String("rejected")) {
    return;
  }
  QJsonObject value;
  value.insert(QStringLiteral("sessionId"), sessionId);
  value.insert(QStringLiteral("approvalId"), approvalId);
  value.insert(QStringLiteral("outcome"), outcome);
  m_rpcClient->callRespond(rpcId, value, [this](bool accepted, QString reason) {
    if (!accepted) {
      m_studyHook->setNoticeText(reason.isEmpty() ? QStringLiteral("准许未受理") : reason);
    }
  });
}

void Application::onQuestionOptionRequested(const QString &label) {
  if (m_questionIndex < 0 || m_questionIndex >= m_questionItems.size()) {
    return;
  }
  const QJsonObject item = m_questionItems.at(m_questionIndex).toObject();
  QJsonObject answer;
  answer.insert(QStringLiteral("id"), item.value(QStringLiteral("id")).toString());
  answer.insert(QStringLiteral("selected"), QJsonArray{label});
  appendQuestionAnswer(answer);
}

void Application::onQuestionCustomRequested(const QString &text) {
  if (m_questionIndex < 0 || m_questionIndex >= m_questionItems.size()) {
    return;
  }
  const QJsonObject item = m_questionItems.at(m_questionIndex).toObject();
  QJsonObject answer;
  answer.insert(QStringLiteral("id"), item.value(QStringLiteral("id")).toString());
  answer.insert(QStringLiteral("selected"), QJsonArray{});
  answer.insert(QStringLiteral("custom"), text);
  appendQuestionAnswer(answer);
}

void Application::appendQuestionAnswer(const QJsonObject &answer) {
  m_questionAnswers.append(answer);
  ++m_questionIndex;
  if (m_questionIndex >= m_questionItems.size()) {
    submitQuestionBatch();
    return;
  }
  publishQuestion();
}

void Application::publishQuestion() {
  if (m_questionIndex < 0 || m_questionIndex >= m_questionItems.size()) {
    m_studyHook->setPendingQuestion({});
    return;
  }
  const QJsonObject item = m_questionItems.at(m_questionIndex).toObject();
  QVariantMap row;
  row.insert(QStringLiteral("rpcId"), m_questionRpcId);
  row.insert(QStringLiteral("sessionId"), m_questionSessionId);
  row.insert(QStringLiteral("questionId"), item.value(QStringLiteral("id")).toString());
  row.insert(QStringLiteral("question"), item.value(QStringLiteral("question")).toString());
  row.insert(QStringLiteral("header"), item.value(QStringLiteral("header")).toString());
  row.insert(QStringLiteral("detail"), item.value(QStringLiteral("detail")).toString());
  row.insert(QStringLiteral("multiSelect"), item.value(QStringLiteral("multiSelect")).toBool());
  const QJsonObject intent = item.value(QStringLiteral("intent")).toObject();
  row.insert(QStringLiteral("intentKind"), intent.value(QStringLiteral("kind")).toString());
  row.insert(QStringLiteral("index"), m_questionIndex + 1);
  row.insert(QStringLiteral("total"), m_questionItems.size());
  QVariantList options;
  for (const QJsonValue &optionValue : item.value(QStringLiteral("options")).toArray()) {
    const QJsonObject option = optionValue.toObject();
    QVariantMap optionRow;
    optionRow.insert(QStringLiteral("label"), option.value(QStringLiteral("label")).toString());
    optionRow.insert(QStringLiteral("description"), option.value(QStringLiteral("description")).toString());
    options.append(optionRow);
  }
  row.insert(QStringLiteral("options"), options);
  m_studyHook->setPendingQuestion(row);
}

void Application::submitQuestionBatch() {
  QJsonObject value;
  value.insert(QStringLiteral("sessionId"), m_questionSessionId);
  QJsonObject answer;
  answer.insert(QStringLiteral("answers"), m_questionAnswers);
  value.insert(QStringLiteral("answer"), answer);
  const QString rpcId = m_questionRpcId;
  m_rpcClient->callRespond(rpcId, value, [this](bool accepted, QString reason) {
    if (!accepted) {
      m_studyHook->setNoticeText(reason.isEmpty() ? QStringLiteral("问答未受理") : reason);
    }
  });
}

void Application::onSettingsOpenRequested() {
  if (!m_connectionHook->connected()) {
    m_studyHook->setNoticeText(QStringLiteral("尚未连接"));
    m_studyHook->setSettingsOpen(false);
    return;
  }
  loadHostCatalog();
}

void Application::onSettingsDocumentRequested() {
  if (!m_connectionHook->connected()) {
    m_studyHook->setNoticeText(QStringLiteral("尚未连接"));
    return;
  }
  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodSettingsOpenDocument), QJsonObject{},
                         [this](bool ok, QJsonValue resultOrError) {
                           if (!ok) {
                             m_studyHook->setNoticeText(dsh::study::rpcErrorMessage(resultOrError));
                           }
                         });
}

void Application::loadHistory(const QString &sessionId) {
  if (sessionId.isEmpty() || !m_connectionHook->connected()) {
    return;
  }
  const int gen = ++m_historyGeneration;
  QJsonObject payload;
  payload.insert(QStringLiteral("sessionId"), sessionId);
  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodSessionHistory), payload,
                         [this, gen, sessionId](bool ok, QJsonValue resultOrError) {
                           if (m_isStopping || gen != m_historyGeneration ||
                               m_studyHook->selectedSessionId() != sessionId) {
                             return;
                           }
                           if (!ok) {
                             m_studyHook->setNoticeText(dsh::study::rpcErrorMessage(resultOrError));
                             return;
                           }
                           const QJsonArray events = resultOrError.toObject().value(QStringLiteral("events")).toArray();
                           m_studyHook->transcriptModel()->resetFromHistory(events);
                         });
}

void Application::loadModels(const QString &sessionId) {
  if (sessionId.isEmpty() || !m_connectionHook->connected()) {
    return;
  }
  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodSessionModels), dsh::study::modelsPayload(sessionId),
                         [this, sessionId](bool ok, QJsonValue resultOrError) {
                           if (m_studyHook->selectedSessionId() != sessionId) {
                             return;
                           }
                           if (!ok) {
                             m_studyHook->setModelOptions({});
                             m_studyHook->setModelLabel(QStringLiteral("模型"));
                             return;
                           }
                           const QJsonObject value = resultOrError.toObject();
                           m_studyHook->setModelOptions(dsh::study::modelOptions(value));
                           m_studyHook->setModelLabel(dsh::study::modelLabel(value));
                         });
}

void Application::flushStreamDelta() {
  if (m_pendingDelta.isEmpty()) {
    return;
  }
  m_studyHook->transcriptModel()->appendStreamDelta(m_pendingDelta);
  m_pendingDelta.clear();
  m_studyHook->setStreaming(m_studyHook->transcriptModel()->hasStreaming());
}

void Application::onMuxFrame(const QString &rpcId, const QJsonObject &payload) {
  if (m_isStopping) {
    return;
  }
  const QString type = payload.value(QStringLiteral("type")).toString();
  const QString sessionId = payload.value(QStringLiteral("sessionId")).toString();
  const QString selected = m_studyHook->selectedSessionId();

  if (type == QLatin1String("session/projection")) {
    applyProjection(payload.value(QStringLiteral("key")).toString(), payload.value(QStringLiteral("value")));
    const QString title = dsh::study::projectionTitleValue(payload.value(QStringLiteral("key")).toString(),
                                                           payload.value(QStringLiteral("value")));
    if (!title.isEmpty() && !sessionId.isEmpty()) {
      m_studyHook->sessionList()->setTitle(sessionId, title);
      if (sessionId == selected) {
        m_studyHook->setSelectedTitle(title);
      }
    }
    return;
  }

  if (type == QLatin1String("session/jobs") && sessionId == selected) {
    m_studyHook->setJobs(dsh::study::jobRows(payload.value(QStringLiteral("jobs")).toArray()));
    return;
  }

  if (type == QLatin1String("approval/requested") && sessionId == selected) {
    QVariantMap row;
    row.insert(QStringLiteral("rpcId"), rpcId);
    row.insert(QStringLiteral("sessionId"), sessionId);
    row.insert(QStringLiteral("approvalId"), payload.value(QStringLiteral("approvalId")).toString());
    row.insert(QStringLiteral("toolName"), payload.value(QStringLiteral("toolName")).toString());
    row.insert(QStringLiteral("reason"), payload.value(QStringLiteral("reason")).toString());
    m_studyHook->setPendingApproval(row);
    return;
  }
  if (type == QLatin1String("approval/resolved") && sessionId == selected) {
    m_studyHook->setPendingApproval({});
    return;
  }
  if (type == QLatin1String("question/requested") && sessionId == selected) {
    m_questionRpcId = rpcId;
    m_questionSessionId = sessionId;
    m_questionItems = payload.value(QStringLiteral("questions")).toArray();
    m_questionAnswers = QJsonArray();
    m_questionIndex = 0;
    publishQuestion();
    return;
  }
  if (type == QLatin1String("question/resolved") && sessionId == selected) {
    m_studyHook->setPendingQuestion({});
    m_questionItems = QJsonArray();
    m_questionAnswers = QJsonArray();
    m_questionIndex = 0;
    m_questionRpcId.clear();
    m_questionSessionId.clear();
    return;
  }

  if (type != QLatin1String("session/event") || sessionId != selected) {
    return;
  }

  const QJsonObject event = payload.value(QStringLiteral("event")).toObject();
  const QString eventType = event.value(QStringLiteral("type")).toString();
  if (eventType == QLatin1String("assistant/chunk")) {
    const QString delta = chunkDelta(event.value(QStringLiteral("data")).toObject());
    if (!delta.isEmpty()) {
      m_pendingDelta += delta;
      if (!m_streamFlush->isActive()) {
        m_streamFlush->start();
      }
    }
    return;
  }
  flushStreamDelta();
  m_studyHook->transcriptModel()->applySessionEvent(event, payload.value(QStringLiteral("view")));
  if (eventType == QLatin1String("assistant/message") || eventType == QLatin1String("turn/end")) {
    m_studyHook->setStreaming(false);
    m_studyHook->transcriptModel()->finishStreaming();
  }
}

void Application::onHostFrame(const QString &, const QJsonObject &payload) {
  if (m_isStopping) {
    return;
  }
  const QString type = payload.value(QStringLiteral("type")).toString();
  const QString sessionId = payload.value(QStringLiteral("sessionId")).toString();
  if (type == QLatin1String("host/session-status")) {
    const bool running = payload.value(QStringLiteral("running")).toBool();
    m_studyHook->sessionList()->setRunning(sessionId, running);
    if (sessionId == m_studyHook->selectedSessionId()) {
      m_studyHook->setSending(running);
      if (!running) {
        flushStreamDelta();
        m_studyHook->setStreaming(false);
        m_studyHook->transcriptModel()->finishStreaming();
      }
    }
    return;
  }
  if (type == QLatin1String("host/agent-error") && sessionId == m_studyHook->selectedSessionId()) {
    m_studyHook->setNoticeText(payload.value(QStringLiteral("message")).toString());
    m_studyHook->setSending(false);
    m_studyHook->setStreaming(false);
    return;
  }
  if (type == QLatin1String("host/session-added") || type == QLatin1String("host/session-removed") ||
      type == QLatin1String("host/workspace-changed") || type == QLatin1String("host/workspace-removed") ||
      type == QLatin1String("host/workspace-order-changed") ||
      type == QLatin1String("host/archived-sessions-changed")) {
    loadStudy();
    return;
  }
  if (type == QLatin1String("host/remote-event")) {
    const QString event = payload.value(QStringLiteral("event")).toString();
    if (event.contains(QLatin1String("settings")) || event.contains(QLatin1String("credentials")) ||
        event.contains(QLatin1String("llm"))) {
      loadHostCatalog();
    }
  }
}

void Application::applyProjection(const QString &key, const QJsonValue &value) {
  if (key == QLatin1String("plan")) {
    const QVariantMap plan = dsh::study::planState(value);
    m_studyHook->setPlanKnown(plan.value(QStringLiteral("known")).toBool());
    m_studyHook->setPlanActive(plan.value(QStringLiteral("active")).toBool());
    refreshSlashItems();
    return;
  }
  if (key == QLatin1String("permissions")) {
    m_permissionProjection = value;
    const QVariantList options = dsh::study::permissionOptions(value, m_settingsDescribe);
    m_studyHook->setPermissionOptions(options);
    QString label = QStringLiteral("权限");
    for (const QVariant &optionValue : options) {
      const QVariantMap option = optionValue.toMap();
      if (option.value(QStringLiteral("current")).toBool()) {
        label = option.value(QStringLiteral("label")).toString();
        break;
      }
    }
    m_studyHook->setPermissionLabel(label);
    refreshSlashItems();
    return;
  }
  if (key == QLatin1String("imageLimits")) {
    m_imageLimits = dsh::study::imageLimits(value);
  }
}

void Application::refreshSlashItems() {
  m_studyHook->setSlashItems(dsh::study::slashItems(m_studyHook->skills(), m_studyHook->permissionOptions(),
                                                    m_studyHook->planActive()));
}

void Application::loadHostCatalog() {
  if (m_isStopping || !m_connectionHook->connected()) {
    return;
  }
  const int gen = ++m_catalogGeneration;
  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodSettingsDescribe), QJsonObject{},
                         [this, gen](bool settingsOk, QJsonValue settingsValue) {
                           if (m_isStopping || gen != m_catalogGeneration) {
                             return;
                           }
                           if (!settingsOk) {
                             m_studyHook->setNoticeText(dsh::study::rpcErrorMessage(settingsValue));
                             return;
                           }
                           const QJsonObject describe = settingsValue.toObject();
                           m_settingsDescribe = describe;
                           m_studyHook->setSettingsWritable(describe.value(QStringLiteral("writable")).toBool());
                           m_studyHook->setSettingsHasDocument(describe.value(QStringLiteral("hasDocument")).toBool());
                           m_studyHook->setSettingsNamespaces(dsh::study::settingsNamespaces(describe));
                           m_studyHook->setSettingsFields(dsh::study::settingsFields(describe));
                           m_studyHook->setPermissionOptions(
                               dsh::study::permissionOptions(m_permissionProjection, describe));
                           refreshSlashItems();

                           m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodLlmProviders), QJsonObject{},
                                                  [this, gen, describe](bool providersOk, QJsonValue providersValue) {
                                                    if (m_isStopping || gen != m_catalogGeneration) {
                                                      return;
                                                    }
                                                    const QJsonObject providers =
                                                        providersOk && providersValue.isObject()
                                                            ? providersValue.toObject()
                                                            : QJsonObject{};
                                                    QStringList refs;
                                                    refs.append(QStringLiteral("DEEPSEEK_API_KEY"));
                                                    m_rpcClient->callUnary(
                                                        QString::fromLatin1(dsh::rpc::kMethodCredentialsDescribe),
                                                        dsh::study::credentialsDescribePayload(refs),
                                                        [this, gen, describe, providers](bool credOk,
                                                                                         QJsonValue credValue) {
                                                          if (m_isStopping || gen != m_catalogGeneration) {
                                                            return;
                                                          }
                                                          const QJsonObject credentials =
                                                              credOk && credValue.isObject() ? credValue.toObject()
                                                                                             : QJsonObject{};
                                                          const QVariantList rows =
                                                              dsh::study::providerRows(providers, describe, credentials);
                                                          m_studyHook->setProviderRows(rows);
                                                          QString keyRef = QStringLiteral("DEEPSEEK_API_KEY");
                                                          for (const QVariant &rowValue : rows) {
                                                            const QVariantMap row = rowValue.toMap();
                                                            if (row.value(QStringLiteral("official")).toBool()) {
                                                              const QString env =
                                                                  row.value(QStringLiteral("apiKeyEnv")).toString();
                                                              if (!env.isEmpty()) {
                                                                keyRef = env;
                                                              }
                                                              break;
                                                            }
                                                          }
                                                          m_studyHook->setOnboardingKeyRef(keyRef);
                                                          if (dsh::study::onboardingNeeded(rows)) {
                                                            m_studyHook->setOnboardingOpen(true);
                                                          } else if (m_studyHook->onboardingOpen()) {
                                                            m_studyHook->setOnboardingOpen(false);
                                                          }
                                                        });
                                                  });

                           m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodAgentPresetList), QJsonObject{},
                                                  [this, gen](bool ok, QJsonValue resultOrError) {
                                                    if (m_isStopping || gen != m_catalogGeneration) {
                                                      return;
                                                    }
                                                    if (ok) {
                                                      m_studyHook->setAgentPresets(
                                                          dsh::study::presetRows(resultOrError.toObject()));
                                                    }
                                                  });
                         });
}

void Application::loadSessionExtras(const QString &sessionId) {
  if (sessionId.isEmpty() || !m_connectionHook->connected()) {
    m_studyHook->setSkills({});
    m_studyHook->setSubagents({});
    refreshSlashItems();
    return;
  }
  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodSkillList), dsh::study::skillListPayload(sessionId),
                         [this, sessionId](bool ok, QJsonValue resultOrError) {
                           if (m_studyHook->selectedSessionId() != sessionId) {
                             return;
                           }
                           m_studyHook->setSkills(ok ? dsh::study::skillRows(resultOrError.toObject()) : QVariantList{});
                           refreshSlashItems();
                         });
  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodSubagentList), dsh::study::subagentListPayload(sessionId),
                         [this, sessionId](bool ok, QJsonValue resultOrError) {
                           if (m_studyHook->selectedSessionId() != sessionId) {
                             return;
                           }
                           m_studyHook->setSubagents(ok ? dsh::study::subagentRows(resultOrError.toObject())
                                                        : QVariantList{});
                         });
}

void Application::onSettingsUpdateRequested(const QString &ns, const QString &key, const QString &kind,
                                            const QVariant &value) {
  if (!m_connectionHook->connected()) {
    m_studyHook->setNoticeText(QStringLiteral("尚未连接"));
    return;
  }
  int revision = -1;
  const QJsonArray namespaces = m_settingsDescribe.value(QStringLiteral("namespaces")).toArray();
  for (const QJsonValue &itemValue : namespaces) {
    const QJsonObject item = itemValue.toObject();
    if (item.value(QStringLiteral("ns")).toString() == ns) {
      revision = item.value(QStringLiteral("revision")).toInt();
      break;
    }
  }
  QJsonValue jsonValue;
  if (kind == QLatin1String("bool")) {
    jsonValue = QJsonValue(value.toBool());
  } else if (kind == QLatin1String("number")) {
    bool ok = false;
    const double number = value.toString().isEmpty() ? value.toDouble(&ok) : value.toString().toDouble(&ok);
    if (!ok && value.canConvert<double>()) {
      jsonValue = QJsonValue(value.toDouble());
    } else if (ok) {
      jsonValue = QJsonValue(number);
    } else {
      m_studyHook->setNoticeText(QStringLiteral("请输入数字"));
      return;
    }
  } else {
    jsonValue = QJsonValue(value.toString());
  }
  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodSettingsUpdate),
                         dsh::study::settingsUpdatePayload(ns, key, jsonValue, revision),
                         [this](bool ok, QJsonValue resultOrError) {
                           if (!ok) {
                             m_studyHook->setNoticeText(dsh::study::rpcErrorMessage(resultOrError));
                             return;
                           }
                           m_studyHook->setNoticeText(QStringLiteral("已写入"));
                           loadHostCatalog();
                         });
}

void Application::onCredentialSetRequested(const QString &ref, const QString &value) {
  if (!m_connectionHook->connected()) {
    m_studyHook->setNoticeText(QStringLiteral("尚未连接"));
    return;
  }
  const QString failure = dsh::study::apiKeyFailure(value);
  if (!failure.isEmpty()) {
    m_studyHook->setNoticeText(failure);
    return;
  }
  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodCredentialsSet),
                         dsh::study::credentialsSetPayload(ref, value.trimmed()),
                         [this](bool ok, QJsonValue resultOrError) {
                           if (!ok) {
                             m_studyHook->setNoticeText(dsh::study::rpcErrorMessage(resultOrError));
                             return;
                           }
                           m_studyHook->setNoticeText(QStringLiteral("密钥已保存"));
                           m_studyHook->setOnboardingOpen(false);
                           loadHostCatalog();
                         });
}

void Application::onPermissionRequested(const QString &preset) {
  onSendRequested(QStringLiteral("/permission ") + preset);
}

void Application::onPlanToggleRequested() {
  onSendRequested(m_studyHook->planActive() ? QStringLiteral("/plan off") : QStringLiteral("/plan"));
}

void Application::onPresetRequested(const QString &id) {
  const QString sessionId = m_studyHook->selectedSessionId();
  if (!m_connectionHook->connected() || sessionId.isEmpty()) {
    m_studyHook->setNoticeText(QStringLiteral("请先选择会话"));
    return;
  }
  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodAgentPresetSelect),
                         dsh::study::agentPresetSelectPayload(sessionId, id),
                         [this](bool ok, QJsonValue resultOrError) {
                           if (!ok) {
                             m_studyHook->setNoticeText(dsh::study::rpcErrorMessage(resultOrError));
                             return;
                           }
                           m_studyHook->setNoticeText(QStringLiteral("已选用预设"));
                           loadSessionExtras(m_studyHook->selectedSessionId());
                         });
}

void Application::onAttachRequested(const QUrl &url) {
  const QString path = url.isLocalFile() ? url.toLocalFile() : url.toString();
  const QString mediaType = dsh::study::imageMediaType(path);
  if (mediaType.isEmpty()) {
    m_studyHook->setNoticeText(QStringLiteral("只接受 png、jpg、webp、gif"));
    return;
  }
  QVariantList current = m_studyHook->attachments();
  int maxCount = m_imageLimits.value(QStringLiteral("maxImagesPerMessage")).toInt();
  if (maxCount <= 0) {
    maxCount = 20;
  }
  if (current.size() >= maxCount) {
    m_studyHook->setNoticeText(QStringLiteral("附页已满"));
    return;
  }
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    m_studyHook->setNoticeText(QStringLiteral("打不开这张图"));
    return;
  }
  const QByteArray bytes = file.readAll();
  int maxBytes = m_imageLimits.value(QStringLiteral("maxImageBytes")).toInt();
  if (maxBytes <= 0) {
    maxBytes = 5 * 1024 * 1024;
  }
  if (bytes.size() > maxBytes) {
    m_studyHook->setNoticeText(QStringLiteral("图像过大"));
    return;
  }
  QVariantMap row;
  row.insert(QStringLiteral("name"), QFileInfo(path).fileName());
  row.insert(QStringLiteral("mediaType"), mediaType);
  row.insert(QStringLiteral("data"), QString::fromLatin1(bytes.toBase64()));
  row.insert(QStringLiteral("bytes"), bytes.size());
  current.append(row);
  m_studyHook->setAttachments(current);
}

void Application::onAttachmentRemoveRequested(int index) {
  QVariantList current = m_studyHook->attachments();
  if (index < 0 || index >= current.size()) {
    return;
  }
  current.removeAt(index);
  m_studyHook->setAttachments(current);
}

void Application::onSlashPicked(const QString &line) {
  const QString trimmed = line.trimmed();
  if (!trimmed.isEmpty()) {
    onSendRequested(trimmed);
  }
}

void Application::onSubagentInterruptRequested(const QString &childId) {
  const QString parentId = m_studyHook->selectedSessionId();
  if (!m_connectionHook->connected() || parentId.isEmpty()) {
    return;
  }
  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodSubagentInterrupt),
                         dsh::study::subagentInterruptPayload(parentId, childId),
                         [this, parentId](bool ok, QJsonValue resultOrError) {
                           if (!ok) {
                             m_studyHook->setNoticeText(dsh::study::rpcErrorMessage(resultOrError));
                             return;
                           }
                           loadSessionExtras(parentId);
                         });
}

void Application::onOnboardingKeyRequested(const QString &key) {
  onCredentialSetRequested(m_studyHook->onboardingKeyRef(), key);
}
