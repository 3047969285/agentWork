#include "models/TranscriptModel.h"

#include "utils/StudyJson.h"

#include <QJsonArray>
#include <QVariantMap>

TranscriptModel::TranscriptModel(QObject *parent) : QAbstractListModel(parent) {}

int TranscriptModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    return 0;
  }
  return m_rows.size();
}

QVariant TranscriptModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) {
    return {};
  }
  const Row &row = m_rows.at(index.row());
  switch (role) {
    case KindRole:
      return row.kind;
    case RoleNameRole:
      return row.role;
    case TextRole:
      return row.text;
    case SeqRole:
      return row.seq;
    case CallIdRole:
      return row.callId;
    case ToolNameRole:
      return row.toolName;
    case CardRole:
      return row.card;
    case TitleRole:
      return row.title;
    case BodyRole:
      return row.body;
    case StatusRole:
      return row.status;
    case StreamingRole:
      return row.streaming;
    default:
      return {};
  }
}

QHash<int, QByteArray> TranscriptModel::roleNames() const {
  return {{KindRole, "kind"},     {RoleNameRole, "role"}, {TextRole, "text"},
          {SeqRole, "seq"},       {CallIdRole, "callId"}, {ToolNameRole, "toolName"},
          {CardRole, "card"},     {TitleRole, "title"},   {BodyRole, "body"},
          {StatusRole, "status"}, {StreamingRole, "streaming"}};
}

void TranscriptModel::resetFromHistory(const QJsonArray &events) {
  beginResetModel();
  m_rows.clear();
  m_toolIndex.clear();
  for (const QJsonValue &entry : events) {
    const Row row = fromHistoryEvent(entry);
    if (row.kind.isEmpty()) {
      continue;
    }
    if (row.kind == QLatin1String("tool") && !row.callId.isEmpty()) {
      const auto it = m_toolIndex.constFind(row.callId);
      if (it != m_toolIndex.cend()) {
        m_rows[it.value()] = row;
        continue;
      }
      m_toolIndex.insert(row.callId, m_rows.size());
    }
    m_rows.append(row);
  }
  endResetModel();
  emit countChanged();
}

void TranscriptModel::applySessionEvent(const QJsonObject &event, const QJsonValue &view) {
  const Row row = fromSessionEvent(event, view);
  if (row.kind.isEmpty()) {
    return;
  }
  // Seq-based dedup: drop events already present from history hydration.
  if (row.seq > 0 && row.seq <= lastSeq()) {
    return;
  }
  if (row.kind == QLatin1String("tool") && !row.callId.isEmpty()) {
    const int existing = findToolRow(row.callId);
    if (existing >= 0) {
      replaceRow(existing, row, {KindRole, RoleNameRole, TextRole, SeqRole, CallIdRole, ToolNameRole,
                                 CardRole, TitleRole, BodyRole, StatusRole, StreamingRole});
      return;
    }
  }
  if (row.kind == QLatin1String("assistant") && !row.streaming) {
    const int streaming = findStreamingRow();
    if (streaming >= 0) {
      replaceRow(streaming, row, {KindRole, RoleNameRole, TextRole, SeqRole, StreamingRole});
      return;
    }
  }
  appendRow(row);
}

void TranscriptModel::appendUser(const QString &text) {
  Row row;
  row.kind = QStringLiteral("user");
  row.role = QStringLiteral("user");
  row.text = text;
  row.seq = -1;
  appendRow(row);
}

void TranscriptModel::appendStreamDelta(const QString &delta) {
  if (delta.isEmpty()) {
    return;
  }
  int streaming = findStreamingRow();
  if (streaming < 0) {
    Row row;
    row.kind = QStringLiteral("assistant");
    row.role = QStringLiteral("assistant");
    row.text = delta;
    row.streaming = true;
    appendRow(row);
    return;
  }
  updateText(streaming, m_rows.at(streaming).text + delta, m_rows.at(streaming).seq, true);
}

void TranscriptModel::finishStreaming() {
  const int streaming = findStreamingRow();
  if (streaming < 0) {
    return;
  }
  m_rows[streaming].streaming = false;
  const QModelIndex idx = index(streaming);
  emit dataChanged(idx, idx, {StreamingRole});
}

