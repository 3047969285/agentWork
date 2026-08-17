#pragma once

#include <QObject>
#include <QString>

class ConnectionHook : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString statusText READ statusText WRITE setStatusText NOTIFY statusTextChanged)
  Q_PROPERTY(bool connected READ connected WRITE setConnected NOTIFY connectedChanged)
  Q_PROPERTY(bool connecting READ connecting WRITE setConnecting NOTIFY connectingChanged)
  Q_PROPERTY(bool hasError READ hasError WRITE setHasError NOTIFY hasErrorChanged)

 public:
  explicit ConnectionHook(QObject *parent = nullptr);

  QString statusText() const;
  bool connected() const;
  bool connecting() const;
  bool hasError() const;

  void setStatusText(const QString &text);
  void setConnected(bool connected);
  void setConnecting(bool connecting);
  void setHasError(bool hasError);

  Q_INVOKABLE void retry();

 signals:
  void statusTextChanged(const QString &statusText);
  void connectedChanged(bool connected);
  void connectingChanged(bool connecting);
  void hasErrorChanged(bool hasError);
  void retryRequested();

 private:
  QString m_statusText;
  bool m_connected = false;
  bool m_connecting = false;
  bool m_hasError = false;
};
