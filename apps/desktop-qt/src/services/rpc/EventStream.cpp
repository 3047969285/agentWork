#include "services/rpc/EventStream.h"

#include "services/rpc/RpcTypes.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QRandomGenerator>
#include <QTcpSocket>
#include <QtEndian>
#include <QtGlobal>

namespace {

constexpr int kMaxFrameBytes = 1024 * 1024;

QByteArray randomKey() {
  QByteArray raw(16, 0);
  for (int i = 0; i < raw.size(); ++i) {
    raw[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
  }
  return raw.toBase64();
}

QUrl wsUrl(const QUrl &httpBase, const char *path) {
  QUrl url = httpBase;
  // Keep the HTTP listen port. QUrl::setScheme("ws") can reset it to 80.
  const int port = httpBase.port();
  url.setScheme(QStringLiteral("ws"));
  if (port > 0) {
    url.setPort(port);
  }
  url.setPath(QString::fromLatin1(path));
  url.setQuery(QString());
  url.setFragment(QString());
  return url;
}

}  // namespace

class EventStream::Downlink : public QObject {
 public:
  Downlink(EventStream *owner, bool mux)
      : QObject(owner), m_owner(owner), m_mux(mux), m_socket(new QTcpSocket(this)) {
    connect(m_socket, &QTcpSocket::connected, this, &Downlink::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &Downlink::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &Downlink::onDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &Downlink::onError);
  }

  void open(const QUrl &httpBase) {
    close();
    m_url = wsUrl(httpBase, m_mux ? dsh::rpc::kMuxEventsPath : dsh::rpc::kHostEventsPath);
    m_httpOrigin = QStringLiteral("http://%1:%2").arg(httpBase.host()).arg(httpBase.port());
    m_key = randomKey();
    m_handshakeDone = false;
    m_header.clear();
    m_buffer.clear();
    m_textCarry.clear();
    const QString host = httpBase.host();
    const quint16 port = static_cast<quint16>(httpBase.port());
    qInfo().noquote() << QStringLiteral("ws connect") << m_url.toString() << host << port;
    m_socket->connectToHost(host, port);
  }

