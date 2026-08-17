#include "utils/StudyJson.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QStringList>
#include <QVariantMap>

namespace dsh::study {
namespace {

QString basenamePath(const QString &path) {
  QString trimmed = path;
  while (trimmed.endsWith(QLatin1Char('/')) || trimmed.endsWith(QLatin1Char('\\'))) {
    trimmed.chop(1);
  }
  if (trimmed.isEmpty()) {
    return {};
  }
  return QFileInfo(trimmed).fileName();
}

bool isSubagentRow(const QJsonObject &item) {
  if (item.value(QStringLiteral("origin")).toString() == QLatin1String("subagent")) {
    return true;
  }
  return item.contains(QStringLiteral("parentSessionId")) &&
         !item.value(QStringLiteral("parentSessionId")).toString().isEmpty();
}

QString projectionTitle(const QJsonObject &item) {
  const QJsonObject projections = item.value(QStringLiteral("projections")).toObject();
  const QJsonObject values = projections.value(QStringLiteral("values")).toObject();
  const QJsonValue title = values.value(QStringLiteral("title"));
  if (title.isString()) {
    return title.toString().trimmed();
  }
  return {};
}

}  // namespace

QString rpcErrorMessage(const QJsonValue &resultOrError) {
  if (resultOrError.isString()) {
    const QString text = resultOrError.toString().trimmed();
    return text.isEmpty() ? QStringLiteral("请求失败") : text;
  }
  if (resultOrError.isObject()) {
    const QJsonObject obj = resultOrError.toObject();
    const QString message = obj.value(QStringLiteral("message")).toString().trimmed();
    if (!message.isEmpty()) {
      return message;
    }
    const QString code = obj.value(QStringLiteral("code")).toString().trimmed();
    if (!code.isEmpty()) {
      return code;
    }
  }
  return QStringLiteral("请求失败");
}

QString displayTitle(const QJsonObject &sessionItem) {
  const QString titled = projectionTitle(sessionItem);
  if (!titled.isEmpty()) {
    return titled;
  }
  if (sessionItem.value(QStringLiteral("blank")).toBool()) {
    return QStringLiteral("新会话");
  }
  const QString fromCwd = basenamePath(sessionItem.value(QStringLiteral("cwd")).toString());
  if (!fromCwd.isEmpty()) {
    return fromCwd;
  }
  const QString id = sessionItem.value(QStringLiteral("sessionId")).toString();
  return id.left(8);
}

QString workspaceTitle(const QJsonObject &workspaceItem) {
  const QString title = workspaceItem.value(QStringLiteral("title")).toString().trimmed();
  if (!title.isEmpty()) {
    return title;
  }
  const QString fromPath = basenamePath(workspaceItem.value(QStringLiteral("path")).toString());
  if (!fromPath.isEmpty()) {
    return fromPath;
  }
  return QStringLiteral("未入席");
}

QJsonObject firstWorkspace(const QJsonObject &listValue) {
  const QJsonArray items = listValue.value(QStringLiteral("items")).toArray();
  if (items.isEmpty() || !items.at(0).isObject()) {
    return {};
  }
  return items.at(0).toObject();
}

QSet<QString> archivedSessionIds(const QJsonObject &listValue) {
  QSet<QString> ids;
  const QJsonArray archived = listValue.value(QStringLiteral("archivedSessionIds")).toArray();
  for (const QJsonValue &value : archived) {
    const QString id = value.toString();
    if (!id.isEmpty()) {
      ids.insert(id);
    }
  }
  return ids;
}

QVariantList sessionRows(const QJsonArray &items, const QSet<QString> *allowIds,
                         const QSet<QString> &archived) {
  QVariantList rows;
  for (const QJsonValue &value : items) {
    if (!value.isObject()) {
      continue;
    }
    const QJsonObject item = value.toObject();
    const QString sessionId = item.value(QStringLiteral("sessionId")).toString();
    if (sessionId.isEmpty() || isSubagentRow(item) || archived.contains(sessionId)) {
      continue;
    }
    if (allowIds != nullptr && !allowIds->contains(sessionId)) {
      continue;
    }
    QVariantMap row;
    row.insert(QStringLiteral("sessionId"), sessionId);
    row.insert(QStringLiteral("title"), displayTitle(item));
    row.insert(QStringLiteral("blank"), item.value(QStringLiteral("blank")).toBool());
    row.insert(QStringLiteral("running"), item.value(QStringLiteral("running")).toBool());
    rows.append(row);
  }
  return rows;
}

QString textFromContent(const QJsonValue &content) {
  if (!content.isArray()) {
    return {};
  }
  QStringList parts;
  for (const QJsonValue &part : content.toArray()) {
    const QJsonObject obj = part.toObject();
    if (obj.value(QStringLiteral("type")).toString() != QLatin1String("text")) {
      continue;
    }
    const QString text = obj.value(QStringLiteral("text")).toString();
    if (!text.isEmpty()) {
      parts.append(text);
    }
  }
  return parts.join(QLatin1Char('\n'));
}

QVariantList messageRows(const QJsonArray &historyEvents) {
  QVariantList rows;
  for (const QJsonValue &value : historyEvents) {
    if (!value.isObject()) {
      continue;
    }
    const QJsonObject wrap = value.toObject();
    QJsonObject event = wrap.value(QStringLiteral("event")).toObject();
    if (event.isEmpty() && wrap.contains(QStringLiteral("type"))) {
      event = wrap;
    }
    const QString type = event.value(QStringLiteral("type")).toString();
    const QJsonObject data = event.value(QStringLiteral("data")).toObject();
    const int seq = event.value(QStringLiteral("seq")).toInt();
    QString role;
    if (type == QLatin1String("user/message")) {
      const QString kind = data.value(QStringLiteral("source")).toObject().value(QStringLiteral("kind")).toString();
      if (!kind.isEmpty() && kind != QLatin1String("user")) {
        continue;
      }
      role = QStringLiteral("user");
    } else if (type == QLatin1String("assistant/message")) {
      role = QStringLiteral("assistant");
    } else {
      continue;
    }
    const QString text = textFromContent(data.value(QStringLiteral("content")));
    if (text.isEmpty()) {
      continue;
    }
    QVariantMap row;
    row.insert(QStringLiteral("role"), role);
    row.insert(QStringLiteral("text"), text);
    row.insert(QStringLiteral("seq"), seq);
    rows.append(row);
  }
  return rows;
}

QJsonObject promptPayload(const QString &sessionId, const QString &text) {
  QJsonObject payload;
  payload.insert(QStringLiteral("sessionId"), sessionId);
  payload.insert(QStringLiteral("mode"), QStringLiteral("queue"));
  QJsonObject part;
  part.insert(QStringLiteral("type"), QStringLiteral("text"));
  part.insert(QStringLiteral("text"), text);
  payload.insert(QStringLiteral("content"), QJsonArray{part});
  return payload;
}

QJsonObject createPayload(const QString &workspaceId) {
  QJsonObject payload;
  if (!workspaceId.isEmpty()) {
    payload.insert(QStringLiteral("workspaceId"), workspaceId);
  }
  return payload;
}

QString blankSessionId(const QVariantList &rows) {
  for (const QVariant &row : rows) {
    const QVariantMap map = row.toMap();
    if (map.value(QStringLiteral("blank")).toBool()) {
      return map.value(QStringLiteral("sessionId")).toString();
    }
  }
  return {};
}

}  // namespace dsh::study
