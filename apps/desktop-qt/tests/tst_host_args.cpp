#include <QtTest>
#include "constants/AppConstants.h"
#include "services/host/HostProcess.h"

class TstHostArgs : public QObject {
    Q_OBJECT
private slots:
    void buildsLoopbackArgs() {
        const QStringList args = HostProcess::buildArguments(1270);
        QVERIFY(args.contains(QStringLiteral("--host")));
        QVERIFY(args.contains(QStringLiteral("127.0.0.1")));
        QVERIFY(!args.contains(QStringLiteral("0.0.0.0")));
        QVERIFY(args.contains(QStringLiteral("--profile")));
        QVERIFY(args.contains(QStringLiteral("web")));
        QVERIFY(args.contains(QStringLiteral("--port")));
        QVERIFY(args.contains(QStringLiteral("1270")));
    }

    void buildsZeroPortArgs() {
        const QStringList args = HostProcess::buildArguments(0);
        QVERIFY(args.contains(QStringLiteral("--port")));
        QVERIFY(args.contains(QStringLiteral("0")));
        QVERIFY(args.contains(QStringLiteral("127.0.0.1")));
    }

    void parsesPortFromReadyOutput() {
        QCOMPARE(HostProcess::parsePortFromOutput(QStringLiteral("dsh web: http://127.0.0.1:3080")), quint16(3080));
        QCOMPARE(HostProcess::parsePortFromOutput(QStringLiteral("dsh web: http://127.0.0.1:4567 (LAN: http://192.168.1.5:4567)")), quint16(4567));
        QCOMPARE(HostProcess::parsePortFromOutput(QStringLiteral("dsh web: http://localhost:8080")), quint16(8080));
        QCOMPARE(HostProcess::parsePortFromOutput(QStringLiteral("random log line without url")), quint16(0));
    }

    void resolvesProgram() {
        QStringList prependArgs;
        const QString program = HostProcess::resolveProgram(&prependArgs);
        QVERIFY(!program.isEmpty());
    }

    void findsRepoRoot() {
        const QString repoRoot = HostProcess::findRepoRoot();
        QVERIFY(!repoRoot.isEmpty());
        QVERIFY(QDir(repoRoot).exists());
    }

    void liveProcessLifecycle() {
        if (qEnvironmentVariableIsEmpty("DSH_LIVE_TEST")) {
            QSKIP("DSH_LIVE_TEST not set, skipping live process test");
        }

        HostProcess host;
        QString err;
        bool ok = host.start(&err);
        QVERIFY2(ok, qPrintable(err));
        QVERIFY(host.isRunning());
        QVERIFY(host.port() > 0);

        host.stop();
        QVERIFY(!host.isRunning());
    }
};

QTEST_MAIN(TstHostArgs)
#include "tst_host_args.moc"
