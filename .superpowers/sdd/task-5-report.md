# Task 5 Report: RpcClient unary + host.describe

**Status:** DONE
**Branch:** `feat/desktop-qt-ink-m1`
**Date:** 2026-08-17

## Summary

Implemented unary HTTP RpcClient for the desktop Qt shell: `client-request` envelope minting, `server-response` parsing, and async `callUnary` over POST `/api/<method>`. Unit tests pass; live `host.describe` is optional via `DSH_DESKTOP_RPC_BASE_URL`.

## Wire contract (verified against repo)

| Item | Source | Value |
|---|---|---|
| API prefix | `packages/client/connection/src/api-path.ts` | `/api` |
| Unary path | `packages/host/apiproxy/src/fetch/client.ts` `callUnary` | `POST /api/${method}` |
| Example | `host.describe` | `POST http://127.0.0.1:<port>/api/host.describe` |
| Request body | `packages/host/apiproxy/src/api/rpc.ts` | `{ type, rpcId, method, payload }` |
| Response body | same | `{ type: server-response, rpcId, result: { ok, value \| error } }` |

Documented in `RpcClient.cpp` (`unaryUrlForMethod`) and `RpcTypes.h` (`kApiPathPrefix`).

## Deliverables

| Item | Path |
|---|---|
| Envelope constants | `apps/desktop-qt/src/services/rpc/RpcTypes.h` |
| RpcClient API | `apps/desktop-qt/src/services/rpc/RpcClient.h` |
| HTTP implementation | `apps/desktop-qt/src/services/rpc/RpcClient.cpp` |
| Unit tests | `apps/desktop-qt/tests/tst_rpc_envelope.cpp` |
| CMake fragment | `apps/desktop-qt/cmake/RpcSources.cmake` |
| Brief | `.superpowers/sdd/task-5-brief.md` |

## Public API

- `RpcClient::setBaseUrl(QUrl)` — e.g. `http://127.0.0.1:3080`
- `RpcClient::callUnary(method, payload, done)` — async; `done(false, …)` on transport/parse/business error (errors not swallowed)
- `RpcClient::makeClientRequest` / `parseServerResponse` — static helpers for envelope round-trip

## Build & test

```powershell
& "C:\Program Files\CMake\bin\cmake.exe" -S apps/desktop-qt -B apps/desktop-qt/build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=D:/Qt/6.8.3/msvc2022_64
& "C:\Program Files\CMake\bin\cmake.exe" --build apps/desktop-qt/build --config Release --target tst_rpc_envelope dsh-desktop
$env:PATH = "D:\Qt\6.8.3\msvc2022_64\bin;" + $env:PATH
apps/desktop-qt/build/Release/tst_rpc_envelope.exe
```

- **Envelope tests:** PASS (3 assertions + 1 optional skip)
- **dsh-desktop:** builds with RpcClient linked
- **ctest:** requires Qt `bin` on `PATH` (exit `0xc0000135` without DLLs); run exe directly or set `PATH` before ctest

### Optional live handshake

```powershell
$env:DSH_DESKTOP_RPC_BASE_URL = "http://127.0.0.1:<port>"
apps/desktop-qt/build/Release/tst_rpc_envelope.exe
```

Runs `hostDescribe_liveOptional`; skipped when env unset.

## Matrix

`desktop.rpc.host_describe` → **wip** (RpcClient ready; ConnectionHook / UI summary in Task 6).

## Scope compliance

- Did **not** edit `services/host/**`, `packages/client/**`, or `apps/web/**`
- CMake: `# --- rpc sources added by Task 5 agent ---` + `include(cmake/RpcSources.cmake)` only
- No push

## Next (Task 6)

Wire `HostProcess::port()` → `RpcClient::setBaseUrl` → `callUnary("host.describe")` in `ConnectionHook` / `Application`.
