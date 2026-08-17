#include "models/SessionListModel.h"

#include <QVariantMap>

SessionListModel::SessionListModel(QObject *parent) : QAbstractListModel(parent) {}

int SessionListModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return m_rows.size();
}

QVariant SessionListModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) {
    return {};
  }
  const Row &row = m_rows.at(index.row());
  switch (role) {
    case SessionIdRole:
      return row.sessionId;
    case TitleRole:
      return row.title;
    case BlankRole:
      return row.blank;
    case RunningRole:
      return row.running;
    default:
      return {};
  }
}

QHash<int, QByteArray> SessionListModel::roleNames() const {
  return {{SessionIdRole, "sessionId"},
          {TitleRole, "title"},
          {BlankRole, "blank"},
          {RunningRole, "running"}};
}

void SessionListModel::replaceAll(const QVariantList &rows) {
  beginResetModel();
  m_rows.clear();
  m_rows.reserve(rows.size());
  for (const QVariant &value : rows) {
    const QVariantMap map = value.toMap();
    Row row;
    row.sessionId = map.value(QStringLiteral("sessionId")).toString();
    if (row.sessionId.isEmpty()) {
      continue;
    }
    row.title = map.value(QStringLiteral("title")).toString();
    row.blank = map.value(QStringLiteral("blank")).toBool();
    row.running = map.value(QStringLiteral("running")).toBool();
    m_rows.append(row);
  }
  endResetModel();
  emit countChanged();
}

void SessionListModel::setTitle(const QString &sessionId, const QString &title) {
  const int row = indexOf(sessionId);
  if (row < 0 || m_rows.at(row).title == title) {
    return;
  }
  m_rows[row].title = title;
  const QModelIndex idx = index(row);
  emit dataChanged(idx, idx, {TitleRole});
}

void SessionListModel::setRunning(const QString &sessionId, bool running) {
  const int row = indexOf(sessionId);
  if (row < 0 || m_rows.at(row).running == running) {
    return;
  }
  m_rows[row].running = running;
  const QModelIndex idx = index(row);
  emit dataChanged(idx, idx, {RunningRole});
}

QString SessionListModel::titleFor(const QString &sessionId) const {
  const int row = indexOf(sessionId);
  if (row < 0) {
    return sessionId.left(8);
  }
  return m_rows.at(row).title;
}

QString SessionListModel::blankSessionId() const {
  for (const Row &row : m_rows) {
    if (row.blank) {
      return row.sessionId;
    }
  }
  return {};
}

QString SessionListModel::firstSessionId() const {
  return m_rows.isEmpty() ? QString() : m_rows.at(0).sessionId;
}

bool SessionListModel::contains(const QString &sessionId) const { return indexOf(sessionId) >= 0; }

int SessionListModel::indexOf(const QString &sessionId) const {
  for (int i = 0; i < m_rows.size(); ++i) {
    if (m_rows.at(i).sessionId == sessionId) {
      return i;
    }
  }
  return -1;
}
