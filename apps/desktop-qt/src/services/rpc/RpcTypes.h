#pragma once

#include <QString>

namespace dsh::rpc {

inline constexpr char kClientRequestType[] = "client-request";
inline constexpr char kServerResponseType[] = "server-response";

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

/** Route prefix shared with packages/client/connection/src/api-path.ts (`API_PATH`). */
inline constexpr char kApiPathPrefix[] = "/api";

}  // namespace dsh::rpc
