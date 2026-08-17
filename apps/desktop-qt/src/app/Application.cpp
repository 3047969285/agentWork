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
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QSet>
#include <QtGlobal>
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

                             QString version = QStringLiteral("dev");
                             if (resultOrError.isObject()) {
                               const QJsonObject obj = resultOrError.toObject();
                               if (obj.contains(QStringLiteral("version")) &&
                                   !obj.value(QStringLiteral("version")).toString().isEmpty()) {
                                 version = obj.value(QStringLiteral("version")).toString();
                               }
                             }

                             m_connectionHook->setStatusText(
                                 QStringLiteral("已连接 · v%1 · %2").arg(version).arg(m_port));
                             connectStreams();
                             loadStudy();
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
  m_studyHook->setPendingApproval({});
  m_studyHook->setPendingQuestion({});
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

  m_studyHook->setSending(true);
  m_studyHook->setNoticeText({});
  m_studyHook->transcriptModel()->appendUser(text);

  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodSessionPrompt),
                         dsh::study::promptPayload(sessionId, text),
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

void Application::onRefreshRequested() { loadStudy(); }

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
  m_rpcClient->callUnary(QString::fromLatin1(dsh::rpc::kMethodSettingsDescribe), QJsonObject{},
                         [this](bool ok, QJsonValue resultOrError) {
                           if (!ok) {
                             m_studyHook->setNoticeText(dsh::study::rpcErrorMessage(resultOrError));
                             return;
                           }
                           const QJsonObject value = resultOrError.toObject();
                           m_studyHook->setSettingsWritable(value.value(QStringLiteral("writable")).toBool());
                           m_studyHook->setSettingsHasDocument(value.value(QStringLiteral("hasDocument")).toBool());
                           m_studyHook->setSettingsNamespaces(dsh::study::settingsNamespaces(value));
                         });
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
  }
}
