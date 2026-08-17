#include <QEventLoop>
#include <QTimer>
#include <QUrl>
#include <QtTest>

#include "services/rpc/RpcClient.h"
#include "services/rpc/RpcTypes.h"

class TstRpcEnvelope : public QObject {
  Q_OBJECT

 private slots:
  void makeClientRequest_hasExpectedEnvelopeFields() {
    const QJsonObject request = RpcClient::makeClientRequest(QString::fromLatin1(dsh::rpc::kMethodHostDescribe), QJsonObject{});

    QCOMPARE(request.value(QString::fromLatin1(dsh::rpc::kFieldType)).toString(),
             QString::fromLatin1(dsh::rpc::kClientRequestType));
    QVERIFY(!request.value(QString::fromLatin1(dsh::rpc::kFieldRpcId)).toString().isEmpty());
    QCOMPARE(request.value(QString::fromLatin1(dsh::rpc::kFieldMethod)).toString(),
             QString::fromLatin1(dsh::rpc::kMethodHostDescribe));
    QCOMPARE(request.value(QString::fromLatin1(dsh::rpc::kFieldPayload)).toObject(), QJsonObject{});
  }

  void parseServerResponse_readsOkValue() {
    const QJsonObject request = RpcClient::makeClientRequest(QString::fromLatin1(dsh::rpc::kMethodHostDescribe), QJsonObject{});
    const QString rpcId = request.value(QString::fromLatin1(dsh::rpc::kFieldRpcId)).toString();

    QJsonObject response;
    response.insert(QString::fromLatin1(dsh::rpc::kFieldType), QString::fromLatin1(dsh::rpc::kServerResponseType));
    response.insert(QString::fromLatin1(dsh::rpc::kFieldRpcId), rpcId);
    QJsonObject result;
    result.insert(QString::fromLatin1(dsh::rpc::kFieldOk), true);
    result.insert(QString::fromLatin1(dsh::rpc::kFieldValue), QJsonObject{{QStringLiteral("version"), QStringLiteral("test")}});
    response.insert(QString::fromLatin1(dsh::rpc::kFieldResult), result);

    bool ok = false;
    QJsonValue resultOrError;
    QString errorMessage;
    QVERIFY(RpcClient::parseServerResponse(response, rpcId, &ok, &resultOrError, &errorMessage));
    QVERIFY(ok);
    QCOMPARE(resultOrError.toObject().value(QStringLiteral("version")).toString(), QStringLiteral("test"));
    QVERIFY(errorMessage.isEmpty());
  }

  void parseServerResponse_readsBusinessError() {
    const QJsonObject request = RpcClient::makeClientRequest(QString::fromLatin1(dsh::rpc::kMethodHostDescribe), QJsonObject{});
    const QString rpcId = request.value(QString::fromLatin1(dsh::rpc::kFieldRpcId)).toString();

    QJsonObject response;
    response.insert(QString::fromLatin1(dsh::rpc::kFieldType), QString::fromLatin1(dsh::rpc::kServerResponseType));
    response.insert(QString::fromLatin1(dsh::rpc::kFieldRpcId), rpcId);
    QJsonObject result;
    result.insert(QString::fromLatin1(dsh::rpc::kFieldOk), false);
    result.insert(QString::fromLatin1(dsh::rpc::kFieldError),
                  QJsonObject{{QStringLiteral("code"), QStringLiteral("internal")},
                              {QStringLiteral("message"), QStringLiteral("boom")},
                              {QStringLiteral("details"), QJsonObject{}}});
    response.insert(QString::fromLatin1(dsh::rpc::kFieldResult), result);

    bool ok = true;
    QJsonValue resultOrError;
    QString errorMessage;
    QVERIFY(RpcClient::parseServerResponse(response, rpcId, &ok, &resultOrError, &errorMessage));
    QVERIFY(!ok);
    QCOMPARE(resultOrError.toObject().value(QStringLiteral("code")).toString(), QStringLiteral("internal"));
    QVERIFY(errorMessage.isEmpty());
  }

  void makeClientResponse_echoesRpcIdAndValue() {
    const QJsonObject response =
        RpcClient::makeClientResponse(QStringLiteral("rpc-1"), QJsonObject{{QStringLiteral("outcome"), QStringLiteral("allowed-once")}});
    QCOMPARE(response.value(QString::fromLatin1(dsh::rpc::kFieldType)).toString(),
             QString::fromLatin1(dsh::rpc::kClientResponseType));
    QCOMPARE(response.value(QString::fromLatin1(dsh::rpc::kFieldRpcId)).toString(), QStringLiteral("rpc-1"));
    const QJsonObject result = response.value(QString::fromLatin1(dsh::rpc::kFieldResult)).toObject();
    QVERIFY(result.value(QString::fromLatin1(dsh::rpc::kFieldOk)).toBool());
    QCOMPARE(result.value(QString::fromLatin1(dsh::rpc::kFieldValue)).toObject().value(QStringLiteral("outcome")).toString(),
             QStringLiteral("allowed-once"));
  }

  void hostDescribe_liveOptional() {
    const QByteArray baseUrlEnv = qgetenv("DSH_DESKTOP_RPC_BASE_URL");
    if (baseUrlEnv.isEmpty()) {
      QSKIP("Set DSH_DESKTOP_RPC_BASE_URL (e.g. http://127.0.0.1:3080) to run live host.describe");
    }

    RpcClient client;
    client.setBaseUrl(QUrl(QString::fromUtf8(baseUrlEnv)));

    QEventLoop loop;
    bool finished = false;
    bool callOk = false;
    QJsonValue callResult;
    client.callUnary(QString::fromLatin1(dsh::rpc::kMethodHostDescribe), QJsonObject{}, [&](bool ok, QJsonValue resultOrError) {
      callOk = ok;
      callResult = resultOrError;
      finished = true;
      loop.quit();
    });

    QTimer::singleShot(10000, &loop, &QEventLoop::quit);
    loop.exec();

    if (!finished) {
      QFAIL("host.describe timed out after 10s");
    }
    QVERIFY2(callOk, qPrintable(callResult.toString()));
    QVERIFY(callResult.isObject());
    QVERIFY(callResult.toObject().contains(QStringLiteral("version")));
  }
};

QTEST_MAIN(TstRpcEnvelope)
#include "tst_rpc_envelope.moc"
