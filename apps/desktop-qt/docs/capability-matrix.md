# Capability matrix

| id | web-anchor | qt-target | rpc-methods | milestone | status | verify |
|---|---|---|---|---|---|---|
| desktop.shell.window | apps/web | pages/shell/MainShell.qml | — | M1 | todo | 窗口可见 |
| desktop.host.spawn | — | services/host/HostProcess | — | M1 | todo | 子进程存在且回环监听 |
| desktop.rpc.host_describe | connection | services/rpc/RpcClient | host.describe | M1 | todo | 返回 ok 并展示摘要 |
| desktop.ink.tokens | ui-theme | styles/tokens/InkTokens.qml | — | M1 | todo | 浅/深切换 |
| desktop.ink.motion_reduce | — | styles/motion/MotionBudget.qml | — | M1 | todo | 开关关闭墨晕 |
| ui-layout | ui-layout | pages/* | — | M2 | todo | — |
| ui-sidebar | ui-sidebar | pages/session | — | M2 | todo | — |
| ui-workspace | ui-workspace | pages/workspace | — | M2 | todo | — |
| ui-conversation | ui-conversation | pages/session | — | M2 | todo | — |
| ui-model-selection | ui-model-selection | components/chat | — | M2 | todo | — |
| ui-tool | ui-tool | — | — | M3 | todo | — |
| ui-permission-presets | ui-permission-presets | — | — | M3 | todo | — |
| ui-user-questions | ui-user-questions | — | — | M3 | todo | — |
| ui-settings | ui-settings | pages/settings | — | M4 | todo | — |
| ui-settings-general | ui-settings-general | pages/settings | — | M4 | todo | — |
| ui-settings-models | ui-settings-models | pages/settings | — | M4 | todo | — |
| ui-settings-plugins | ui-settings-plugins | pages/settings | — | M4 | todo | — |
| ui-settings-plugin-inventory | ui-settings-plugin-inventory | pages/settings | — | M4 | todo | — |
| ui-agent-preset | ui-agent-preset | — | — | M5 | todo | — |
| ui-attachment | ui-attachment | — | — | M5 | todo | — |
| ui-commands | ui-commands | — | — | M5 | todo | — |
| ui-deliverables | ui-deliverables | — | — | M5 | todo | — |
| ui-directory-picker-browse | ui-directory-picker-browse | — | — | M5 | todo | — |
| ui-directory-picker-native | ui-directory-picker-native | — | — | M5 | todo | — |
| ui-goal | ui-goal | — | — | M5 | todo | — |
| ui-input-trigger | ui-input-trigger | — | — | M5 | todo | — |
| ui-jobs | ui-jobs | — | — | M5 | todo | — |
| ui-message-feedback | ui-message-feedback | — | — | M5 | todo | — |
| ui-plan | ui-plan | — | — | M5 | todo | — |
| ui-primitives | ui-primitives | components/* | — | M5 | todo | — |
| ui-skill | ui-skill | — | — | M5 | todo | — |
| ui-slots | ui-slots | — | — | M5 | todo | — |
| ui-subagent | ui-subagent | — | — | M5 | todo | — |
| ui-theme | ui-theme | styles/themes | — | M1 | todo | 水墨浅/深 |
| ui-trajectory | ui-trajectory | — | — | M5 | todo | — |
| ui-workflow-run | ui-workflow-run | — | — | M5 | todo | — |
| web.pwa | apps/web manifest | — | — | — | waived | Web 专有，不移植 |
| web.hmr | client/hmr | — | — | — | waived | 开发态 Web 专有 |
