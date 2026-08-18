#include "utils/StudyJson.h"

#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSet>
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

QVariantList visibleSessionRows(const QJsonObject &workspaceList, const QJsonObject &sessionList,
                                const QString &workspaceId) {
  const QSet<QString> archived = archivedSessionIds(workspaceList);
  const QJsonArray items = sessionList.value(QStringLiteral("items")).toArray();
  const QJsonObject workspace = workspaceById(workspaceList, workspaceId);
  QSet<QString> allowIds;
  const QSet<QString> *allowPtr = nullptr;
  if (!workspace.isEmpty()) {
    const QJsonArray sessionIds = workspace.value(QStringLiteral("sessionIds")).toArray();
    for (const QJsonValue &value : sessionIds) {
      const QString id = value.toString();
      if (!id.isEmpty()) {
        allowIds.insert(id);
      }
    }
    if (!allowIds.isEmpty()) {
      allowPtr = &allowIds;
    }
  }
  QVariantList rows = sessionRows(items, allowPtr, archived);
  if (rows.isEmpty() && allowPtr != nullptr) {
    rows = sessionRows(items, nullptr, archived);
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
  return promptPayload(sessionId, text, {});
}

QJsonObject promptPayload(const QString &sessionId, const QString &text, const QVariantList &images) {
  QJsonObject payload;
  payload.insert(QStringLiteral("sessionId"), sessionId);
  payload.insert(QStringLiteral("mode"), QStringLiteral("queue"));
  QJsonArray content;
  const QString trimmed = text.trimmed();
  if (!trimmed.isEmpty()) {
    QJsonObject part;
    part.insert(QStringLiteral("type"), QStringLiteral("text"));
    part.insert(QStringLiteral("text"), trimmed);
    content.append(part);
  }
  for (const QVariant &imageValue : images) {
    const QVariantMap image = imageValue.toMap();
    const QString mediaType = image.value(QStringLiteral("mediaType")).toString();
    const QString data = image.value(QStringLiteral("data")).toString();
    if (mediaType.isEmpty() || data.isEmpty()) {
      continue;
    }
    QJsonObject part;
    part.insert(QStringLiteral("type"), QStringLiteral("image"));
    part.insert(QStringLiteral("mediaType"), mediaType);
    part.insert(QStringLiteral("data"), data);
    const QString name = image.value(QStringLiteral("name")).toString();
    if (!name.isEmpty()) {
      part.insert(QStringLiteral("name"), name);
    }
    content.append(part);
  }
  payload.insert(QStringLiteral("content"), content);
  return payload;
}

QJsonObject createPayload(const QString &workspaceId) {
  QJsonObject payload;
  if (!workspaceId.isEmpty()) {
    payload.insert(QStringLiteral("workspaceId"), workspaceId);
  }
  return payload;
}

QJsonObject workspaceCreatePayload(const QString &path) {
  QJsonObject payload;
  payload.insert(QStringLiteral("path"), path);
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
    row.insert(QStringLiteral("title"), namespaceTitle(ns));
    row.insert(QStringLiteral("applies"), item.value(QStringLiteral("applies")).toString());
    row.insert(QStringLiteral("writable"), writable);
    rows.append(row);
  }
  return rows;
}

QJsonObject settingsUpdatePayload(const QString &ns, const QString &key, const QJsonValue &value, int revision) {
  QJsonObject payload;
  payload.insert(QStringLiteral("ns"), ns);
  QJsonObject patch;
  patch.insert(key, value);
  payload.insert(QStringLiteral("patch"), patch);
  if (revision >= 0) {
    payload.insert(QStringLiteral("expectedRevision"), revision);
  }
  return payload;
}

QJsonObject credentialsDescribePayload(const QStringList &refs) {
  QJsonObject payload;
  QJsonArray array;
  for (const QString &ref : refs) {
    if (!ref.isEmpty()) {
      array.append(ref);
    }
  }
  payload.insert(QStringLiteral("refs"), array);
  return payload;
}

QJsonObject credentialsSetPayload(const QString &ref, const QString &value) {
  QJsonObject payload;
  payload.insert(QStringLiteral("ref"), ref);
  payload.insert(QStringLiteral("value"), value);
  return payload;
}

QJsonObject skillListPayload(const QString &sessionId) {
  QJsonObject payload;
  payload.insert(QStringLiteral("sessionId"), sessionId);
  return payload;
}