  void close() {
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
      m_socket->disconnectFromHost();
      if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort();
      }
    }
    m_handshakeDone = false;
    m_header.clear();
    m_buffer.clear();
    m_textCarry.clear();
  }

  bool isOpen() const { return m_handshakeDone && m_socket->state() == QAbstractSocket::ConnectedState; }

 private:
  void onConnected() {
    const QByteArray path = m_url.path(QUrl::FullyEncoded).toUtf8();
    const QByteArray host =
        m_url.host().toUtf8() + ':' + QByteArray::number(m_url.port() > 0 ? m_url.port() : 80);
    QByteArray req;
    req += "GET ";
    req += path.isEmpty() ? QByteArray("/") : path;
    req += " HTTP/1.1\r\n";
    req += "Host: ";
    req += host;
    req += "\r\n";
    req += "Origin: ";
    req += m_httpOrigin.toUtf8();
    req += "\r\n";
    req += "Upgrade: websocket\r\n";
    req += "Connection: Upgrade\r\n";
    req += "Sec-WebSocket-Key: ";
    req += m_key;
    req += "\r\n";
    req += "Sec-WebSocket-Version: 13\r\n";
    req += "\r\n";
    m_socket->write(req);
  }

  void onReadyRead() {
    m_buffer += m_socket->readAll();
    if (!m_handshakeDone) {
      const int split = m_buffer.indexOf("\r\n\r\n");
      if (split < 0) {
        return;
      }
      m_header = m_buffer.left(split);
      m_buffer.remove(0, split + 4);
      if (!m_header.startsWith("HTTP/1.1 101") && !m_header.startsWith("HTTP/1.0 101")) {
        const QString firstLine = QString::fromUtf8(m_header.left(m_header.indexOf('\r')));
        qWarning().noquote() << QStringLiteral("ws handshake") << m_url.toString() << firstLine;
        emit m_owner->errorOccurred(QStringLiteral("事件流握手失败：%1").arg(firstLine));
        close();
        return;
      }
      qInfo().noquote() << QStringLiteral("ws open") << m_url.toString();
      m_handshakeDone = true;
      m_owner->emitOpenIfNeeded();
    }
    while (takeFrame()) {
    }
  }

  void onDisconnected() {
    m_handshakeDone = false;
    m_owner->emitClosedIfNeeded();
  }

  void onError(QAbstractSocket::SocketError error) {
    if (m_socket->state() == QAbstractSocket::UnconnectedState) {
      return;
    }
    QString message;
    switch (error) {
      case QAbstractSocket::ConnectionRefusedError:
        message = QStringLiteral("事件流连接被拒绝");
        break;
      case QAbstractSocket::RemoteHostClosedError:
        message = QStringLiteral("事件流被宿主关闭");
        break;
      case QAbstractSocket::HostNotFoundError:
        message = QStringLiteral("找不到事件流宿主");
        break;
      case QAbstractSocket::SocketTimeoutError:
        message = QStringLiteral("事件流连接超时");
        break;
      case QAbstractSocket::NetworkError:
        message = QStringLiteral("事件流网络错误");
        break;
      default:
        message = QStringLiteral("事件流出错");
        break;
    }
    emit m_owner->errorOccurred(message);
  }

  bool takeFrame() {
    if (m_buffer.size() < 2) {
      return false;
    }
    const auto *bytes = reinterpret_cast<const unsigned char *>(m_buffer.constData());
    const bool fin = (bytes[0] & 0x80) != 0;
    const int opcode = bytes[0] & 0x0f;
    const bool masked = (bytes[1] & 0x80) != 0;
    quint64 len = bytes[1] & 0x7f;
    int header = 2;
    if (len == 126) {
      if (m_buffer.size() < 4) {
        return false;
      }
      len = qFromBigEndian<quint16>(bytes + 2);
      header = 4;
    } else if (len == 127) {
      if (m_buffer.size() < 10) {
        return false;
      }
      len = qFromBigEndian<quint64>(bytes + 2);
      header = 10;
    }
    if (len > static_cast<quint64>(kMaxFrameBytes)) {
      emit m_owner->errorOccurred(QStringLiteral("事件流数据过大"));
      close();
      return false;
    }
    const int maskBytes = masked ? 4 : 0;
    if (static_cast<quint64>(m_buffer.size()) < static_cast<quint64>(header + maskBytes) + len) {
      return false;
    }
    QByteArray payload = m_buffer.mid(header + maskBytes, static_cast<int>(len));
    if (masked) {
      const unsigned char *mask = bytes + header;
      for (int i = 0; i < payload.size(); ++i) {
        payload[i] = static_cast<char>(static_cast<unsigned char>(payload[i]) ^ mask[i & 3]);
      }
    }
    m_buffer.remove(0, header + maskBytes + static_cast<int>(len));

    switch (opcode) {
      case 0x0:
      case 0x1:
        m_textCarry += payload;
        if (fin) {
          dispatchText(m_textCarry);
          m_textCarry.clear();
        }
        break;
      case 0x8:
        close();
        break;
      case 0x9:
        sendMasked(0xA, payload);
        break;
      case 0xA:
        break;
      default:
        break;
    }
    return true;
  }

  void dispatchText(const QByteArray &text) {
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(text, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
      return;
    }
    const QJsonObject envelope = document.object();
    if (envelope.value(QString::fromLatin1(dsh::rpc::kFieldType)).toString() !=
        QLatin1String(dsh::rpc::kServerRequestType)) {
      return;
    }
    const QString rpcId = envelope.value(QString::fromLatin1(dsh::rpc::kFieldRpcId)).toString();
    const QJsonObject payload = envelope.value(QString::fromLatin1(dsh::rpc::kFieldPayload)).toObject();
    if (payload.isEmpty()) {
      return;
    }
    if (m_mux) {
      emit m_owner->muxFrame(rpcId, payload);
    } else {
      emit m_owner->hostFrame(rpcId, payload);
    }
  }

  void sendMasked(int opcode, const QByteArray &payload) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
      return;
    }
    const quint32 maskValue = QRandomGenerator::global()->generate();
    unsigned char mask[4];
    qToBigEndian(maskValue, mask);
    QByteArray frame;
    frame.reserve(payload.size() + 14);
    frame.append(static_cast<char>(0x80 | (opcode & 0x0f)));
    if (payload.size() < 126) {
      frame.append(static_cast<char>(0x80 | payload.size()));
    } else if (payload.size() <= 0xffff) {
      frame.append(static_cast<char>(0x80 | 126));
      quint16 n = static_cast<quint16>(payload.size());
      char len[2];
      qToBigEndian(n, len);
      frame.append(len, 2);
    } else {
      return;
    }
    frame.append(reinterpret_cast<const char *>(mask), 4);
    for (int i = 0; i < payload.size(); ++i) {
      frame.append(static_cast<char>(static_cast<unsigned char>(payload[i]) ^ mask[i & 3]));
    }
    m_socket->write(frame);
  }

  EventStream *m_owner = nullptr;
  bool m_mux = true;
  QTcpSocket *m_socket = nullptr;
  QUrl m_url;
  QString m_httpOrigin;
  QByteArray m_key;
  QByteArray m_header;
  QByteArray m_buffer;
  QByteArray m_textCarry;
  bool m_handshakeDone = false;
};

EventStream::EventStream(QObject *parent)
    : QObject(parent), m_mux(new Downlink(this, true)), m_host(new Downlink(this, false)) {}

EventStream::~EventStream() { disconnectFromHost(); }

void EventStream::connectTo(const QUrl &httpBase) {
  disconnectFromHost();
  m_mux->open(httpBase);
  m_host->open(httpBase);
}

void EventStream::disconnectFromHost() {
  m_mux->close();
  m_host->close();
  emitClosedIfNeeded();
}

bool EventStream::isOpen() const { return m_mux->isOpen() && m_host->isOpen(); }

void EventStream::emitOpenIfNeeded() {
  if (!m_openSignaled && isOpen()) {
    m_openSignaled = true;
    emit openChanged(true);
  }
}

void EventStream::emitClosedIfNeeded() {
  if (m_openSignaled && !isOpen()) {
    m_openSignaled = false;
    emit openChanged(false);
  }
}
