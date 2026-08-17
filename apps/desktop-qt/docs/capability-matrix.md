# Capability matrix

| id | web-anchor | qt-target | rpc-methods | milestone | status | verify |
|---|---|---|---|---|---|---|
| desktop.shell.window | apps/web | pages/shell/MainShell.qml | — | M1 | done | 窗口可见，展示水墨主壳与动态连接状态 |
| desktop.host.spawn | — | services/host/HostProcess | — | M1 | done | 子进程存在且回环监听（tst_host_args + live 启动均通过） |
| desktop.rpc.host_describe | connection | services/rpc/RpcClient | host.describe | M1 | done | RpcClient 单测通过；HostProcess 联动解析端口并完成握手展示 |
| desktop.ink.tokens | ui-theme | styles/tokens/InkTokens.qml | — | M1 | done | 水墨色系 Token 体系完整，宣纸质感底色与主辅文字色适配 |
| desktop.ink.motion_reduce | — | styles/motion/MotionBudget.qml | — | M1 | done | 开关切换 reduceMotion 即时禁用 InkBloom 动效 |
| ui-layout | ui-layout | pages/shell/MainShell.qml | — | M2 | done | 书房三栏：侧栏、卷轴、输入；权限/问答条与设置册叠在壳内 |
| ui-sidebar | ui-sidebar | components/chrome/Sidebar.qml | session.list, session.create | M2 | done | ListView reuseItems；会话列表与新会话 |
| ui-workspace | ui-workspace | components/chrome/Sidebar.qml | workspace.list | M2 | done | 点工作区名切换；按 sessionIds 过滤，无假成功 |
| ui-conversation | ui-conversation | components/chat/ConversationPane.qml | session.history, session.prompt, events.mux | M2 | done | 历史折页；mux 流式 chunk 按帧合并；reuseItems |
| ui-model-selection | ui-model-selection | components/chat/Composer.qml | session.models, session.selectModel | M2 | done | 输入区模型名展开目录后选择 |
| ui-tool | ui-tool | components/chat/ToolCard.qml | session.history view, events.mux | M3 | done | generic/terminal 等 card 字段的水墨帖 |
| ui-permission-presets | ui-permission-presets | components/chat/ApprovalStrip.qml | events.mux approval, /api/respond | M3 | done | 卷轴下沿允/却，无系统对话框 |
| ui-user-questions | ui-user-questions | components/chat/QuestionStrip.qml | events.mux question, /api/respond | M3 | done | 选项或书答，整批 respond |
| ui-settings | ui-settings | pages/settings/SettingsPane.qml | settings.describe, settings.openDocument | M4 | done | 标题「册」打开内嵌一览 |
| ui-settings-general | ui-settings-general | pages/settings/SettingsPane.qml | settings.describe | M4 | wip | 列出命名空间；表单编辑未做 |
| ui-settings-models | ui-settings-models | pages/settings | — | M4 | todo | 模型设置仍走会话模型芯片 |
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
| ui-theme | ui-theme | styles/themes | — | M1 | done | 水墨浅色主题激活（InkLight）；InkTokens 提供基础调色板 |
| ui-trajectory | ui-trajectory | — | — | M5 | todo | — |
| ui-workflow-run | ui-workflow-run | — | — | M5 | todo | — |
| web.pwa | apps/web manifest | — | — | — | waived | Web 专有，不移植 |
| web.hmr | client/hmr | — | — | — | waived | 开发态 Web 专有 |