QJsonObject subagentListPayload(const QString &parentSessionId) {
  QJsonObject payload;
  payload.insert(QStringLiteral("parentSessionId"), parentSessionId);
  return payload;
}

QJsonObject subagentInterruptPayload(const QString &parentSessionId, const QString &childSessionId) {
  QJsonObject payload;
  payload.insert(QStringLiteral("parentSessionId"), parentSessionId);
  payload.insert(QStringLiteral("childSessionId"), childSessionId);
  payload.insert(QStringLiteral("mode"), QStringLiteral("continuable"));
  return payload;
}

QJsonObject agentPresetSelectPayload(const QString &sessionId, const QString &agentPreset) {
  QJsonObject payload;
  payload.insert(QStringLiteral("sessionId"), sessionId);
  payload.insert(QStringLiteral("agentPreset"), agentPreset);
  return payload;
}

QString namespaceTitle(const QString &ns) {
  if (ns == QLatin1String("llm-deepseek")) {
    return QStringLiteral("深度求索");
  }
  if (ns == QLatin1String("llm-pi-ai")) {
    return QStringLiteral("兼容接口");
  }
  if (ns == QLatin1String("permission")) {
    return QStringLiteral("权限");
  }
  if (ns == QLatin1String("ui-theme")) {
    return QStringLiteral("外观");
  }
  if (ns == QLatin1String("locale")) {
    return QStringLiteral("语言");
  }
  if (ns == QLatin1String("ui-conversation")) {
    return QStringLiteral("对话");
  }
  if (ns == QLatin1String("shell")) {
    return QStringLiteral("终端");
  }
  if (ns == QLatin1String("agent-loop")) {
    return QStringLiteral("回路");
  }
  if (ns == QLatin1String("web-search-deepseek")) {
    return QStringLiteral("检索");
  }
  if (ns == QLatin1String("ui-onboarding")) {
    return QStringLiteral("入门");
  }
  if (ns == QLatin1String("agent-presets")) {
    return QStringLiteral("智能体预设");
  }
  return ns;
}

QString fieldTitle(const QString &key) {
  if (key == QLatin1String("baseURL")) {
    return QStringLiteral("接口地址");
  }
  if (key == QLatin1String("apiKey")) {
    return QStringLiteral("密钥");
  }
  if (key == QLatin1String("apiKeyEnv")) {
    return QStringLiteral("密钥环境变量");
  }
  if (key == QLatin1String("preference")) {
    return QStringLiteral("偏好");
  }
  if (key == QLatin1String("timeoutMs")) {
    return QStringLiteral("超时毫秒");
  }
  if (key == QLatin1String("maxParallelToolCalls")) {
    return QStringLiteral("并行工具上限");
  }
  if (key == QLatin1String("busyEnter")) {
    return QStringLiteral("忙碌时回车");
  }
  if (key == QLatin1String("defaultPreset")) {
    return QStringLiteral("默认权限");
  }
  if (key == QLatin1String("welcomeNoticeVersion")) {
    return QStringLiteral("入门版本");
  }
  return key;
}

QString appliesLabel(const QString &applies) {
  if (applies == QLatin1String("restart")) {
    return QStringLiteral("须重启");
  }
  return QStringLiteral("即时生效");
}

QString jobStatusLabel(const QString &status) {
  if (status == QLatin1String("running")) {
    return QStringLiteral("进行中");
  }
  if (status == QLatin1String("stopping")) {
    return QStringLiteral("停止中");
  }
  if (status == QLatin1String("completed")) {
    return QStringLiteral("完成");
  }
  if (status == QLatin1String("killed")) {
    return QStringLiteral("已结束");
  }
  if (status == QLatin1String("failed")) {
    return QStringLiteral("失败");
  }
  return status;
}

QString permissionLabel(const QString &id) {
  if (id == QLatin1String("danger-full-access")) {
    return QStringLiteral("完全访问");
  }
  if (id == QLatin1String("read-only")) {
    return QStringLiteral("只读");
  }
  if (id == QLatin1String("workspace-write")) {
    return QStringLiteral("工作区可写");
  }
  if (id == QLatin1String("custom")) {
    return QStringLiteral("自定义");
  }
  return id;
}

