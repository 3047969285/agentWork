# Agent Note: Desktop Qt ink study layout

Status: implemented

English | [中文](2026-08-17-desktop-qt-ink-study-layout.zh.md)

## Problem

The M1 Qt shell completed `host.describe` and then filled the window with a centered connection paragraph, version, port, and a Material-like reduce-motion switch. That is a handshake dump, not a usable study. Users cannot list sessions, open one, or send a prompt. Qt `GradientStop` `"transparent"` also interpolates toward black, so the paper wash reads as a broken gray slab.

## Decision

`apps/desktop-qt` keeps self-managed `HostProcess` and unary `RpcClient`. After `host.describe` succeeds, `Application` loads `workspace.list` then `session.list`, opens downlink-only WebSockets to `/api/events.mux` and `/api/events.host` (the same paths as `packages/client/connection`), and exposes study state through `StudyHook`. The QML shell is a three-pane 书房: sidebar, conversation scroll, composer.

Connection state is a small rotated ink seal in `TitleBar` plus inline `statusText`; reduce-motion is the title-bar `动`/`静` toggle and is honored on hover/press timings and InkBloom. `session.create` reuses a listed blank row when one exists; otherwise it posts `{ workspaceId }` or `{}`. `session.prompt` uses `mode: "queue"` and `content: [{ type: "text", text }]`. Failure strings come from the RPC error object; the UI does not invent a successful send.

Workspace switching is a client-side filter of `workspace.list` items by `sessionIds` (there is no `workspace.select` RPC). Model choice uses `session.models` and `session.selectModel`. Approvals and questions answer through `POST /api/respond` with a `client-response` envelope echoing the mux frame's `rpcId`. Settings `册` calls `settings.describe` and `settings.openDocument`.

Live assistant text is `assistant/chunk` `text-delta` on the mux stream, flushed on a 16 ms timer into `TranscriptModel` (`dataChanged` on one row, not a model reset). `session.history` is the reconnect baseline. Session and transcript `ListView`s use `reuseItems`, `cacheBuffer`, and required-property delegates. Scroll-to-end coalesces on a 16 ms timer. InkBloom is title/empty-scroll only. Qt has no WebSockets module in this prefix, so `EventStream` speaks RFC 6455 over `QTcpSocket`. The scroll shows `selectedTitle` as a 题签.

`dsh::study` parsers in `utils/StudyJson` own title, row, tool-card, and prompt JSON so QML stays presentation-only. Paper gradients keep the xuan-paper hue and vary only alpha; `PaperBackground` paints cached fiber strokes once, never on list delegates.

## Alternatives considered

**Keep the centered handshake status as the M1 window.** Rejected because it blocks the product loop the window exists for.

**Embed the web GUI in a Qt WebView.** Rejected by the desktop design: the shell is native QML over the existing host, not a second browser.

**Poll `session.history` as the live reply path.** Rejected: a one-second full-list replace misses slow turns and relayouts the scroll. Mux chunk frames batched onto one `TranscriptModel` row are the live path; history remains the snapshot used on session select and reconnect.

**Mint a local session id and paint a fake assistant bubble on send.** Rejected because a false success hides host admission failures.

**QMessageBox / native file dialogs for approval, questions, or workspace create.** Rejected: product chrome stays inline; directory pickers remain later matrix rows rather than a system dialog.

## Consequences

A connected desktop can switch workspaces, enter a session, choose a model, compose, watch streamed text, render tool cards, and answer approvals/questions inline. Settings lists namespaces and can open the host document; it does not edit schema forms. Attachments, slash commands, jobs, plan, skills, subagents, and native directory pickers remain later matrix rows. Force-killing the UI can leave a `dsh --profile web` child until the next clean quit. The web product entry is unchanged.

## Testing

`tst_study_json` covers titles, nested `assistant/message` text, tool-card views, workspace rows, and `TranscriptModel` history folding. `tst_rpc_envelope` covers `client-request` and `client-response` envelopes. Release `dsh-desktop.exe` must load the QML module after `windeployqt`.
