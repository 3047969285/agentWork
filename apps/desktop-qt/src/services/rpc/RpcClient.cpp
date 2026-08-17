#include "services/rpc/RpcClient.h"

#include "services/rpc/RpcTypes.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUuid>

namespace {

QString mintRpcId() { return QUuid::createUuid().toString(QUuid::WithoutBraces); }

}  // namespace

RpcClient::RpcClient(QObject *parent) : QObject(parent), m_network(new QNetworkAccessManager(this)) {}

void RpcClient::setBaseUrl(const QUrl &baseUrl) { m_baseUrl = baseUrl; }

QUrl RpcClient::baseUrl() const { return m_baseUrl; }

QJsonObject RpcClient::makeClientRequest(const QString &method, const QJsonValue &payload) {
  QJsonObject request;
  request.insert(QString::fromLatin1(dsh::rpc::kFieldType), QString::fromLatin1(dsh::rpc::kClientRequestType));
  request.insert(QString::fromLatin1(dsh::rpc::kFieldRpcId), mintRpcId());
  request.insert(QString::fromLatin1(dsh::rpc::kFieldMethod), method);
  request.insert(QString::fromLatin1(dsh::rpc::kFieldPayload), payload);
  return request;
}

bool RpcClient::parseServerResponse(const QJsonObject &response, const QString &expectedRpcId, bool *ok,
                                    QJsonValue *resultOrError, QString *errorMessage) {
  if (errorMessage != nullptr) {
    errorMessage->clear();
  }

  const QString type = response.value(QString::fromLatin1(dsh::rpc::kFieldType)).toString();
  if (type != QLatin1String(dsh::rpc::kServerResponseType)) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("unexpected response type: %1").arg(type);
    }
    return false;
  }

  const QString rpcId = response.value(QString::fromLatin1(dsh::rpc::kFieldRpcId)).toString();
  if (rpcId != expectedRpcId) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("rpcId mismatch: expected %1, got %2").arg(expectedRpcId, rpcId);
    }
    return false;
  }

  if (!response.contains(QString::fromLatin1(dsh::rpc::kFieldResult)) ||
      !response.value(QString::fromLatin1(dsh::rpc::kFieldResult)).isObject()) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("missing result object");
    }
    return false;
  }

  const QJsonObject result = response.value(QString::fromLatin1(dsh::rpc::kFieldResult)).toObject();
  if (!result.contains(QString::fromLatin1(dsh::rpc::kFieldOk))) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("missing result.ok");
    }
    return false;
  }

  const bool businessOk = result.value(QString::fromLatin1(dsh::rpc::kFieldOk)).toBool();
  if (ok != nullptr) {
    *ok = businessOk;
  }
  if (resultOrError != nullptr) {
    *resultOrError =
        businessOk ? result.value(QString::fromLatin1(dsh::rpc::kFieldValue)) : result.value(QString::fromLatin1(dsh::rpc::kFieldError));
  }
  return true;
}

QUrl RpcClient::unaryUrlForMethod(const QString &method) const {
  // Wire carrier matches packages/host/apiproxy/src/fetch/client.ts callUnary:
  // POST `/api/${method}` (method is the RpcMethodMap key, e.g. host.describe).
  // packages/client/connection/src/api-path.ts defines API_PATH = '/api'.
  const QString path = QString::fromLatin1(dsh::rpc::kApiPathPrefix) + QLatin1Char('/') + method;
  return m_baseUrl.resolved(QUrl(path));
}

void RpcClient::callUnary(const QString &method, const QJsonValue &payload,
                          const std::function<void(bool ok, QJsonValue resultOrError)> &done) {
  if (!m_baseUrl.isValid()) {
    done(false, QStringLiteral("RpcClient base URL is not set"));
    return;
  }

  const QJsonObject requestObject = makeClientRequest(method, payload);
  const QString rpcId = requestObject.value(QString::fromLatin1(dsh::rpc::kFieldRpcId)).toString();
  const QUrl url = unaryUrlForMethod(method);

  QNetworkRequest networkRequest(url);
  networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

  const QByteArray body = QJsonDocument(requestObject).toJson(QJsonDocument::Compact);
  QNetworkReply *reply = m_network->post(networkRequest, body);

  connect(reply, &QNetworkReply::finished, this, [reply, rpcId, done]() {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
      done(false, reply->errorString());
      return;
    }

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode < 200 || statusCode >= 300) {
      done(false, QStringLiteral("HTTP %1").arg(statusCode));
      return;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
      done(false, QStringLiteral("invalid JSON response: %1").arg(parseError.errorString()));
      return;
    }

    bool businessOk = false;
    QJsonValue resultOrError;
    QString parseMessage;
    if (!parseServerResponse(document.object(), rpcId, &businessOk, &resultOrError, &parseMessage)) {
      done(false, parseMessage);
      return;
    }

    done(businessOk, resultOrError);
  });
}
