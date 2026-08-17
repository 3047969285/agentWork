# Task 4 Report: HostProcess 自管拉起 dsh

## 交付概述
已完成 Task 4：实现 Qt/C++ `HostProcess` 与 `AppConstants`，负责管理 `dsh --profile web` 子进程的生命周期，支持自动发现 repo 根目录与可执行文件入口、回环启动参数构造（`127.0.0.1`，禁用 `0.0.0.0`）、进程就绪输出端口提取，以及安全的超时停止与强制清理。

## 文件变动清单
- **新建** `apps/desktop-qt/src/constants/AppConstants.h`：定义回环 host、web profile、超时与重试常量。
- **新建** `apps/desktop-qt/src/services/host/HostProcess.h`：定义 `HostProcess` 接口与信号。
- **新建** `apps/desktop-qt/src/services/host/HostProcess.cpp`：实现 `start`、`stop`、`buildArguments`、`parsePortFromOutput`、`resolveProgram`、`findRepoRoot` 等。
- **新建** `apps/desktop-qt/tests/tst_host_args.cpp`：单元测试覆盖回环参数构造、端口解析、程序解析、仓库根路径探测，以及可选 live 进程测试。
- **修改** `apps/desktop-qt/CMakeLists.txt`：加入 `HostProcess.cpp` 到主程序目标，并添加 `tst_host_args` 测试目标（保留 `# --- rpc sources added by Task 5 agent ---` 标记）。
- **更新** `apps/desktop-qt/docs/capability-matrix.md`：标记 `desktop.host.spawn` 为 `done`。

## 验证与测试结果
1. **CLI 行为确认：** 验证 `pnpm dsh --profile web --help`，确认 `--host <host>`、`--port <port>`（`0` 代表由 OS 分配空闲端口）、输出格式 `dsh web: http://127.0.0.1:<port>`。
2. **TDD 单元测试：** `tst_host_args` 100% 通过（断言包含 `--host 127.0.0.1`，绝不包含 `0.0.0.0`，正确匹配 URL 端口）。
3. **真实子进程 Live 启停：** 在已运行 `pnpm run build` 的环境下，通过 `HostProcess` 成功启动自管 Node 子进程，动态监听到 `dsh web: http://127.0.0.1:<port>`，提取端口成功并随后优雅 `stop()` 退出。
4. **全量构建验证：** MSVC 2022 x64 Release 配置下 `dsh-desktop`、`tst_host_args`、`tst_rpc_envelope` 全部编译通过且 ctest 全绿。
