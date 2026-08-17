### Task 5: RpcClient unary + host.describe

**Files:**
- Create: `apps/desktop-qt/src/services/rpc/RpcTypes.h`
- Create: `apps/desktop-qt/src/services/rpc/RpcClient.h`
- Create: `apps/desktop-qt/src/services/rpc/RpcClient.cpp`
- Create: `apps/desktop-qt/tests/tst_rpc_envelope.cpp`
- Create: `apps/desktop-qt/cmake/RpcSources.cmake`
- Modify: `apps/desktop-qt/CMakeLists.txt`（在 `# --- rpc sources added by Task 5 agent ---` 标记处 include）

**Interfaces:**
- Consumes: `HostProcess::port()`（Task 6 接线；本 Task 仅 `setBaseUrl`）
- Produces:
  - `RpcClient::setBaseUrl(const QUrl &)` 例如 `http://127.0.0.1:3080`
  - `void callUnary(const QString &method, const QJsonValue &payload, const std::function<void(bool ok, QJsonValue resultOrError)> &done)`
  - Wire JSON：

```json
{
  "type": "client-request",
  "rpcId": "<uuid>",
  "method": "host.describe",
  "payload": {}
}
```

  POST 到 `http://127.0.0.1:<port>/api/host.describe`（路径以仓库 `apiproxy` 实际路由为准；实现前打开 `packages/host/apiproxy` 中客户端调用或 webserver 路由确认。若为 `/api/<method>` 则 method 含点号，需确认编码。）

  响应期望：

```json
{
  "type": "server-response",
  "rpcId": "<same>",
  "result": { "ok": true, "value": { } }
}
```

- [ ] **Step 1: 对照现网路由**

只读检索：

```powershell
rg -n "host\.describe|/api/" packages/host/apiproxy/src packages/client/connection/src --glob "*.ts" | Select-Object -First 40
```

把真实 URL 模板写进 `RpcClient.cpp` 注释与常量。

- [ ] **Step 2: 单测信封**

`tst_rpc_envelope.cpp` 断言：`RpcClient::makeClientRequest("host.describe", QJsonObject{})` 产生的对象含 `type==client-request`、非空 `rpcId`、`method==host.describe`。

- [ ] **Step 3: 跑测失败 → 实现 makeClientRequest / parseServerResponse → 跑测通过**

- [ ] **Step 4: 集成：对已启动 Host 调用 `host.describe`**

失败时错误不得吞掉；`ConnectionHook` 显示 `result.error` 或网络错误字符串。（ConnectionHook 属 Task 6；本 Task 提供 `callUnary` 即可。）

- [ ] **Step 5: 矩阵** `desktop.rpc.host_describe` → `done`（live 手测可选）

- [ ] **Step 6: Commit**

```bash
git commit -am "feat(desktop-qt): add RpcClient unary and host.describe handshake"
```

---