QString enumValueLabel(const QString &value) {
  if (value == QLatin1String("light")) {
    return QStringLiteral("浅色");
  }
  if (value == QLatin1String("dark")) {
    return QStringLiteral("深色");
  }
  if (value == QLatin1String("system")) {
    return QStringLiteral("跟随系统");
  }
  if (value == QLatin1String("steer")) {
    return QStringLiteral("插入");
  }
  if (value == QLatin1String("queue")) {
    return QStringLiteral("排队");
  }
  if (value == QLatin1String("zh") || value == QLatin1String("zh-CN")) {
    return QStringLiteral("简体中文");
  }
  if (value == QLatin1String("en")) {
    return QStringLiteral("英文");
  }
  return permissionLabel(value);
}

QString sectionForNs(const QString &ns) {
  if (ns.startsWith(QLatin1String("llm-"))) {
    return QStringLiteral("models");
  }
  if (ns == QLatin1String("permission")) {
    return QStringLiteral("permission");
  }
  return QStringLiteral("general");
}

QVariantList enumChoices(const QJsonObject &node) {
  QVariantList choices;
  if (node.value(QStringLiteral("type")).toString() != QLatin1String("union")) {
    return choices;
  }
  const QJsonArray list = node.value(QStringLiteral("list")).toArray();
  for (const QJsonValue &entry : list) {
    const QJsonObject item = entry.toObject();
    if (item.value(QStringLiteral("type")).toString() != QLatin1String("const")) {
      continue;
    }
    const QJsonValue raw = item.value(QStringLiteral("value"));
    if (!raw.isString()) {
      continue;
    }
    QVariantMap choice;
    const QString id = raw.toString();
    choice.insert(QStringLiteral("id"), id);
    const QString described = item.value(QStringLiteral("meta")).toObject().value(QStringLiteral("description")).toString();
    choice.insert(QStringLiteral("label"), described.isEmpty() ? enumValueLabel(id) : enumValueLabel(id));
    choices.append(choice);
  }
  return choices;
}

bool secretSetAt(const QJsonArray &secrets, const QString &key) {
  for (const QJsonValue &value : secrets) {
    const QJsonObject item = value.toObject();
    const QJsonArray path = item.value(QStringLiteral("path")).toArray();
    if (path.size() == 1 && path.at(0).toString() == key) {
      return item.value(QStringLiteral("set")).toBool();
    }
  }
  return false;
}

bool isSecretKey(const QJsonArray &secrets, const QString &key) {
  for (const QJsonValue &value : secrets) {
    const QJsonArray path = value.toObject().value(QStringLiteral("path")).toArray();
    if (path.size() == 1 && path.at(0).toString() == key) {
      return true;
    }
  }
  return false;
}

void appendField(QVariantList *rows, const QString &ns, const QString &key, const QString &kind,
                 const QVariant &value, const QVariantList &choices, bool secretSet, int revision,
                 const QString &applies, bool writable) {
  QVariantMap row;
  row.insert(QStringLiteral("ns"), ns);
  row.insert(QStringLiteral("nsTitle"), namespaceTitle(ns));
  row.insert(QStringLiteral("key"), key);
  row.insert(QStringLiteral("title"), fieldTitle(key));
  row.insert(QStringLiteral("kind"), kind);
  row.insert(QStringLiteral("value"), value);
  row.insert(QStringLiteral("choices"), choices);
  row.insert(QStringLiteral("secretSet"), secretSet);
  row.insert(QStringLiteral("revision"), revision);
  row.insert(QStringLiteral("applies"), applies);
  row.insert(QStringLiteral("writable"), writable);
  row.insert(QStringLiteral("section"), sectionForNs(ns));
  rows->append(row);
}

