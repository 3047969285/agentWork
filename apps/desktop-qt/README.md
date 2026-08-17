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

M1 Task 3 shows the ink paper shell (`MainShell`) at 1280×800 with title bar, connection placeholder, and a reduce-motion toggle. Host process and RPC wiring arrive in later tasks.
