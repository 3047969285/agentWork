#include "hooks/ConnectionHook.h"

ConnectionHook::ConnectionHook(QObject *parent)
    : QObject(parent),
      m_statusText(QStringLiteral("等待连接…")),
      m_hostVersion(QStringLiteral("开发版")),
      m_connected(false),
      m_connecting(false),
      m_hasError(false) {}

QString ConnectionHook::statusText() const {
  return m_statusText;
}

QString ConnectionHook::hostVersion() const {
  return m_hostVersion;
}

int ConnectionHook::hostPort() const {
  return m_hostPort;
}

bool ConnectionHook::connected() const {
  return m_connected;
}

bool ConnectionHook::connecting() const {
  return m_connecting;
}

bool ConnectionHook::hasError() const {
  return m_hasError;
}

void ConnectionHook::setStatusText(const QString &text) {
  if (m_statusText != text) {
    m_statusText = text;
    emit statusTextChanged(m_statusText);
  }
}

void ConnectionHook::setHostVersion(const QString &version) {
  if (m_hostVersion != version) {
    m_hostVersion = version;
    emit hostVersionChanged();
  }
}

void ConnectionHook::setHostPort(int port) {
  if (m_hostPort != port) {
    m_hostPort = port;
    emit hostPortChanged();
  }
}

void ConnectionHook::setConnected(bool connected) {
  if (m_connected != connected) {
    m_connected = connected;
    emit connectedChanged(m_connected);
  }
}

void ConnectionHook::setConnecting(bool connecting) {
  if (m_connecting != connecting) {
    m_connecting = connecting;
    emit connectingChanged(m_connecting);
  }
}

void ConnectionHook::setHasError(bool hasError) {
  if (m_hasError != hasError) {
    m_hasError = hasError;
    emit hasErrorChanged(m_hasError);
  }
}

void ConnectionHook::retry() {
  emit retryRequested();
}