QVariantList settingsFields(const QJsonObject &describeValue) {
  QVariantList rows;
  const bool writable = describeValue.value(QStringLiteral("writable")).toBool();
  const QJsonArray namespaces = describeValue.value(QStringLiteral("namespaces")).toArray();
  for (const QJsonValue &nsValue : namespaces) {
    const QJsonObject item = nsValue.toObject();
    const QString ns = item.value(QStringLiteral("ns")).toString();
    if (ns.isEmpty()) {
      continue;
    }
    const int revision = item.value(QStringLiteral("revision")).toInt();
    const QString applies = item.value(QStringLiteral("applies")).toString();
    const QJsonObject value = item.value(QStringLiteral("value")).toObject();
    const QJsonArray secrets = item.value(QStringLiteral("secrets")).toArray();
    const QJsonObject dict = item.value(QStringLiteral("schema")).toObject().value(QStringLiteral("dict")).toObject();
    QSet<QString> seen;
    const QStringList keys = dict.isEmpty() ? value.keys() : dict.keys();
    for (const QString &key : keys) {
      if (key.isEmpty() || seen.contains(key)) {
        continue;
      }
      seen.insert(key);
      if (isSecretKey(secrets, key)) {
        appendField(&rows, ns, key, QStringLiteral("secret"), QString(), {}, secretSetAt(secrets, key),
                    revision, applies, writable);
        continue;
      }
      const QJsonObject node = dict.value(key).toObject();
      const QString type = node.value(QStringLiteral("type")).toString();
      if (type == QLatin1String("boolean") || value.value(key).isBool()) {
        appendField(&rows, ns, key, QStringLiteral("bool"), value.value(key).toBool(), {}, false, revision,
                    applies, writable);
        continue;
      }
      if (type == QLatin1String("number") || type == QLatin1String("integer") || value.value(key).isDouble()) {
        appendField(&rows, ns, key, QStringLiteral("number"), value.value(key).toDouble(), {}, false, revision,
                    applies, writable);
        continue;
      }
      const QVariantList choices = enumChoices(node);
      if (!choices.isEmpty() || type == QLatin1String("union")) {
        if (choices.isEmpty()) {
          continue;
        }
        appendField(&rows, ns, key, QStringLiteral("enum"), value.value(key).toString(), choices, false,
                    revision, applies, writable);
        continue;
      }
      if (type == QLatin1String("string") || value.value(key).isString() || type.isEmpty()) {
        if (value.value(key).isObject() || value.value(key).isArray()) {
          continue;
        }
        appendField(&rows, ns, key, QStringLiteral("string"), value.value(key).toString(), {}, false, revision,
                    applies, writable);
      }
    }
    for (const QJsonValue &secretValue : secrets) {
      const QJsonArray path = secretValue.toObject().value(QStringLiteral("path")).toArray();
      if (path.size() != 1) {
        continue;
      }
      const QString key = path.at(0).toString();
      if (key.isEmpty() || seen.contains(key)) {
        continue;
      }
      seen.insert(key);
      appendField(&rows, ns, key, QStringLiteral("secret"), QString(), {},
                  secretValue.toObject().value(QStringLiteral("set")).toBool(), revision, applies, writable);
    }
  }
  return rows;
}

QString apiKeyEnvOf(const QJsonObject &value, const QJsonArray &path) {
  QJsonValue current = value;
  for (const QJsonValue &segment : path) {
    if (!current.isObject()) {
      return {};
    }
    current = current.toObject().value(segment.toString());
  }
  if (current.isObject()) {
    return current.toObject().value(QStringLiteral("apiKeyEnv")).toString();
  }
  return value.value(QStringLiteral("apiKeyEnv")).toString();
}

QVariantList providerRows(const QJsonObject &providersValue, const QJsonObject &describeValue,
                          const QJsonObject &credentialsValue) {
  QVariantList rows;
  QHash<QString, QJsonObject> namespaces;
  const QJsonArray nsItems = describeValue.value(QStringLiteral("namespaces")).toArray();
  for (const QJsonValue &value : nsItems) {
    const QJsonObject item = value.toObject();
    namespaces.insert(item.value(QStringLiteral("ns")).toString(), item);
  }
  const QJsonObject credentials = credentialsValue.value(QStringLiteral("credentials")).toObject();
  const QJsonArray providers = providersValue.value(QStringLiteral("providers")).toArray();
  for (const QJsonValue &value : providers) {
    const QJsonObject entry = value.toObject();
    const QString provider = entry.value(QStringLiteral("provider")).toString();
    if (provider.isEmpty()) {
      continue;
    }
    const QString ns = entry.value(QStringLiteral("settingsNs")).toString();
    const QJsonObject namespaceView = namespaces.value(ns);
    const QJsonObject nsValue = namespaceView.value(QStringLiteral("value")).toObject();
    const QJsonArray settingsPath = entry.value(QStringLiteral("settingsPath")).toArray();
    QString ref = apiKeyEnvOf(nsValue, settingsPath);
    if (ref.isEmpty() && provider == QLatin1String("deepseek-official")) {
      ref = QStringLiteral("DEEPSEEK_API_KEY");
    }
    const QJsonObject credential = credentials.value(ref).toObject();
    const bool configured = credential.value(QStringLiteral("configured")).toBool();
    const bool active = entry.value(QStringLiteral("active")).toBool();
    QVariantMap row;
    row.insert(QStringLiteral("provider"), provider);
    const QString display = entry.value(QStringLiteral("displayName")).toString();
    row.insert(QStringLiteral("title"), provider == QLatin1String("deepseek-official")
                                            ? QStringLiteral("深度求索")
                                            : (display.isEmpty() ? provider : display));
    row.insert(QStringLiteral("settingsNs"), ns);
    row.insert(QStringLiteral("active"), active);
    row.insert(QStringLiteral("apiKeyEnv"), ref);
    row.insert(QStringLiteral("configured"), configured);
    row.insert(QStringLiteral("credentialWritable"), credential.value(QStringLiteral("writable")).toBool());
    row.insert(QStringLiteral("usable"), active && (ref.isEmpty() || configured));
    row.insert(QStringLiteral("official"), provider == QLatin1String("deepseek-official") &&
                                               ns == QLatin1String("llm-deepseek") && settingsPath.isEmpty());
    rows.append(row);
  }
  return rows;
}

