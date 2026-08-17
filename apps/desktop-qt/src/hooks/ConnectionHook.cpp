#include "hooks/ConnectionHook.h"

ConnectionHook::ConnectionHook(QObject *parent)
    : QObject(parent),
      m_statusText(QStringLiteral("等待连接…")),
      m_connected(false),
      m_connecting(false),
      m_hasError(false) {}

QString ConnectionHook::statusText() const {
  return m_statusText;
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
