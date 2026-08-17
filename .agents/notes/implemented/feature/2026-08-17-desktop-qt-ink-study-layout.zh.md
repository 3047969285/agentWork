# Agent Note: Desktop Qt ink study layout

Status: implemented

[English](2026-08-17-desktop-qt-ink-study-layout.md) | 中文

## Problem

M1 的 Qt 壳在 `host.describe` 成功后，把连接段落、版本、端口和类 Material 的「减少动态效果」开关堆在窗口正中。那是握手摘要，不是能用的书房：用户不能列出会话、进入会话、发送提示。Qt `GradientStop` 的 `"transparent"` 还会向黑色插值，宣纸洗墨看起来像坏掉的灰板。

## Decision

`apps/desktop-qt` 继续自管 `HostProcess` 与 unary `RpcClient`。`host.describe` 成功后，`Application` 依次请求 `workspace.list` 与 `session.list`，经 `StudyHook` 交给 QML；主壳是三栏书房：侧栏、卷轴、输入。

连接态是 `TitleBar` 里一枚小墨印，旁边内联 `statusText`；减少动态效果是标题栏「动 / 静」。`session.create` 若列表里已有空白行则复用，否则 POST `{ workspaceId }` 或 `{}`。`session.prompt` 使用 `mode: "queue"` 与 `content: [{ type: "text", text }]`。失败文案取自 RPC 错误对象，界面不伪造发送成功。助手文本不是 mux WebSocket：入队成功后轮询 `session.history`，直到出现更新的 `assistant/message` seq，或满二十次一秒间隔。

`utils/StudyJson` 里的 `dsh::study` 解析标题、列表行与 prompt JSON，QML 只负责呈现。宣纸渐变停在宣纸色相上，只改变透明度。

## Alternatives considered

**把居中握手状态留作 M1 主窗。** 否决：它挡住窗口本应提供的产品主路径。

**在 Qt WebView 里嵌现网 GUI。** 否决：桌面设计要求原生 QML 消费现有宿主，而不是第二套浏览器。

**本轮订阅 `events.mux`。** 否决：那是 M2 的流式里程碑；unary 历史轮询足以显示回复且不伪造。

**本地捏造 session id，发送后画假助手气泡。** 否决：假成功会盖住宿主入队失败。

## Consequences

已连接的桌面可以进入会话、书写，并看到历史文本。工作区切换、模型选择、工具卡片、审批和 live token 流仍是后续矩阵行。历史轮询在二十秒后可能赶不上慢回合；再点该会话会重新加载。Web 产品入口未改。
