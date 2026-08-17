#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QUrl>

class QTcpSocket;

/**
 * Downlink-only WebSocket pair for `/api/events.mux` and `/api/events.host`.
 * Matches packages/client/connection WebSocketDownlinks: the client never
 * sends application messages; ping/pong and close are protocol-only.
 */
class EventStream : public QObject {
  Q_OBJECT

 public:
  explicit EventStream(QObject *parent = nullptr);
  ~EventStream() override;

  void connectTo(const QUrl &httpBase);
  void disconnectFromHost();
  bool isOpen() const;

 signals:
  void muxFrame(const QString &rpcId, const QJsonObject &payload);
  void hostFrame(const QString &rpcId, const QJsonObject &payload);
  void openChanged(bool open);
  void errorOccurred(const QString &message);

 private:
  class Downlink;

  void emitOpenIfNeeded();
  void emitClosedIfNeeded();

  Downlink *m_mux = nullptr;
  Downlink *m_host = nullptr;
  bool m_openSignaled = false;
};
