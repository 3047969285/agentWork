#pragma once

#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QUrl>

#include <functional>

class QNetworkAccessManager;

class RpcClient : public QObject {
  Q_OBJECT

 public:
  explicit RpcClient(QObject *parent = nullptr);

  void setBaseUrl(const QUrl &baseUrl);
  QUrl baseUrl() const;

  void callUnary(const QString &method, const QJsonValue &payload,
                 const std::function<void(bool ok, QJsonValue resultOrError)> &done);

  static QJsonObject makeClientRequest(const QString &method, const QJsonValue &payload);
  static bool parseServerResponse(const QJsonObject &response, const QString &expectedRpcId,
                                  bool *ok, QJsonValue *resultOrError, QString *errorMessage);

 private:
  QUrl unaryUrlForMethod(const QString &method) const;

  QUrl m_baseUrl;
  QNetworkAccessManager *m_network = nullptr;
};
