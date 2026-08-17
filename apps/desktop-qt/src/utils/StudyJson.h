#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QSet>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace dsh::study {

/** Human-readable RPC error; prefers `message` on an error object. */
QString rpcErrorMessage(const QJsonValue &resultOrError);

/** Sidebar title: projection `title`, else 「新会话」 when blank, else cwd basename, else id prefix. */
QString displayTitle(const QJsonObject &sessionItem);

/** Workspace label: `title`, else path basename, else 「未入席」. */
QString workspaceTitle(const QJsonObject &workspaceItem);

/** First `workspace.list` item, or empty object when the list is empty. */
QJsonObject firstWorkspace(const QJsonObject &listValue);

/** Workspace matching `workspaceId`, else first item. */
QJsonObject workspaceById(const QJsonObject &listValue, const QString &workspaceId);

QSet<QString> archivedSessionIds(const QJsonObject &listValue);

/** Sidebar workspace rows from `workspace.list`. */
QVariantList workspaceRows(const QJsonObject &listValue);

/**
 * Session sidebar rows. Subagent / forked-child rows are omitted.
 * When `allowIds` is non-null, only those ids (minus archived) remain.
 */
QVariantList sessionRows(const QJsonArray &items, const QSet<QString> *allowIds,
                         const QSet<QString> &archived);

/** Unwrap a history entry or a live session event to the inner event object. */
QJsonObject unwrapEvent(const QJsonValue &entry);

/** Text from a content-block array. */
QString textFromContent(const QJsonValue &content);

/** User/assistant/tool text from event data (`content` or nested `message.content`). */
QString eventText(const QJsonObject &data);

/** Tool card fields derived from a `tool/call` or `tool/result` plus optional host view. */
QVariantMap toolRow(const QJsonObject &event, const QJsonValue &view);

QJsonObject promptPayload(const QString &sessionId, const QString &text);
QJsonObject promptPayload(const QString &sessionId, const QString &text, const QVariantList &images);
QJsonObject createPayload(const QString &workspaceId);
QJsonObject modelsPayload(const QString &sessionId);
QJsonObject selectModelPayload(const QString &sessionId, const QString &provider, const QString &model);
QJsonObject cancelPayload(const QString &sessionId);
QJsonObject settingsUpdatePayload(const QString &ns, const QString &key, const QJsonValue &value, int revision);
QJsonObject credentialsDescribePayload(const QStringList &refs);
QJsonObject credentialsSetPayload(const QString &ref, const QString &value);
QJsonObject skillListPayload(const QString &sessionId);
QJsonObject subagentListPayload(const QString &parentSessionId);
QJsonObject subagentInterruptPayload(const QString &parentSessionId, const QString &childSessionId);
QJsonObject agentPresetSelectPayload(const QString &sessionId, const QString &agentPreset);

QVariantList modelOptions(const QJsonObject &modelsValue);
QString modelLabel(const QJsonObject &modelsValue);
QVariantList settingsNamespaces(const QJsonObject &describeValue);
QVariantList settingsFields(const QJsonObject &describeValue);
QVariantList providerRows(const QJsonObject &providersValue, const QJsonObject &describeValue,
                          const QJsonObject &credentialsValue);
QVariantList skillRows(const QJsonObject &listValue);
QVariantList subagentRows(const QJsonObject &listValue);
QVariantList presetRows(const QJsonObject &listValue);
QVariantList jobRows(const QJsonArray &jobs);
QVariantList permissionOptions(const QJsonValue &projectionValue, const QJsonObject &describeValue);
QString permissionLabel(const QString &id);
QVariantMap planState(const QJsonValue &projectionValue);
QVariantMap imageLimits(const QJsonValue &projectionValue);
QVariantList slashItems(const QVariantList &skills, const QVariantList &permissions, bool planActive);
QString namespaceTitle(const QString &ns);
QString fieldTitle(const QString &key);
QString appliesLabel(const QString &applies);
QString jobStatusLabel(const QString &status);
QString apiKeyFailure(const QString &draft);
QString imageMediaType(const QString &path);
bool onboardingNeeded(const QVariantList &providers);

/** First blank row's sessionId, or empty. */
QString blankSessionId(const QVariantList &rows);

/** Projection title string when `key` is title, else empty. */
QString projectionTitleValue(const QString &key, const QJsonValue &value);

}  // namespace dsh::study
