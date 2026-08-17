#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QSet>
#include <QString>
#include <QVariantList>

namespace dsh::study {

/** Human-readable RPC error; prefers `message` on an error object. */
QString rpcErrorMessage(const QJsonValue &resultOrError);

/** Sidebar title: projection `title`, else 「新会话」 when blank, else cwd basename, else id prefix. */
QString displayTitle(const QJsonObject &sessionItem);

/** Workspace label: `title`, else path basename, else 「未入席」. */
QString workspaceTitle(const QJsonObject &workspaceItem);

/** First `workspace.list` item, or empty object when the list is empty. */
QJsonObject firstWorkspace(const QJsonObject &listValue);

QSet<QString> archivedSessionIds(const QJsonObject &listValue);

/**
 * Session sidebar rows. Subagent / forked-child rows are omitted.
 * When `allowIds` is non-null, only those ids (minus archived) remain.
 */
QVariantList sessionRows(const QJsonArray &items, const QSet<QString> *allowIds,
                         const QSet<QString> &archived);

/** `user/message` (source.kind user or absent) and `assistant/message` text rows. */
QVariantList messageRows(const QJsonArray &historyEvents);

QJsonObject promptPayload(const QString &sessionId, const QString &text);
QJsonObject createPayload(const QString &workspaceId);

/** First blank row's sessionId, or empty. */
QString blankSessionId(const QVariantList &rows);

QString textFromContent(const QJsonValue &content);

}  // namespace dsh::study
