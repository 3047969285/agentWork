#pragma once

#include <QObject>
#include <QString>

class ConnectionHook : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString statusText READ statusText WRITE setStatusText NOTIFY statusTextChanged)
  Q_PROPERTY(QString hostVersion READ hostVersion NOTIFY hostVersionChanged)
  Q_PROPERTY(int hostPort READ hostPort NOTIFY hostPortChanged)
  Q_PROPERTY(bool connected READ connected WRITE setConnected NOTIFY connectedChanged)
  Q_PROPERTY(bool connecting READ connecting WRITE setConnecting NOTIFY connectingChanged)
  Q_PROPERTY(bool hasError READ hasError WRITE setHasError NOTIFY hasErrorChanged)

 public:
  explicit ConnectionHook(QObject *parent = nullptr);

  QString statusText() const;
  QString hostVersion() const;
  int hostPort() const;
  bool connected() const;
  bool connecting() const;
  bool hasError() const;

  void setStatusText(const QString &text);
  void setHostVersion(const QString &version);
  void setHostPort(int port);
  void setConnected(bool connected);
  void setConnecting(bool connecting);
  void setHasError(bool hasError);

  Q_INVOKABLE void retry();

 signals:
  void statusTextChanged(const QString &statusText);
  void hostVersionChanged();
  void hostPortChanged();
  void connectedChanged(bool connected);
  void connectingChanged(bool connecting);
  void hasErrorChanged(bool hasError);
  void retryRequested();

 private:
  QString m_statusText;
  QString m_hostVersion;
  int m_hostPort = 0;
  bool m_connected = false;
  bool m_connecting = false;
  bool m_hasError = false;
};
