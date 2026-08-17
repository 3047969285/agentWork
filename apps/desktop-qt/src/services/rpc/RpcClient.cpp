#include "services/rpc/RpcClient.h"

#include "services/rpc/RpcTypes.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUuid>
#include <QtGlobal>

namespace {

QString mintRpcId() { return QUuid::createUuid().toString(QUuid::WithoutBraces); }

QString networkFailure(QNetworkReply *reply) {
  switch (reply->error()) {
    case QNetworkReply::ConnectionRefusedError:
      return QStringLiteral("连接被拒绝");
    case QNetworkReply::RemoteHostClosedError:
      return QStringLiteral("宿主关闭了连接");
    case QNetworkReply::HostNotFoundError:
      return QStringLiteral("找不到宿主");
    case QNetworkReply::TimeoutError:
      return QStringLiteral("请求超时");
    case QNetworkReply::OperationCanceledError:
      return QStringLiteral("请求已取消");
    case QNetworkReply::SslHandshakeFailedError:
      return QStringLiteral("安全握手失败");
    case QNetworkReply::TemporaryNetworkFailureError:
      return QStringLiteral("网络暂时中断");
    case QNetworkReply::NetworkSessionFailedError:
      return QStringLiteral("网络会话失败");
    case QNetworkReply::BackgroundRequestNotAllowedError:
      return QStringLiteral("不允许后台请求");
    case QNetworkReply::TooManyRedirectsError:
      return QStringLiteral("重定向过多");
    case QNetworkReply::InsecureRedirectError:
      return QStringLiteral("不安全的重定向");
    default:
      return QStringLiteral("网络错误");
  }
}

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

QJsonObject RpcClient::makeClientResponse(const QString &rpcId, const QJsonValue &value) {
  QJsonObject request;
  request.insert(QString::fromLatin1(dsh::rpc::kFieldType), QString::fromLatin1(dsh::rpc::kClientResponseType));
  request.insert(QString::fromLatin1(dsh::rpc::kFieldRpcId), rpcId);
  QJsonObject result;
  result.insert(QString::fromLatin1(dsh::rpc::kFieldOk), true);
  result.insert(QString::fromLatin1(dsh::rpc::kFieldValue), value);
  request.insert(QString::fromLatin1(dsh::rpc::kFieldResult), result);
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
      *errorMessage = QStringLiteral("响应类型异常：%1").arg(type);
    }
    return false;
  }

  const QString rpcId = response.value(QString::fromLatin1(dsh::rpc::kFieldRpcId)).toString();
  if (rpcId != expectedRpcId) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("请求编号不一致：期望 %1，实际 %2").arg(expectedRpcId, rpcId);
    }
    return false;
  }

  if (!response.contains(QString::fromLatin1(dsh::rpc::kFieldResult)) ||
      !response.value(QString::fromLatin1(dsh::rpc::kFieldResult)).isObject()) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("响应缺少结果");
    }
    return false;
  }

  const QJsonObject result = response.value(QString::fromLatin1(dsh::rpc::kFieldResult)).toObject();
  if (!result.contains(QString::fromLatin1(dsh::rpc::kFieldOk))) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("响应缺少结果状态");
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

QUrl RpcClient::unaryUrl(const QUrl &baseUrl, const QString &method) {
  // Do not use QUrl::resolved() with `/api/host.describe`: Qt may treat the
  // dotted segment as a host/suffix and drop the real listen port.
  QUrl url = baseUrl;
  url.setPath(QString::fromLatin1(dsh::rpc::kApiPathPrefix) + QLatin1Char('/') + method);
  url.setQuery(QString());
  url.setFragment(QString());
  return url;
}

QUrl RpcClient::unaryUrlForMethod(const QString &method) const { return unaryUrl(m_baseUrl, method); }

void RpcClient::attachLoopbackOrigin(QNetworkRequest *request) const {
  if (request == nullptr || !m_baseUrl.isValid()) {
    return;
  }
  const int port = m_baseUrl.port();
  const QString origin = port > 0
                             ? QStringLiteral("http://%1:%2").arg(m_baseUrl.host()).arg(port)
                             : QStringLiteral("http://%1").arg(m_baseUrl.host());
  request->setRawHeader(QByteArrayLiteral("Origin"), origin.toLatin1());
}

