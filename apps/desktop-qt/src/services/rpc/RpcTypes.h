#pragma once

#include <QString>

namespace dsh::rpc {

inline constexpr char kClientRequestType[] = "client-request";
inline constexpr char kServerResponseType[] = "server-response";
inline constexpr char kServerRequestType[] = "server-request";
inline constexpr char kClientResponseType[] = "client-response";

inline constexpr char kFieldType[] = "type";
inline constexpr char kFieldRpcId[] = "rpcId";
inline constexpr char kFieldMethod[] = "method";
inline constexpr char kFieldPayload[] = "payload";
inline constexpr char kFieldResult[] = "result";
inline constexpr char kFieldOk[] = "ok";
inline constexpr char kFieldValue[] = "value";
inline constexpr char kFieldError[] = "error";

inline constexpr char kMethodHostDescribe[] = "host.describe";
inline constexpr char kMethodWorkspaceList[] = "workspace.list";
inline constexpr char kMethodSessionList[] = "session.list";
inline constexpr char kMethodSessionCreate[] = "session.create";
inline constexpr char kMethodSessionHistory[] = "session.history";
inline constexpr char kMethodSessionPrompt[] = "session.prompt";
inline constexpr char kMethodSessionModels[] = "session.models";
inline constexpr char kMethodSessionSelectModel[] = "session.selectModel";
inline constexpr char kMethodSessionCancel[] = "session.cancel";
inline constexpr char kMethodSettingsDescribe[] = "settings.describe";
inline constexpr char kMethodSettingsOpenDocument[] = "settings.openDocument";

/** Route prefix shared with packages/client/connection/src/api-path.ts (`API_PATH`). */
inline constexpr char kApiPathPrefix[] = "/api";
inline constexpr char kRespondPath[] = "/api/respond";
inline constexpr char kMuxEventsPath[] = "/api/events.mux";
inline constexpr char kHostEventsPath[] = "/api/events.host";

}  // namespace dsh::rpc
