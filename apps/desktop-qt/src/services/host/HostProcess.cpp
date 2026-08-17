#include "HostProcess.h"
#include "constants/AppConstants.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDebug>
#include <QtGlobal>

HostProcess::HostProcess(QObject *parent)
    : QObject(parent)
{
}

HostProcess::~HostProcess()
{
    stop();
}

bool HostProcess::start(QString *errorMessage)
{
    if (isRunning() && m_ready) {
        return true;
    }

    stop();

    QStringList prependArgs;
    const QString program = resolveProgram(&prependArgs);
    const QString repoRoot = findRepoRoot();

    QStringList fullArgs = prependArgs;
    fullArgs.append(buildArguments(AppConstants::kDefaultPort));

    if (!m_process) {
        m_process = new QProcess(this);
        m_process->setProcessChannelMode(QProcess::MergedChannels);
        connect(m_process, &QProcess::readyRead, this, &HostProcess::onReadyReadStandardOutput);
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &HostProcess::onProcessFinished);
        connect(m_process, &QProcess::errorOccurred, this, &HostProcess::onProcessErrorOccurred);
    }

    m_process->setWorkingDirectory(repoRoot);
    m_process->setProgram(program);
    m_process->setArguments(fullArgs);
    m_process->setProcessEnvironment(QProcessEnvironment::systemEnvironment());

    m_port = 0;
    m_ready = false;
    m_running = false;
    m_outputBuffer.clear();

    m_process->start();
    if (!m_process->waitForStarted(5000)) {
        const QString err = QStringLiteral("无法启动宿主进程「%1」")
            .arg(program);
        if (errorMessage) {
            *errorMessage = err;
        }
        emit errorOccurred(err);
        return false;
    }

    m_running = true;
    emit started();

    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < AppConstants::kHostReadyTimeoutMs) {
        if (!m_running || m_process->state() == QProcess::NotRunning) {
            onReadyReadStandardOutput();
            const QString err = QStringLiteral("宿主进程过早退出，退出码 %1")
                .arg(m_process->exitCode());
            if (errorMessage) {
                *errorMessage = err;
            }
            emit errorOccurred(err);
            return false;
        }

        if (m_ready && m_port > 0) {
            return true;
        }

        m_process->waitForReadyRead(100);
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);

        if (m_ready && m_port > 0) {
            return true;
        }
    }

    const QString timeoutErr = QStringLiteral("等待宿主就绪超时（%1 毫秒）")
        .arg(AppConstants::kHostReadyTimeoutMs);
    if (errorMessage) {
        *errorMessage = timeoutErr;
    }
    emit errorOccurred(timeoutErr);
    stop();
    return false;
}

void HostProcess::stop()
{
    if (!m_process || m_process->state() == QProcess::NotRunning) {
        m_running = false;
        m_ready = false;
        m_port = 0;
        return;
    }

    m_process->terminate();
    if (!m_process->waitForFinished(AppConstants::kHostStopTimeoutMs)) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }

    m_running = false;
    m_ready = false;
    m_port = 0;
    emit stopped();
}

quint16 HostProcess::port() const
{
    return m_port;
}

bool HostProcess::isRunning() const
{
    return m_process && (m_process->state() == QProcess::Running);
}

QStringList HostProcess::buildArguments(quint16 port)
{
    QStringList args;
    args << QStringLiteral("--profile") << AppConstants::kProfileWeb;
    args << QStringLiteral("--host") << AppConstants::kDefaultHost;
    args << QStringLiteral("--port") << QString::number(port);
    return args;
}

quint16 HostProcess::parsePortFromOutput(const QString &text)
{
    static const QRegularExpression regex(QStringLiteral(R"(dsh web:\s*https?://(?:127\.0\.0\.1|localhost):(\d+))"));
    const auto match = regex.match(text);
    if (match.hasMatch()) {
        bool ok = false;
        const quint16 p = match.captured(1).toUShort(&ok);
        if (ok && p > 0) {
            return p;
        }
    }
    return 0;
}

