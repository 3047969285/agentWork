# DeepSeek Harness Desktop (Qt/QML)

Native Qt 6 desktop shell for DeepSeek Harness M1. This target is independent of the web GUI (`apps/web`); the web product entry is not removed in this phase.

## Prerequisites

- Windows 10+
- Visual Studio 2022 (MSVC x64)
- Qt 6.5+ with Quick and Network modules (tested with Qt 6.8.3 MSVC2022_64)
- CMake 3.21+

## Configure

Set `CMAKE_PREFIX_PATH` to your Qt installation prefix, for example:

```powershell
cmake -S apps/desktop-qt -B apps/desktop-qt/build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=D:/Qt/6.8.3/msvc2022_64
```

## Build

```powershell
cmake --build apps/desktop-qt/build --config Release
```

The executable is emitted as `apps/desktop-qt/build/Release/dsh-desktop.exe` (Visual Studio multi-config generator).

## Run

```powershell
apps/desktop-qt/build/Release/dsh-desktop.exe
```

M1 delivers the native Qt 6 desktop shell: self-managed `dsh --profile web` subprocess lifecycle on loopback `127.0.0.1`, unary HTTP RPC via `host.describe`, `workspace.list`, `session.list`, `session.create`, `session.history`, and `session.prompt`. The main window is an ink-wash 书房: title seal for connection, sidebar sessions, empty-scroll calligraphy, and a cinnabar send seal. Reduce-motion lives in the title bar (`静`/`动`).

## 书房操作

1. 启动后标题旁墨点表示连接；失败时点「重连」。
2. 左侧选择会话，或点「新会话」（已有空白会话则复用，不伪造成功）。
3. 底部书写后点朱砂「发」或 `Ctrl+Enter`，经 `session.prompt` 入队；卷轴用 `session.history` 轮询补答，不是 WebSocket 流。
4. 「减少动态效果」在标题栏右侧「动/静」二字，悬停可见完整文案。

## M1 验收清单 (Milestone 1 Acceptance)

1. **Release 构建可正常启动：** MSVC 2022 x64 Release 配置下构建的 `dsh-desktop.exe` 能顺利启动并加载 QML 界面。
2. **自管 dsh 宿主回环监听：** 启动后自动拉起 `dsh` 子进程，绑定 `127.0.0.1`，禁用 `0.0.0.0`，并动态解析就绪端口。
3. **完成 host.describe 握手并展示：** `RpcClient` 通过 HTTP POST `/api/host.describe` 发送 `client-request` 信封并解析 `server-response`；连接态收成标题栏墨印，悬停可见版本与端口，主区不再堆状态段落。
4. **水墨沉浸与动效降级：** 浅色宣纸背景（`InkTokens.windowBg`）对比度清晰可读；标题栏「动/静」切换 `MotionBudget.reduceMotion` 后即时关闭墨晕（InkBloom）。
5. **UI 退出安全回收子进程：** 关闭桌面应用主窗口（触发 `aboutToQuit`）后，自管的 `dsh` / `node` 子进程被干净停止，无后台残留。
6. **Web 产品入口完整保留：** 未修改、未破坏且未删除 `apps/web` 与 `packages/client/**`，Web 与桌面版并行不悖。
