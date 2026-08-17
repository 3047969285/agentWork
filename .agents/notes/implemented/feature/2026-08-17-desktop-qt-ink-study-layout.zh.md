# Agent Note: Desktop Qt ink study layout

Status: implemented

[English](2026-08-17-desktop-qt-ink-study-layout.md) | 中文

## Problem

M1 的 Qt 壳在 `host.describe` 成功后，把连接段落、版本、端口和类 Material 的「减少动态效果」开关堆在窗口正中。那是握手摘要，不是能用的书房：用户不能列出会话、进入会话、发送提示。Qt `GradientStop` 的 `"transparent"` 还会向黑色插值，宣纸洗墨看起来像坏掉的灰板。

## Decision

`apps/desktop-qt` 继续自管 `HostProcess` 与 unary `RpcClient`。`host.describe` 成功后，`Application` 加载 `workspace.list` 与 `session.list`，再打开只下行的 WebSocket `/api/events.mux` 与 `/api/events.host`（与 `packages/client/connection` 同路径），经 `StudyHook` 交给 QML。主壳是三栏书房：侧栏、卷轴、输入。

连接态是 `TitleBar` 里一枚小墨印，旁边内联 `statusText`；减少动态效果是标题栏「动 / 静」，并约束悬停/按下时长和 InkBloom。`session.create` 若列表里已有空白行则复用，否则 POST `{ workspaceId }` 或 `{}`。`session.prompt` 使用 `mode: "queue"` 与 `content: [{ type: "text", text }]`。失败文案取自 RPC 错误对象，界面不伪造发送成功。

工作区切换是客户端按 `workspace.list` 的 `sessionIds` 过滤（没有 `workspace.select` RPC）。模型选择走 `session.models` 与 `session.selectModel`。审批与问答通过 `POST /api/respond` 回传 `client-response`，rpcId 回声 mux 帧。设置「册」调用 `settings.describe` 与 `settings.openDocument`。

直播助手文本是 mux 上的 `assistant/chunk` `text-delta`，用 16 ms 定时器刷进 `TranscriptModel`（单行 `dataChanged`，不整表重置）。`session.history` 是重连基线。会话与卷轴 `ListView` 使用 `reuseItems` 与 `cacheBuffer`。InkBloom 只出现在标题与空卷。本套 Qt 前缀没有 WebSockets 模块，`EventStream` 在 `QTcpSocket` 上实现 RFC 6455。

`utils/StudyJson` 里的 `dsh::study` 解析标题、列表行、工具帖与 prompt JSON，QML 只负责呈现。宣纸渐变停在宣纸色相上，只改变透明度；`PaperBackground` 把纤维笔触缓存绘制一次，不画在列表委托上。

## Alternatives considered

**把居中握手状态留作 M1 主窗。** 否决：它挡住窗口本应提供的产品主路径。

**在 Qt WebView 里嵌现网 GUI。** 否决：桌面设计要求原生 QML 消费现有宿主，而不是第二套浏览器。

**用轮询 `session.history` 作为直播回复路径。** 否决：一秒一次整表替换既赶不上慢回合，也会重排卷轴。直播路径是 mux 分片按帧合并到 `TranscriptModel` 一行；历史仍是选会话与重连时的快照。

**本地捏造 session id，发送后画假助手气泡。** 否决：假成功会盖住宿主入队失败。

**用 QMessageBox / 系统文件对话框做审批、问答或新建工作区。** 否决：产品铬层保持内联；目录选择留在后续矩阵行，不引入系统对话框。

## Consequences

已连接的桌面可以切换工作区、进入会话、选择模型、书写、看流式正文、渲染工具帖，并在卷轴下沿回答审批/问答。设置列出命名空间并可打开宿主文书，但不编辑 schema 表单。附件、斜杠命令、任务、计划、技能、子代理与原生目录选择仍是后续矩阵行。强杀 UI 可能留下 `dsh --profile web` 子进程，直到下一次正常退出。Web 产品入口未改。

## Testing

`tst_study_json` 覆盖标题、嵌套 `assistant/message` 文本、工具帖视图、工作区行，以及 `TranscriptModel` 历史折叠。`tst_rpc_envelope` 覆盖 `client-request` 与 `client-response` 信封。Release 的 `dsh-desktop.exe` 必须在 `windeployqt` 之后能加载 QML 模块。