void TranscriptModel::clear() {
  if (m_rows.isEmpty()) {
    return;
  }
  beginResetModel();
  m_rows.clear();
  m_toolIndex.clear();
  endResetModel();
  emit countChanged();
}

int TranscriptModel::lastSeq() const {
  int maxSeq = 0;
  for (const Row &row : m_rows) {
    if (row.seq > maxSeq) {
      maxSeq = row.seq;
    }
  }
  return maxSeq;
}

bool TranscriptModel::hasStreaming() const { return findStreamingRow() >= 0; }

void TranscriptModel::appendRow(const Row &row) {
  const int at = m_rows.size();
  beginInsertRows(QModelIndex(), at, at);
  m_rows.append(row);
  if (row.kind == QLatin1String("tool") && !row.callId.isEmpty()) {
    m_toolIndex.insert(row.callId, at);
  }
  endInsertRows();
  emit countChanged();
}

void TranscriptModel::replaceRow(int index, const Row &row, const QVector<int> &roles) {
  m_rows[index] = row;
  if (row.kind == QLatin1String("tool") && !row.callId.isEmpty()) {
    m_toolIndex.insert(row.callId, index);
  }
  const QModelIndex idx = this->index(index);
  emit dataChanged(idx, idx, roles);
}

void TranscriptModel::updateText(int index, const QString &text, int seq, bool streaming) {
  m_rows[index].text = text;
  m_rows[index].seq = seq;
  m_rows[index].streaming = streaming;
  const QModelIndex idx = this->index(index);
  emit dataChanged(idx, idx, {TextRole, SeqRole, StreamingRole});
}

int TranscriptModel::findToolRow(const QString &callId) const {
  return m_toolIndex.value(callId, -1);
}

int TranscriptModel::findStreamingRow() const {
  for (int i = m_rows.size() - 1; i >= 0; --i) {
    if (m_rows.at(i).streaming && m_rows.at(i).kind == QLatin1String("assistant")) {
      return i;
    }
  }
  return -1;
}

TranscriptModel::Row TranscriptModel::fromHistoryEvent(const QJsonValue &entry) {
  const QJsonObject event = dsh::study::unwrapEvent(entry);
  const QJsonValue view = entry.toObject().value(QStringLiteral("view"));
  return fromSessionEvent(event, view);
}

TranscriptModel::Row TranscriptModel::fromSessionEvent(const QJsonObject &event, const QJsonValue &view) {
  const QString type = event.value(QStringLiteral("type")).toString();
  const QJsonObject data = event.value(QStringLiteral("data")).toObject();
  Row row;
  row.seq = event.value(QStringLiteral("seq")).toInt();
  if (type == QLatin1String("user/message")) {
    const QString kind = data.value(QStringLiteral("source")).toObject().value(QStringLiteral("kind")).toString();
    if (!kind.isEmpty() && kind != QLatin1String("user")) {
      return {};
    }
    row.kind = QStringLiteral("user");
    row.role = QStringLiteral("user");
    row.text = dsh::study::eventText(data);
    if (row.text.isEmpty()) {
      return {};
    }
    return row;
  }
  if (type == QLatin1String("assistant/message")) {
    row.kind = QStringLiteral("assistant");
    row.role = QStringLiteral("assistant");
    row.text = dsh::study::eventText(data);
    row.streaming = false;
    if (row.text.isEmpty()) {
      return {};
    }
    return row;
  }
  if (type == QLatin1String("tool/call") || type == QLatin1String("tool/result")) {
    const QVariantMap map = dsh::study::toolRow(event, view);
    row.kind = map.value(QStringLiteral("kind")).toString();
    row.role = map.value(QStringLiteral("role")).toString();
    row.text = map.value(QStringLiteral("text")).toString();
    row.callId = map.value(QStringLiteral("callId")).toString();
    row.toolName = map.value(QStringLiteral("toolName")).toString();
    row.card = map.value(QStringLiteral("card")).toString();
    row.title = map.value(QStringLiteral("title")).toString();
    row.body = map.value(QStringLiteral("body")).toString();
    row.status = map.value(QStringLiteral("status")).toString();
    row.streaming = false;
    return row;
  }
  return {};
}
