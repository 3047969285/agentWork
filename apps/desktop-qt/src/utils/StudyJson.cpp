#include "utils/StudyJson.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
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

QString viewTitle(const QJsonValue &view) {
  const QJsonObject obj = view.toObject();
  const QJsonObject inner = obj.value(QStringLiteral("view")).toObject();
  const QString title = inner.value(QStringLiteral("title")).toString().trimmed();
  return title;
}

QString viewBody(const QJsonValue &view) {
  const QJsonObject inner = view.toObject().value(QStringLiteral("view")).toObject();
  const QString output = inner.value(QStringLiteral("output")).toString();
  if (!output.isEmpty()) {
    return output;
  }
  const QString fromContent = textFromContent(inner.value(QStringLiteral("content")));
  if (!fromContent.isEmpty()) {
    return fromContent;
  }
  const QJsonValue raw = inner.value(QStringLiteral("rawInput"));
  if (raw.isString()) {
    return raw.toString();
  }
  if (raw.isObject() || raw.isArray()) {
    return QString::fromUtf8(QJsonDocument::fromVariant(raw.toVariant()).toJson(QJsonDocument::Compact));
  }
  return {};
}

QString viewCard(const QJsonValue &view) {
  const QString card = view.toObject().value(QStringLiteral("view")).toObject().value(QStringLiteral("card")).toString();
  return card.isEmpty() ? QStringLiteral("generic") : card;
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

QJsonObject workspaceById(const QJsonObject &listValue, const QString &workspaceId) {
  if (workspaceId.isEmpty()) {
    return firstWorkspace(listValue);
  }
  const QJsonArray items = listValue.value(QStringLiteral("items")).toArray();
  for (const QJsonValue &value : items) {
    if (!value.isObject()) {
      continue;
    }
    const QJsonObject item = value.toObject();
    if (item.value(QStringLiteral("workspaceId")).toString() == workspaceId) {
      return item;
    }
  }
  return firstWorkspace(listValue);
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

QVariantList workspaceRows(const QJsonObject &listValue) {
  QVariantList rows;
  const QJsonArray items = listValue.value(QStringLiteral("items")).toArray();
  for (const QJsonValue &value : items) {
    if (!value.isObject()) {
      continue;
    }
    const QJsonObject item = value.toObject();
    const QString id = item.value(QStringLiteral("workspaceId")).toString();
    if (id.isEmpty()) {
      continue;
    }
    QVariantMap row;
    row.insert(QStringLiteral("workspaceId"), id);
    row.insert(QStringLiteral("title"), workspaceTitle(item));
    row.insert(QStringLiteral("path"), item.value(QStringLiteral("path")).toString());
    rows.append(row);
  }
  return rows;
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

QJsonObject unwrapEvent(const QJsonValue &entry) {
  if (!entry.isObject()) {
    return {};
  }
  const QJsonObject wrap = entry.toObject();
  QJsonObject event = wrap.value(QStringLiteral("event")).toObject();
  if (event.isEmpty() && wrap.contains(QStringLiteral("type"))) {
    event = wrap;
  }
  return event;
}

QString textFromContent(const QJsonValue &content) {
  if (!content.isArray()) {
    return {};
  }
  QStringList parts;
  for (const QJsonValue &part : content.toArray()) {
    const QJsonObject obj = part.toObject();
    const QString type = obj.value(QStringLiteral("type")).toString();
    if (type != QLatin1String("text") && type != QLatin1String("tool-result")) {
      continue;
    }
    const QString text = obj.value(QStringLiteral("text")).toString();
    if (!text.isEmpty()) {
      parts.append(text);
    }
  }
  return parts.join(QLatin1Char('\n'));
}

QString eventText(const QJsonObject &data) {
  const QString direct = textFromContent(data.value(QStringLiteral("content")));
  if (!direct.isEmpty()) {
    return direct;
  }
  const QJsonObject message = data.value(QStringLiteral("message")).toObject();
  return textFromContent(message.value(QStringLiteral("content")));
}

QVariantMap toolRow(const QJsonObject &event, const QJsonValue &view) {
  const QJsonObject data = event.value(QStringLiteral("data")).toObject();
  const QString type = event.value(QStringLiteral("type")).toString();
  QVariantMap row;
  row.insert(QStringLiteral("kind"), QStringLiteral("tool"));
  row.insert(QStringLiteral("role"), QStringLiteral("tool"));
  row.insert(QStringLiteral("seq"), event.value(QStringLiteral("seq")).toInt());
  row.insert(QStringLiteral("callId"), data.value(QStringLiteral("callId")).toString());
  if (row.value(QStringLiteral("callId")).toString().isEmpty()) {
    const QJsonObject message = data.value(QStringLiteral("message")).toObject();
    const QJsonArray content = message.value(QStringLiteral("content")).toArray();
    if (!content.isEmpty()) {
      row.insert(QStringLiteral("callId"), content.at(0).toObject().value(QStringLiteral("callId")).toString());
    }
  }
  const QString name = data.value(QStringLiteral("name")).toString();
  row.insert(QStringLiteral("toolName"), name);
  row.insert(QStringLiteral("card"), viewCard(view));
  QString title = viewTitle(view);
  if (title.isEmpty()) {
    title = name.isEmpty() ? QStringLiteral("工具") : name;
  }
  row.insert(QStringLiteral("title"), title);
  QString body = viewBody(view);
  if (body.isEmpty()) {
    body = eventText(data);
  }
  if (body.isEmpty() && type == QLatin1String("tool/call")) {
    body = data.value(QStringLiteral("arguments")).toString();
  }
  row.insert(QStringLiteral("body"), body);
  row.insert(QStringLiteral("text"), title);
  if (type == QLatin1String("tool/result")) {
    const bool failed = data.contains(QStringLiteral("error"));
    row.insert(QStringLiteral("status"), failed ? QStringLiteral("error") : QStringLiteral("done"));
  } else {
    row.insert(QStringLiteral("status"), QStringLiteral("pending"));
  }
  row.insert(QStringLiteral("streaming"), false);
  return row;
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

QJsonObject modelsPayload(const QString &sessionId) {
  QJsonObject payload;
  payload.insert(QStringLiteral("sessionId"), sessionId);
  return payload;
}

QJsonObject selectModelPayload(const QString &sessionId, const QString &provider, const QString &model) {
  QJsonObject payload;
  payload.insert(QStringLiteral("sessionId"), sessionId);
  payload.insert(QStringLiteral("provider"), provider);
  payload.insert(QStringLiteral("model"), model);
  return payload;
}

QJsonObject cancelPayload(const QString &sessionId) {
  QJsonObject payload;
  payload.insert(QStringLiteral("sessionId"), sessionId);
  return payload;
}

QVariantList modelOptions(const QJsonObject &modelsValue) {
  QVariantList rows;
  const QJsonArray groups = modelsValue.value(QStringLiteral("groups")).toArray();
  for (const QJsonValue &groupValue : groups) {
    const QJsonObject group = groupValue.toObject();
    const QString provider = group.value(QStringLiteral("id")).toString();
    const QString providerName = group.value(QStringLiteral("name")).toString();
    const QJsonArray models = group.value(QStringLiteral("models")).toArray();
    for (const QJsonValue &modelValue : models) {
      const QJsonObject model = modelValue.toObject();
      const QString id = model.value(QStringLiteral("id")).toString();
      if (provider.isEmpty() || id.isEmpty()) {
        continue;
      }
      QVariantMap row;
      row.insert(QStringLiteral("provider"), provider);
      row.insert(QStringLiteral("model"), id);
      const QString name = model.value(QStringLiteral("name")).toString();
      row.insert(QStringLiteral("name"), name.isEmpty() ? id : name);
      row.insert(QStringLiteral("group"), providerName.isEmpty() ? provider : providerName);
      rows.append(row);
    }
  }
  return rows;
}

QString modelLabel(const QJsonObject &modelsValue) {
  const QJsonObject current = modelsValue.value(QStringLiteral("current")).toObject();
  const QString model = current.value(QStringLiteral("model")).toString();
  if (!model.isEmpty()) {
    return model;
  }
  return QStringLiteral("模型");
}

QVariantList settingsNamespaces(const QJsonObject &describeValue) {
  QVariantList rows;
  const QJsonArray namespaces = describeValue.value(QStringLiteral("namespaces")).toArray();
  const bool writable = describeValue.value(QStringLiteral("writable")).toBool();
  for (const QJsonValue &value : namespaces) {
    const QJsonObject item = value.toObject();
    const QString ns = item.value(QStringLiteral("ns")).toString();
    if (ns.isEmpty()) {
      continue;
    }
    QVariantMap row;
    row.insert(QStringLiteral("ns"), ns);
    row.insert(QStringLiteral("applies"), item.value(QStringLiteral("applies")).toString());
    row.insert(QStringLiteral("writable"), writable);
    rows.append(row);
  }
  return rows;
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

QString projectionTitleValue(const QString &key, const QJsonValue &value) {
  if (key != QLatin1String("title")) {
    return {};
  }
  if (value.isString()) {
    return value.toString().trimmed();
  }
  if (value.isObject()) {
    return value.toObject().value(QStringLiteral("title")).toString().trimmed();
  }
  return {};
}

}  // namespace dsh::study
