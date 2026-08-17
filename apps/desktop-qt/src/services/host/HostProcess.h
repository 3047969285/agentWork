#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QByteArray>

class HostProcess : public QObject {
    Q_OBJECT

public:
    explicit HostProcess(QObject *parent = nullptr);
    ~HostProcess() override;

    bool start(QString *errorMessage = nullptr);
    void stop();

    quint16 port() const;
    bool isRunning() const;

    static QStringList buildArguments(quint16 port = 0);
    static quint16 parsePortFromOutput(const QString &text);
    static QString resolveProgram(QStringList *prependArgs = nullptr);
    static QString findRepoRoot();

signals:
    void started();
    void ready(quint16 port);
    void stopped();
    void errorOccurred(const QString &error);
    void logReceived(const QString &line);

private slots:
    void onReadyReadStandardOutput();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessErrorOccurred(QProcess::ProcessError error);

private:
    void processOutputBuffer(const QByteArray &data);

    QProcess *m_process = nullptr;
    quint16 m_port = 0;
    bool m_running = false;
    bool m_ready = false;
    QByteArray m_outputBuffer;
};
