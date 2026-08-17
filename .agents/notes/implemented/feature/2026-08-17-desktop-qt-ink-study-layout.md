# Agent Note: Desktop Qt ink study layout

Status: implemented

English | [中文](2026-08-17-desktop-qt-ink-study-layout.zh.md)

## Problem

The M1 Qt shell completed `host.describe` and then filled the window with a centered connection paragraph, version, port, and a Material-like reduce-motion switch. That is a handshake dump, not a usable study. Users cannot list sessions, open one, or send a prompt. Qt `GradientStop` `"transparent"` also interpolates toward black, so the paper wash reads as a broken gray slab.

## Decision

`apps/desktop-qt` keeps self-managed `HostProcess` and unary `RpcClient`. After `host.describe` succeeds, `Application` loads `workspace.list` then `session.list`, exposes them through `StudyHook`, and the QML shell is a three-pane 书房: sidebar, conversation scroll, composer.

Connection state is a small rotated ink seal in `TitleBar` plus inline `statusText`; reduce-motion is the title-bar `动`/`静` toggle. `session.create` reuses a listed blank row when one exists; otherwise it posts `{ workspaceId }` or `{}`. `session.prompt` uses `mode: "queue"` and `content: [{ type: "text", text }]`. Failure strings come from the RPC error object; the UI does not invent a successful send. Assistant text is not a mux WebSocket: after accept, `session.history` polls until a newer `assistant/message` seq appears or twenty one-second attempts elapse.

`dsh::study` parsers in `utils/StudyJson` own title, row, and prompt JSON so QML stays presentation-only. Paper gradients keep the xuan-paper hue and vary only alpha.

## Alternatives considered

**Keep the centered handshake status as the M1 window.** Rejected because it blocks the product loop the window exists for.

**Embed the web GUI in a Qt WebView.** Rejected by the desktop design: the shell is native QML over the existing host, not a second browser.

**Subscribe to `events.mux` in this pass.** Rejected as the M2 stream milestone; unary history polling is enough to show a reply without faking one.

**Mint a local session id and paint a fake assistant bubble on send.** Rejected because a false success hides host admission failures.

## Consequences

A connected desktop can enter a session, compose, and see history text. Workspace switching, model picker, tool cards, approvals, and live token streaming remain later matrix rows. History polling can miss a slow turn after twenty seconds; selecting the session again reloads. The web product entry is unchanged.