QVariantList skillRows(const QJsonObject &listValue) {
  QVariantList rows;
  const QJsonArray skills = listValue.value(QStringLiteral("skills")).toArray();
  for (const QJsonValue &value : skills) {
    const QJsonObject item = value.toObject();
    const QString name = item.value(QStringLiteral("name")).toString();
    if (name.isEmpty()) {
      continue;
    }
    QVariantMap row;
    row.insert(QStringLiteral("name"), name);
    row.insert(QStringLiteral("description"), item.value(QStringLiteral("description")).toString());
    row.insert(QStringLiteral("whenToUse"), item.value(QStringLiteral("whenToUse")).toString());
    row.insert(QStringLiteral("modelInvocable"), item.value(QStringLiteral("modelInvocable")).toBool());
    rows.append(row);
  }
  return rows;
}

QVariantList subagentRows(const QJsonObject &listValue) {
  QVariantList rows;
  const QJsonArray entries = listValue.value(QStringLiteral("entries")).toArray();
  for (const QJsonValue &value : entries) {
    const QJsonObject item = value.toObject();
    const QString id = item.value(QStringLiteral("id")).toString();
    if (id.isEmpty()) {
      continue;
    }
    QVariantMap row;
    row.insert(QStringLiteral("id"), id);
    row.insert(QStringLiteral("kind"), item.value(QStringLiteral("kind")).toString());
    const QString label = item.value(QStringLiteral("label")).toString();
    row.insert(QStringLiteral("label"), label.isEmpty() ? id.left(8) : label);
    row.insert(QStringLiteral("mode"), item.value(QStringLiteral("mode")).toString());
    row.insert(QStringLiteral("activity"), item.value(QStringLiteral("activity")).toString());
    row.insert(QStringLiteral("reason"), item.value(QStringLiteral("reason")).toString());
    row.insert(QStringLiteral("running"), item.value(QStringLiteral("activity")).toString() == QLatin1String("running"));
    row.insert(QStringLiteral("continuable"), item.value(QStringLiteral("mode")).toString() == QLatin1String("continuable"));
    rows.append(row);
  }
  return rows;
}

QVariantList presetRows(const QJsonObject &listValue) {
  QVariantList rows;
  const QJsonArray presets = listValue.value(QStringLiteral("presets")).toArray();
  for (const QJsonValue &value : presets) {
    const QJsonObject item = value.toObject();
    const QString id = item.value(QStringLiteral("id")).toString();
    if (id.isEmpty()) {
      continue;
    }
    QVariantMap row;
    row.insert(QStringLiteral("id"), id);
    const QString name = item.value(QStringLiteral("name")).toString();
    row.insert(QStringLiteral("name"), name.isEmpty() ? id : name);
    row.insert(QStringLiteral("description"), item.value(QStringLiteral("description")).toString());
    row.insert(QStringLiteral("isDefault"), item.value(QStringLiteral("isDefault")).toBool());
    row.insert(QStringLiteral("broken"), item.value(QStringLiteral("broken")).toString());
    row.insert(QStringLiteral("trust"), item.value(QStringLiteral("trust")).toString() == QLatin1String("system")
                                            ? QStringLiteral("系统")
                                            : QStringLiteral("自备"));
    rows.append(row);
  }
  return rows;
}

