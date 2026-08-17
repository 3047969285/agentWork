#include "app/Application.h"

#include "app/WindowController.h"
#include "constants/AppConstants.h"
#include "hooks/ConnectionHook.h"
#include "hooks/StudyHook.h"
#include "services/host/HostProcess.h"
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

constexpr int kHistoryPollCount = 20;
constexpr int kHistoryPollMs = 1000;

}  // namespace

Application::Application(QObject *parent)
    : QObject(parent),
      m_hostProcess(new HostProcess(this)),
      m_rpcClient(new RpcClient(this)),
      m_connectionHook(new ConnectionHook(this)),
      m_studyHook(new StudyHook(this)),
      m_windowController(new WindowController(this)) {
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
}

Application::~Application() {
  stop();
}

bool Application::init() {
  return m_windowController->init(m_connectionHook, m_studyHook);
}

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

void Application::loadStudy() {
  if (m_isStopping || !m_connectionHook->connected()) {
    return;
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
  const QJsonObject workspace = dsh::study::firstWorkspace(workspaceList);
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
  m_studyHook->setSessions(rows);

  QString selected = m_studyHook->selectedSessionId();
  bool stillListed = false;
  for (const QVariant &row : rows) {
    if (row.toMap().value(QStringLiteral("sessionId")).toString() == selected) {
      stillListed = true;
      break;
    }
  }
  if (!stillListed) {
    selected = rows.isEmpty() ? QString() : rows.at(0).toMap().value(QStringLiteral("sessionId")).toString();
  }
  m_studyHook->setSelectedSessionId(selected);
  m_studyHook->setSelectedTitle(titleForSession(selected));
  if (selected.isEmpty()) {
    m_studyHook->setMessages({});
    return;
  }
  loadHistory(selected);
}

QString Application::titleForSession(const QString &sessionId) const {
  if (sessionId.isEmpty()) {
    return {};
  }
  for (const QVariant &row : m_studyHook->sessions()) {
    const QVariantMap map = row.toMap();
    if (map.value(QStringLiteral("sessionId")).toString() == sessionId) {
      return map.value(QStringLiteral("title")).toString();
    }
  }
  return sessionId.left(8);
}

void Application::onSelectRequested(const QString &sessionId) {
  if (!m_connectionHook->connected()) {
    m_studyHook->setNoticeText(QStringLiteral("尚未连接"));
    return;
  }
  m_pollRemaining = 0;
  m_studyHook->setSending(false);
  m_studyHook->setNoticeText({});
  m_studyHook->setSelectedSessionId(sessionId);
  m_studyHook->setSelectedTitle(titleForSession(sessionId));
  loadHistory(sessionId);
}

void Application::onCreateRequested() {
  if (!m_connectionHook->connected()) {
    m_studyHook->setNoticeText(QStringLiteral("尚未连接"));
    return;
  }
  const QString reusable = dsh::study::blankSessionId(m_studyHook->sessions());
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

  int maxSeq = 0;
  for (const QVariant &row : m_studyHook->messages()) {
    maxSeq = qMax(maxSeq, row.toMap().value(QStringLiteral("seq")).toInt());
  }
  m_pollAfterSeq = maxSeq;

  QVariantList optimistic = m_studyHook->messages();
  QVariantMap userRow;
  userRow.insert(QStringLiteral("role"), QStringLiteral("user"));
  userRow.insert(QStringLiteral("text"), text);
  userRow.insert(QStringLiteral("seq"), -1);
  optimistic.append(userRow);
  m_studyHook->setMessages(optimistic);

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
                           }
                           beginHistoryPoll(sessionId);
                         });
}

void Application::onRefreshRequested() { loadStudy(); }

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
                           m_studyHook->setMessages(dsh::study::messageRows(events));
                         });
}

void Application::beginHistoryPoll(const QString &sessionId) {
  m_pollRemaining = kHistoryPollCount;
  pollHistoryTick(sessionId);
}

void Application::pollHistoryTick(const QString &sessionId) {
  if (m_isStopping || m_studyHook->selectedSessionId() != sessionId) {
    m_studyHook->setSending(false);
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
                           if (ok) {
                             const QJsonArray events =
                                 resultOrError.toObject().value(QStringLiteral("events")).toArray();
                             const QVariantList rows = dsh::study::messageRows(events);
                             m_studyHook->setMessages(rows);
                             bool sawNewAssistant = false;
                             for (const QVariant &row : rows) {
                               const QVariantMap map = row.toMap();
                               if (map.value(QStringLiteral("role")).toString() == QLatin1String("assistant") &&
                                   map.value(QStringLiteral("seq")).toInt() > m_pollAfterSeq) {
                                 sawNewAssistant = true;
                                 break;
                               }
                             }
                             if (sawNewAssistant) {
                               m_pollRemaining = 0;
                               m_studyHook->setSending(false);
                               loadStudy();
                               return;
                             }
                           }
                           if (--m_pollRemaining <= 0) {
                             m_studyHook->setSending(false);
                             loadStudy();
                             return;
                           }
                           QTimer::singleShot(kHistoryPollMs, this, [this, sessionId]() { pollHistoryTick(sessionId); });
                         });
}