QString HostProcess::resolveProgram(QStringList *prependArgs)
{
    const QString envPath = qEnvironmentVariable("DSH_DESKTOP_DSH_PATH");
    if (!envPath.isEmpty()) {
        QFileInfo fi(envPath);
        if (fi.exists()) {
            if (envPath.endsWith(QStringLiteral(".ts"), Qt::CaseInsensitive)) {
                QString node = QStandardPaths::findExecutable(QStringLiteral("node"));
                if (node.isEmpty()) node = QStringLiteral("node");
                if (prependArgs) {
                    *prependArgs << QStringLiteral("--import") << QStringLiteral("tsx/esm")
                                 << QDir::toNativeSeparators(fi.absoluteFilePath());
                }
                return node;
            }
            if (envPath.endsWith(QStringLiteral(".js"), Qt::CaseInsensitive)) {
                QString node = QStandardPaths::findExecutable(QStringLiteral("node"));
                if (node.isEmpty()) node = QStringLiteral("node");
                if (prependArgs) {
                    *prependArgs << QDir::toNativeSeparators(fi.absoluteFilePath());
                }
                return node;
            }
            return fi.absoluteFilePath();
        }
    }

    const QString repoRoot = findRepoRoot();

    const QString builtBin = repoRoot + QStringLiteral("/apps/cli/lib/bin.js");
    if (QFileInfo::exists(builtBin)) {
        QString node = QStandardPaths::findExecutable(QStringLiteral("node"));
        if (node.isEmpty()) node = QStringLiteral("node");
        if (prependArgs) {
            *prependArgs << QDir::toNativeSeparators(builtBin);
        }
        return node;
    }

    const QString srcBin = repoRoot + QStringLiteral("/apps/cli/src/bin.ts");
    if (QFileInfo::exists(srcBin)) {
        QString node = QStandardPaths::findExecutable(QStringLiteral("node"));
        if (node.isEmpty()) node = QStringLiteral("node");
        if (prependArgs) {
            *prependArgs << QStringLiteral("--import") << QStringLiteral("tsx/esm")
                         << QDir::toNativeSeparators(srcBin);
        }
        return node;
    }

    QString dshInPath = QStandardPaths::findExecutable(QStringLiteral("dsh"));
    if (!dshInPath.isEmpty()) {
        return dshInPath;
    }

    QString pnpmInPath = QStandardPaths::findExecutable(QStringLiteral("pnpm"));
    if (!pnpmInPath.isEmpty()) {
        if (prependArgs) {
            *prependArgs << QStringLiteral("dsh");
        }
        return pnpmInPath;
    }

    return QStringLiteral("dsh");
}

QString HostProcess::findRepoRoot()
{
    QList<QDir> searchDirs;
    searchDirs << QDir(QCoreApplication::applicationDirPath());
    searchDirs << QDir::current();

    for (QDir dir : searchDirs) {
        for (int i = 0; i < 8; ++i) {
            if (dir.exists(QStringLiteral("pnpm-workspace.yaml")) ||
                dir.exists(QStringLiteral("package.json"))) {
                if (dir.exists(QStringLiteral("pnpm-workspace.yaml")) ||
                    dir.exists(QStringLiteral("apps/cli"))) {
                    return dir.absolutePath();
                }
            }
            if (!dir.cdUp()) {
                break;
            }
        }
    }

    return QDir::currentPath();
}

void HostProcess::onReadyReadStandardOutput()
{
    if (!m_process) return;
    const QByteArray data = m_process->readAll();
    if (!data.isEmpty()) {
        processOutputBuffer(data);
    }
}

void HostProcess::processOutputBuffer(const QByteArray &data)
{
    m_outputBuffer.append(data);
    int newlineIdx;
    while ((newlineIdx = m_outputBuffer.indexOf('\n')) != -1) {
        QByteArray rawLine = m_outputBuffer.left(newlineIdx);
        m_outputBuffer.remove(0, newlineIdx + 1);
        QString line = QString::fromUtf8(rawLine).trimmed();
        if (!line.isEmpty()) {
            qInfo().noquote() << QStringLiteral("host") << line;
            emit logReceived(line);
            if (!m_ready) {
                const quint16 detectedPort = parsePortFromOutput(line);
                if (detectedPort > 0) {
                    m_port = detectedPort;
                    m_ready = true;
                    emit ready(m_port);
                }
            }
        }
    }
}

void HostProcess::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);
    onReadyReadStandardOutput();
    m_running = false;
    m_ready = false;
    emit stopped();
}

void HostProcess::onProcessErrorOccurred(QProcess::ProcessError error)
{
    QString err;
    switch (error) {
        case QProcess::FailedToStart:
            err = QStringLiteral("无法启动宿主进程");
            break;
        case QProcess::Crashed:
            err = QStringLiteral("宿主进程已崩溃");
            break;
        case QProcess::Timedout:
            err = QStringLiteral("宿主进程等待超时");
            break;
        case QProcess::WriteError:
            err = QStringLiteral("无法向宿主进程写入");
            break;
        case QProcess::ReadError:
            err = QStringLiteral("无法读取宿主进程输出");
            break;
        case QProcess::UnknownError:
        default:
            err = QStringLiteral("宿主进程出错");
            break;
    }
    emit errorOccurred(err);
}