void RpcClient::callUnary(const QString &method, const QJsonValue &payload,
                          const std::function<void(bool ok, QJsonValue resultOrError)> &done) {
  if (!m_baseUrl.isValid()) {
    done(false, QStringLiteral("尚未设置宿主地址"));
    return;
  }

  const QJsonObject requestObject = makeClientRequest(method, payload);
  const QString rpcId = requestObject.value(QString::fromLatin1(dsh::rpc::kFieldRpcId)).toString();
  const QUrl url = unaryUrlForMethod(method);

  QNetworkRequest networkRequest(url);
  networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
  attachLoopbackOrigin(&networkRequest);

  const QByteArray body = QJsonDocument(requestObject).toJson(QJsonDocument::Compact);
  qInfo().noquote() << QStringLiteral("rpc POST") << url.toString() << method;
  QNetworkReply *reply = m_network->post(networkRequest, body);

  connect(reply, &QNetworkReply::finished, this, [reply, rpcId, method, url, done]() {
    reply->deleteLater();
    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray raw = reply->readAll();
    const QNetworkReply::NetworkError netError = reply->error();

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(raw, &parseError);
    if (parseError.error == QJsonParseError::NoError && document.isObject()) {
      bool businessOk = false;
      QJsonValue resultOrError;
      QString parseMessage;
      if (parseServerResponse(document.object(), rpcId, &businessOk, &resultOrError, &parseMessage)) {
        if (!businessOk) {
          const QString errDump = resultOrError.isObject()
                                      ? QString::fromUtf8(QJsonDocument(resultOrError.toObject()).toJson(QJsonDocument::Compact))
                                      : resultOrError.toString();
          qWarning().noquote() << QStringLiteral("rpc fail") << method << url.toString() << statusCode << errDump;
        }
        done(businessOk, resultOrError);
        return;
      }
      if (statusCode >= 200 && statusCode < 300) {
        qWarning().noquote() << QStringLiteral("rpc parse") << method << parseMessage;
        done(false, parseMessage);
        return;
      }
    }

    QString message;
    if (statusCode == 403) {
      message = QStringLiteral("宿主拒绝了请求（403）");
    } else if (statusCode >= 400) {
      message = QStringLiteral("服务返回状态 %1").arg(statusCode);
    } else if (netError != QNetworkReply::NoError) {
      message = networkFailure(reply);
    } else {
      message = QStringLiteral("响应不是有效 JSON");
    }
    qWarning().noquote() << QStringLiteral("rpc error") << method << url.toString() << statusCode << netError
                         << message << QString::fromUtf8(raw.left(300));
    done(false, message);
  });
}

void RpcClient::callRespond(const QString &rpcId, const QJsonValue &value,
                            const std::function<void(bool accepted, QString reason)> &done) {
  if (!m_baseUrl.isValid()) {
    done(false, QStringLiteral("尚未设置宿主地址"));
    return;
  }
  if (rpcId.isEmpty()) {
    done(false, QStringLiteral("缺少请求编号"));
    return;
  }

  const QJsonObject requestObject = makeClientResponse(rpcId, value);
  QUrl url = m_baseUrl;
  url.setPath(QString::fromLatin1(dsh::rpc::kRespondPath));
  url.setQuery(QString());
  url.setFragment(QString());

  QNetworkRequest networkRequest(url);
  networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
  attachLoopbackOrigin(&networkRequest);
  qInfo().noquote() << QStringLiteral("rpc POST") << url.toString() << QStringLiteral("respond");

  const QByteArray body = QJsonDocument(requestObject).toJson(QJsonDocument::Compact);
  QNetworkReply *reply = m_network->post(networkRequest, body);

  connect(reply, &QNetworkReply::finished, this, [reply, done]() {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
      done(false, networkFailure(reply));
      return;
    }
    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode < 200 || statusCode >= 300) {
      done(false, QStringLiteral("服务返回状态 %1").arg(statusCode));
      return;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
      done(false, QStringLiteral("响应不是有效 JSON"));
      return;
    }
    const QJsonObject receipt = document.object();
    const bool accepted = receipt.value(QStringLiteral("accepted")).toBool();
    done(accepted, accepted ? QString() : receipt.value(QStringLiteral("reason")).toString());
  });
}
