#include "app/Application.h"

#include "app/WindowController.h"
#include "constants/AppConstants.h"
#include "hooks/ConnectionHook.h"
#include "services/host/HostProcess.h"
#include "services/rpc/RpcClient.h"
#include "services/rpc/RpcTypes.h"

#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonValue>

Application::Application(QObject *parent)
    : QObject(parent),
      m_hostProcess(new HostProcess(this)),
      m_rpcClient(new RpcClient(this)),
      m_connectionHook(new ConnectionHook(this)),
      m_windowController(new WindowController(this)) {
  connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &Application::stop);

  connect(m_hostProcess, &HostProcess::started, this, &Application::onHostStarted);
  connect(m_hostProcess, &HostProcess::ready, this, &Application::onHostReady);
  connect(m_hostProcess, &HostProcess::stopped, this, &Application::onHostStopped);
  connect(m_hostProcess, &HostProcess::errorOccurred, this, &Application::onHostError);

  connect(m_connectionHook, &ConnectionHook::retryRequested, this, &Application::onRetryRequested);
}

Application::~Application() {
  stop();
}

bool Application::init() {
  return m_windowController->init(m_connectionHook);
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

  // Defer spawn until the event loop is running so the ink shell can paint
  // "正在启动宿主进程…" before HostProcess::start() blocks on ready output.
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
  m_connectionHook->setStatusText(QStringLiteral("宿主已就绪 (端口 %1)，正在握手…").arg(port));
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

                             const QString summary =
                                 QStringLiteral("已连接 · DeepSeek Harness v%1 (端口 %2)").arg(version).arg(m_port);
                             m_connectionHook->setStatusText(summary);
                           } else {
                             QString errorMsg = QStringLiteral("host.describe 失败");
                             if (resultOrError.isString()) {
                               errorMsg = resultOrError.toString();
                             } else if (resultOrError.isObject()) {
                               const QJsonObject errObj = resultOrError.toObject();
                               if (errObj.contains(QStringLiteral("message"))) {
                                 errorMsg = errObj.value(QStringLiteral("message")).toString();
                               }
                             }
                             scheduleRetry(errorMsg);
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
    m_connectionHook->setStatusText(QStringLiteral("连接失败: %1 (已达最大重试次数)").arg(reason));
  }
}
