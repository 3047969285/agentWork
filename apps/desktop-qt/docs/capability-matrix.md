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
| ui-permission-presets | ui-permission-presets | components/chat/Composer.qml, SettingsPane | session.prompt /permission, settings.update, events.mux | M3 | done | 输入区权限芯片、设置册默认项、卷轴下沿允/却 |
| ui-user-questions | ui-user-questions | components/chat/QuestionStrip.qml | events.mux question, /api/respond | M3 | done | 选项或书答，整批 respond |
| ui-settings | ui-settings | pages/settings/SettingsPane.qml | settings.describe, settings.update, settings.openDocument | M4 | done | 设置册：概览/常规/模型/权限/技能/子代理/预设 |
| ui-settings-general | ui-settings-general | pages/settings/SettingsPane.qml | settings.describe, settings.update | M4 | done | 常规字段可写（字符串/开关/枚举/密钥） |
| ui-settings-models | ui-settings-models | pages/settings/SettingsPane.qml, OnboardingPane.qml | llm.providers, credentials.describe/set, settings.update | M4 | done | 模型供应方与密钥；缺密钥弹出入门 |
| ui-settings-plugins | ui-settings-plugins | pages/settings | — | M4 | todo | 宿主无插件清单 unary |
| ui-settings-plugin-inventory | ui-settings-plugin-inventory | pages/settings | — | M4 | todo | 宿主无插件清单 unary |
| ui-agent-preset | ui-agent-preset | pages/settings/SettingsPane.qml | agentPreset.list, agentPreset.select | M5 | done | 设置册「预设」点选当前会话 |
| ui-attachment | ui-attachment | components/chat/Composer.qml | session.prompt image parts | M5 | done | 附页选图，随 prompt 提交 |
| ui-commands | ui-commands | components/chat/Composer.qml | session.prompt `/…` | M5 | done | 输入 `/` 出斜杠目录（计划/权限/技能） |
| ui-deliverables | ui-deliverables | — | — | M5 | todo | 无独立 RPC |
| ui-directory-picker-browse | ui-directory-picker-browse | — | host.listDirectory | M5 | todo | 未接目录浏览 |
| ui-directory-picker-native | ui-directory-picker-native | — | host.pickDirectory | M5 | todo | 未接原生选目录 |
| ui-goal | ui-goal | — | goal.* | M5 | todo | 未接目标 RPC |
| ui-input-trigger | ui-input-trigger | components/chat/Composer.qml | skill.list + session.prompt | M5 | done | 斜杠目录复用技能与命令 |
| ui-jobs | ui-jobs | components/chat/JobStrip.qml | events.mux session/jobs | M5 | done | 卷轴下沿差事条 |
| ui-message-feedback | ui-message-feedback | — | — | M5 | todo | 无独立 RPC |
| ui-plan | ui-plan | components/chat/Composer.qml | session.prompt /plan, session/projection plan | M5 | done | 计划芯片；问答条识别计划待审 |
| ui-primitives | ui-primitives | components/* | — | M5 | done | TapHandler 水墨控件 |
| ui-skill | ui-skill | pages/settings, Composer | skill.list | M5 | done | 技能列表与斜杠插入 |
| ui-slots | ui-slots | — | — | M5 | waived | Web 槽位组合模型，Qt 以 pages 直装 |
| ui-subagent | ui-subagent | pages/settings/SettingsPane.qml | subagent.list, subagent.interrupt | M5 | done | 设置册列出并可打断可续子代理 |
| ui-theme | ui-theme | styles/themes | — | M1 | done | 水墨浅色主题激活（InkLight）；InkTokens 提供基础调色板 |
| ui-trajectory | ui-trajectory | — | — | M5 | todo | — |
| ui-workflow-run | ui-workflow-run | — | — | M5 | todo | — |
| web.pwa | apps/web manifest | — | — | — | waived | Web 专有，不移植 |
| web.hmr | client/hmr | — | — | — | waived | 开发态 Web 专有 |
