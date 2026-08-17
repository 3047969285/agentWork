#pragma once

#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QUrl>

#include <functional>

class QNetworkAccessManager;
class QNetworkRequest;

class RpcClient : public QObject {
  Q_OBJECT

 public:
  explicit RpcClient(QObject *parent = nullptr);

  void setBaseUrl(const QUrl &baseUrl);
  QUrl baseUrl() const;

  void callUnary(const QString &method, const QJsonValue &payload,
                 const std::function<void(bool ok, QJsonValue resultOrError)> &done);

  void callRespond(const QString &rpcId, const QJsonValue &value,
                   const std::function<void(bool accepted, QString reason)> &done);

  static QJsonObject makeClientRequest(const QString &method, const QJsonValue &payload);
  static QJsonObject makeClientResponse(const QString &rpcId, const QJsonValue &value);
  static bool parseServerResponse(const QJsonObject &response, const QString &expectedRpcId,
                                  bool *ok, QJsonValue *resultOrError, QString *errorMessage);
  /** POST URL for `method`; keeps dotted names like `host.describe` as the path. */
  static QUrl unaryUrl(const QUrl &baseUrl, const QString &method);

 private:
  QUrl unaryUrlForMethod(const QString &method) const;
  void attachLoopbackOrigin(QNetworkRequest *request) const;

  QUrl m_baseUrl;
  QNetworkAccessManager *m_network = nullptr;
};