QVariantList jobRows(const QJsonArray &jobs) {
  QVariantList rows;
  for (const QJsonValue &value : jobs) {
    const QJsonObject item = value.toObject();
    const QString id = item.value(QStringLiteral("id")).toString();
    if (id.isEmpty()) {
      continue;
    }
    QVariantMap row;
    row.insert(QStringLiteral("id"), id);
    row.insert(QStringLiteral("kind"), item.value(QStringLiteral("kind")).toString());
    row.insert(QStringLiteral("label"), item.value(QStringLiteral("label")).toString());
    const QString status = item.value(QStringLiteral("status")).toString();
    row.insert(QStringLiteral("status"), status);
    row.insert(QStringLiteral("statusLabel"), jobStatusLabel(status));
    row.insert(QStringLiteral("detail"), item.value(QStringLiteral("detail")).toString());
    row.insert(QStringLiteral("live"), status == QLatin1String("running") || status == QLatin1String("stopping"));
    rows.append(row);
  }
  return rows;
}

QVariantList permissionOptions(const QJsonValue &projectionValue, const QJsonObject &describeValue) {
  QVariantList rows;
  QSet<QString> seen;
  const QJsonObject projection = projectionValue.toObject();
  const QJsonArray options = projection.value(QStringLiteral("options")).toArray();
  for (const QJsonValue &value : options) {
    const QJsonObject item = value.toObject();
    const QString id = item.value(QStringLiteral("value")).toString();
    if (id.isEmpty() || seen.contains(id)) {
      continue;
    }
    seen.insert(id);
    QVariantMap row;
    row.insert(QStringLiteral("id"), id);
    const QString name = item.value(QStringLiteral("name")).toString();
    row.insert(QStringLiteral("label"), permissionLabel(id == QLatin1String("danger-full-access") ? id : (name.isEmpty() ? id : id)));
    if (id == QLatin1String("danger-full-access")) {
      row.insert(QStringLiteral("label"), QStringLiteral("完全访问"));
    } else if (!name.isEmpty() && name != id) {
      row.insert(QStringLiteral("label"), permissionLabel(id) == id ? name : permissionLabel(id));
    }
    row.insert(QStringLiteral("current"), id == projection.value(QStringLiteral("currentValue")).toString());
    rows.append(row);
  }
  if (!rows.isEmpty()) {
    return rows;
  }
  const QJsonArray namespaces = describeValue.value(QStringLiteral("namespaces")).toArray();
  for (const QJsonValue &value : namespaces) {
    const QJsonObject item = value.toObject();
    if (item.value(QStringLiteral("ns")).toString() != QLatin1String("permission")) {
      continue;
    }
    const QJsonObject node =
        item.value(QStringLiteral("schema")).toObject().value(QStringLiteral("dict")).toObject().value(QStringLiteral("defaultPreset")).toObject();
    const QString current = item.value(QStringLiteral("value")).toObject().value(QStringLiteral("defaultPreset")).toString();
    for (const QVariant &choiceValue : enumChoices(node)) {
      const QVariantMap choice = choiceValue.toMap();
      const QString id = choice.value(QStringLiteral("id")).toString();
      if (id.isEmpty() || seen.contains(id)) {
        continue;
      }
      seen.insert(id);
      QVariantMap row;
      row.insert(QStringLiteral("id"), id);
      row.insert(QStringLiteral("label"), permissionLabel(id));
      row.insert(QStringLiteral("current"), id == current);
      rows.append(row);
    }
  }
  return rows;
}

QVariantMap planState(const QJsonValue &projectionValue) {
  QVariantMap row;
  if (!projectionValue.isObject()) {
    row.insert(QStringLiteral("known"), false);
    row.insert(QStringLiteral("active"), false);
    return row;
  }
  const QJsonObject plan = projectionValue.toObject();
  const bool active = plan.value(QStringLiteral("active")).toBool();
  const bool pending = plan.value(QStringLiteral("pending")).toBool();
  row.insert(QStringLiteral("known"), true);
  row.insert(QStringLiteral("active"), pending ? !active : active);
  row.insert(QStringLiteral("pending"), pending);
  return row;
}

QVariantMap imageLimits(const QJsonValue &projectionValue) {
  QVariantMap row;
  const QJsonObject limits = projectionValue.toObject();
  row.insert(QStringLiteral("maxImageBytes"),
             limits.value(QStringLiteral("maxImageBytes")).toInt(5 * 1024 * 1024));
  row.insert(QStringLiteral("maxImagesPerMessage"), limits.value(QStringLiteral("maxImagesPerMessage")).toInt(20));
  row.insert(QStringLiteral("maxMessageImageBytes"),
             limits.value(QStringLiteral("maxMessageImageBytes")).toInt(100 * 1024 * 1024));
  return row;
}

QVariantList slashItems(const QVariantList &skills, const QVariantList &permissions, bool planActive) {
  QVariantList rows;
  QVariantMap plan;
  plan.insert(QStringLiteral("kind"), QStringLiteral("command"));
  plan.insert(QStringLiteral("line"), planActive ? QStringLiteral("/plan off") : QStringLiteral("/plan"));
  plan.insert(QStringLiteral("title"), planActive ? QStringLiteral("退出计划") : QStringLiteral("计划"));
  plan.insert(QStringLiteral("detail"), planActive ? QStringLiteral("关闭计划模式") : QStringLiteral("进入计划模式"));
  rows.append(plan);
  for (const QVariant &value : permissions) {
    const QVariantMap option = value.toMap();
    const QString id = option.value(QStringLiteral("id")).toString();
    if (id.isEmpty()) {
      continue;
    }
    QVariantMap row;
    row.insert(QStringLiteral("kind"), QStringLiteral("permission"));
    row.insert(QStringLiteral("line"), QStringLiteral("/permission ") + id);
    row.insert(QStringLiteral("title"), option.value(QStringLiteral("label")).toString());
    row.insert(QStringLiteral("detail"), QStringLiteral("切换本会话权限"));
    rows.append(row);
  }
  for (const QVariant &value : skills) {
    const QVariantMap skill = value.toMap();
    const QString name = skill.value(QStringLiteral("name")).toString();
    if (name.isEmpty()) {
      continue;
    }
    QVariantMap row;
    row.insert(QStringLiteral("kind"), QStringLiteral("skill"));
    row.insert(QStringLiteral("line"), QLatin1Char('/') + name + QLatin1Char(' '));
    row.insert(QStringLiteral("title"), name);
    row.insert(QStringLiteral("detail"), skill.value(QStringLiteral("description")).toString());
    rows.append(row);
  }
  return rows;
}

QString apiKeyFailure(const QString &draft) {
  if (draft.isEmpty()) {
    return {};
  }
  const QString value = draft.trimmed();
  if (value.isEmpty()) {
    return QStringLiteral("密钥不能只含空白");
  }
  static const QRegularExpression envLine(QStringLiteral("^[A-Z][A-Z0-9_]*=[^=]"));
  if (envLine.match(value).hasMatch()) {
    return QStringLiteral("请粘贴密钥本身，不要带环境变量名");
  }
  const QChar first = value.front();
  if ((first == QLatin1Char('"') || first == QLatin1Char('\'') || first == QLatin1Char('`')) &&
      value.size() > 1 && value.back() == first) {
    return QStringLiteral("密钥不要带引号");
  }
  for (const QChar ch : value) {
    const ushort code = ch.unicode();
    if (code < 0x21 || code > 0x7E) {
      return QStringLiteral("密钥含非法字符");
    }
  }
  return {};
}

QString imageMediaType(const QString &path) {
  const QString suffix = QFileInfo(path).suffix().toLower();
  if (suffix == QLatin1String("png")) {
    return QStringLiteral("image/png");
  }
  if (suffix == QLatin1String("jpg") || suffix == QLatin1String("jpeg")) {
    return QStringLiteral("image/jpeg");
  }
  if (suffix == QLatin1String("webp")) {
    return QStringLiteral("image/webp");
  }
  if (suffix == QLatin1String("gif")) {
    return QStringLiteral("image/gif");
  }
  return {};
}

bool onboardingNeeded(const QVariantList &providers) {
  bool anyUsable = false;
  bool officialMissing = false;
  for (const QVariant &value : providers) {
    const QVariantMap row = value.toMap();
    if (row.value(QStringLiteral("usable")).toBool()) {
      anyUsable = true;
    }
    if (row.value(QStringLiteral("official")).toBool() && row.value(QStringLiteral("active")).toBool() &&
        !row.value(QStringLiteral("configured")).toBool() && row.value(QStringLiteral("credentialWritable")).toBool()) {
      officialMissing = true;
    }
  }
  return !anyUsable && officialMissing;
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
