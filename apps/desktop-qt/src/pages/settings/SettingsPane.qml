import QtQuick
import QtQuick.Controls

Item {
    id: root
    visible: typeof study !== "undefined" && study.settingsOpen
    enabled: visible
    z: visible ? 20 : -1
    anchors.left: parent.left
    anchors.top: parent.top
    width: visible ? parent.width : 0
    height: visible ? parent.height : 0

    function fieldMatches(field, section) {
        return field && field.section === section
    }

    function tabLabel(id) {
        if (id === "overview")
            return qsTr("概览")
        if (id === "general")
            return qsTr("常规")
        if (id === "models")
            return qsTr("模型")
        if (id === "permission")
            return qsTr("权限")
        if (id === "skills")
            return qsTr("技能")
        if (id === "subagents")
            return qsTr("子代理")
        if (id === "presets")
            return qsTr("预设")
        return id
    }

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0.102, 0.086, 0.071, 0.18)
        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: study.closeSettings()
        }
    }

    Rectangle {
        id: sheet
        width: Math.min(480, parent.width - 48)
        height: Math.min(680, parent.height - 72)
        anchors.right: parent.right
        anchors.rightMargin: 24
        anchors.top: parent.top
        anchors.topMargin: 56
        color: InkTokens.scrollPaper
        border.width: 1
        border.color: InkTokens.hairline
        radius: 1

        TapHandler {
            acceptedButtons: Qt.LeftButton
        }

        Text {
            id: heading
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: 18
            anchors.leftMargin: 20
            text: qsTr("设置")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 22
            color: InkTokens.ink900
        }

        Text {
            anchors.top: heading.top
            anchors.right: parent.right
            anchors.rightMargin: 18
            text: qsTr("关闭")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 14
            color: InkTokens.ink500
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                margin: 8
                acceptedButtons: Qt.LeftButton
                onTapped: study.closeSettings()
            }
        }

        Row {
            id: tabs
            anchors.top: heading.bottom
            anchors.topMargin: 10
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.right: parent.right
            anchors.rightMargin: 12
            spacing: 12

            Repeater {
                model: ["overview", "general", "models", "permission", "skills", "subagents", "presets"]
                Text {
                    required property string modelData
                    text: root.tabLabel(modelData)
                    font.family: InkTokens.calligraphyFamily
                    font.pixelSize: 13
                    color: (typeof study !== "undefined" && study.settingsTab === modelData)
                           ? InkTokens.cinnabar : InkTokens.ink500
                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }
                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        onTapped: study.setSettingsSection(modelData)
                    }
                }
            }
        }

        Flickable {
            id: body
            anchors.top: tabs.bottom
            anchors.topMargin: 10
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: actions.top
            anchors.bottomMargin: 8
            clip: true
            contentWidth: width
            contentHeight: column.height
            boundsBehavior: Flickable.StopAtBounds
            flickDeceleration: 2500

            Column {
                id: column
                width: body.width
                spacing: 10

                Column {
                    visible: typeof study !== "undefined" && study.settingsTab === "overview"
                    width: parent.width
                    spacing: 8

                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: qsTr("连接")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 12
                        color: InkTokens.ink500
                    }
                    Text {
                        width: column.width - 36
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        wrapMode: Text.Wrap
                        text: {
                            if (typeof connection === "undefined")
                                return qsTr("等待连接…")
                            if (connection.connected)
                                return qsTr("已连接")
                            if (connection.connecting)
                                return qsTr("正在连接…")
                            if (connection.hasError)
                                return qsTr("连接失败")
                            return qsTr("未连接")
                        }
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 14
                        color: InkTokens.ink900
                    }
                    Text {
                        width: column.width - 36
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        wrapMode: Text.Wrap
                        visible: typeof connection !== "undefined" && connection.statusText.length > 0
                        text: typeof connection !== "undefined" ? connection.statusText : ""
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 13
                        color: InkTokens.ink700
                    }
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: {
                            var version = (typeof connection !== "undefined" && connection.hostVersion.length > 0)
                                          ? connection.hostVersion : qsTr("开发版")
                            var port = (typeof connection !== "undefined" && connection.hostPort > 0)
                                       ? String(connection.hostPort) : qsTr("无")
                            return qsTr("版本") + " " + version + " · " + qsTr("端口") + " " + port
                        }
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 13
                        color: InkTokens.ink700
                    }
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: (typeof study !== "undefined" && study.streamOpen)
                              ? qsTr("事件流：已接通") : qsTr("事件流：未接通")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 13
                        color: InkTokens.ink700
                    }
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: qsTr("工作区")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 12
                        color: InkTokens.ink500
                    }
                    Text {
                        width: column.width - 36
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        wrapMode: Text.Wrap
                        text: (typeof study !== "undefined") ? study.workspaceTitle : qsTr("未入席")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 14
                        color: InkTokens.ink900
                    }
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: qsTr("会话")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 12
                        color: InkTokens.ink500
                    }
                    Text {
                        width: column.width - 36
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        wrapMode: Text.Wrap
                        text: {
                            if (typeof study === "undefined" || study.selectedTitle.length === 0)
                                return qsTr("尚未选择会话")
                            return study.selectedTitle
                        }
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 14
                        color: InkTokens.ink900
                    }
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: qsTr("模型")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 12
                        color: InkTokens.ink500
                    }
                    Text {
                        width: column.width - 36
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        wrapMode: Text.Wrap
                        text: (typeof study !== "undefined") ? study.modelLabel : qsTr("模型")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 14
                        color: InkTokens.ink900
                    }
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: {
                            var n = (typeof study !== "undefined") ? study.modelOptions.length : 0
                            return qsTr("可选模型") + " " + n + " " + qsTr("个")
                        }
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 13
                        color: InkTokens.ink700
                    }
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: qsTr("工具")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 12
                        color: InkTokens.ink500
                    }
                    Text {
                        width: column.width - 36
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        wrapMode: Text.Wrap
                        text: qsTr("对话中的工具调用会显示为帖；权限确认在卷轴下沿。")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 13
                        color: InkTokens.ink700
                    }
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: qsTr("配置")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 12
                        color: InkTokens.ink500
                    }
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: (typeof study !== "undefined" && study.settingsWritable)
                              ? qsTr("配置可写") : qsTr("配置只读")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 13
                        color: InkTokens.ink700
                    }
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: (typeof study !== "undefined" && study.settingsHasDocument)
                              ? qsTr("已有配置文书") : qsTr("尚无配置文书")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 13
                        color: InkTokens.ink700
                    }
                    Repeater {
                        model: (typeof study !== "undefined") ? study.settingsNamespaces : []
                        Text {
                            required property var modelData
                            width: column.width - 36
                            anchors.left: parent.left
                            anchors.leftMargin: 20
                            wrapMode: Text.Wrap
                            text: (modelData.title ? modelData.title : modelData.ns) + " · " + (modelData.applies === "restart" ? qsTr("须重启") : qsTr("即时生效"))
                            font.family: InkTokens.calligraphyFamily
                            font.pixelSize: 13
                            color: InkTokens.ink700
                        }
                    }
                    Text {
                        visible: typeof study !== "undefined" && study.settingsNamespaces.length === 0
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: qsTr("尚无配置命名空间")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 13
                        color: InkTokens.ink300
                    }
                }

                Repeater {
                    model: (typeof study !== "undefined") ? study.settingsFields : []
                    Column {
                        id: fieldCol
                        required property var modelData
                        readonly property string fieldNs: modelData.ns
                        readonly property string fieldKey: modelData.key
                        readonly property string fieldKind: modelData.kind
                        readonly property var fieldValue: modelData.value
                        visible: typeof study !== "undefined" && (study.settingsTab === "general" || study.settingsTab === "models" || study.settingsTab === "permission") && modelData.section === study.settingsTab
                        width: column.width
                        spacing: 4

                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 20
                            text: modelData.nsTitle + " · " + modelData.title
                            font.family: InkTokens.calligraphyFamily
                            font.pixelSize: 12
                            color: InkTokens.ink500
                        }

                        Text {
                            visible: fieldCol.fieldKind === "bool"
                            anchors.left: parent.left
                            anchors.leftMargin: 20
                            text: fieldCol.fieldValue ? qsTr("开 · 点此关闭") : qsTr("关 · 点此打开")
                            font.family: InkTokens.calligraphyFamily
                            font.pixelSize: 14
                            color: InkTokens.cinnabar
                            HoverHandler {
                                cursorShape: Qt.PointingHandCursor
                            }
                            TapHandler {
                                enabled: modelData.writable
                                acceptedButtons: Qt.LeftButton
                                onTapped: study.updateSetting(fieldCol.fieldNs, fieldCol.fieldKey, "bool", !fieldCol.fieldValue)
                            }
                        }

                        Flow {
                            visible: fieldCol.fieldKind === "enum"
                            width: parent.width - 36
                            anchors.left: parent.left
                            anchors.leftMargin: 20
                            spacing: 10
                            Repeater {
                                model: fieldCol.modelData.choices
                                Text {
                                    required property var modelData
                                    text: modelData.label
                                    font.family: InkTokens.calligraphyFamily
                                    font.pixelSize: 14
                                    color: modelData.id === fieldCol.fieldValue ? InkTokens.cinnabar : InkTokens.ink700
                                    HoverHandler {
                                        cursorShape: Qt.PointingHandCursor
                                    }
                                    TapHandler {
                                        acceptedButtons: Qt.LeftButton
                                        onTapped: study.updateSetting(fieldCol.fieldNs, fieldCol.fieldKey, "enum", modelData.id)
                                    }
                                }
                            }
                        }

                        Row {
                            visible: fieldCol.fieldKind === "string" || fieldCol.fieldKind === "number" || fieldCol.fieldKind === "secret"
                            anchors.left: parent.left
                            anchors.leftMargin: 20
                            spacing: 10
                            TextField {
                                id: edit
                                width: 220
                                text: fieldCol.fieldKind === "secret" ? "" : String(fieldCol.fieldValue)
                                echoMode: fieldCol.fieldKind === "secret" ? TextInput.Password : TextInput.Normal
                                placeholderText: fieldCol.fieldKind === "secret"
                                                 ? (fieldCol.modelData.secretSet ? qsTr("已保存 · 留空则保持") : qsTr("在此写入…"))
                                                 : ""
                                font.family: InkTokens.calligraphyFamily
                                font.pixelSize: 13
                                color: InkTokens.ink900
                                enabled: fieldCol.modelData.writable
                                background: Rectangle {
                                    color: "transparent"
                                    border.width: 1
                                    border.color: InkTokens.hairline
                                }
                            }
                            Text {
                                text: qsTr("写入")
                                font.family: InkTokens.calligraphyFamily
                                font.pixelSize: 14
                                color: InkTokens.cinnabar
                                anchors.verticalCenter: parent.verticalCenter
                                HoverHandler {
                                    cursorShape: Qt.PointingHandCursor
                                }
                                TapHandler {
                                    enabled: fieldCol.modelData.writable && edit.text.length > 0
                                    acceptedButtons: Qt.LeftButton
                                    onTapped: study.updateSetting(fieldCol.fieldNs, fieldCol.fieldKey, fieldCol.fieldKind, edit.text)
                                }
                            }
                        }
                    }
                }

                Column {
                    visible: typeof study !== "undefined" && study.settingsTab === "models"
                    width: parent.width
                    spacing: 8

                    Repeater {
                        model: (typeof study !== "undefined") ? study.providerRows : []
                        Column {
                            required property var modelData
                            width: column.width
                            spacing: 4
                            Text {
                                anchors.left: parent.left
                                anchors.leftMargin: 20
                                text: modelData.title + " · " + (modelData.active ? qsTr("在册") : qsTr("未激活")) + " · " + (modelData.configured ? qsTr("密钥已备") : qsTr("密钥未备"))
                                font.family: InkTokens.calligraphyFamily
                                font.pixelSize: 13
                                color: InkTokens.ink900
                            }
                            Row {
                                visible: modelData.apiKeyEnv && modelData.apiKeyEnv.length > 0
                                anchors.left: parent.left
                                anchors.leftMargin: 20
                                spacing: 10
                                TextField {
                                    id: providerKey
                                    width: 220
                                    echoMode: TextInput.Password
                                    placeholderText: qsTr("写入密钥…")
                                    font.family: InkTokens.calligraphyFamily
                                    font.pixelSize: 13
                                    color: InkTokens.ink900
                                    enabled: modelData.credentialWritable
                                    background: Rectangle {
                                        color: "transparent"
                                        border.width: 1
                                        border.color: InkTokens.hairline
                                    }
                                }
                                Text {
                                    text: qsTr("保存密钥")
                                    font.family: InkTokens.calligraphyFamily
                                    font.pixelSize: 14
                                    color: InkTokens.cinnabar
                                    anchors.verticalCenter: parent.verticalCenter
                                    HoverHandler {
                                        cursorShape: Qt.PointingHandCursor
                                    }
                                    TapHandler {
                                        enabled: providerKey.text.length > 0
                                        acceptedButtons: Qt.LeftButton
                                        onTapped: study.setCredential(modelData.apiKeyEnv, providerKey.text)
                                    }
                                }
                            }
                        }
                    }
                    Text {
                        visible: typeof study !== "undefined" && study.providerRows.length === 0
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: qsTr("尚无模型供应方")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 13
                        color: InkTokens.ink300
                    }
                }

                Column {
                    visible: typeof study !== "undefined" && study.settingsTab === "permission"
                    width: parent.width
                    spacing: 8
                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: qsTr("本会话")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 12
                        color: InkTokens.ink500
                    }
                    Repeater {
                        model: (typeof study !== "undefined") ? study.permissionOptions : []
                        Text {
                            required property var modelData
                            anchors.left: parent.left
                            anchors.leftMargin: 20
                            text: (modelData.current ? "· " : "") + modelData.label
                            font.family: InkTokens.calligraphyFamily
                            font.pixelSize: 14
                            color: modelData.current ? InkTokens.cinnabar : InkTokens.ink700
                            HoverHandler {
                                cursorShape: Qt.PointingHandCursor
                            }
                            TapHandler {
                                acceptedButtons: Qt.LeftButton
                                onTapped: study.selectPermission(modelData.id)
                            }
                        }
                    }
                    Text {
                        visible: typeof study !== "undefined" && study.permissionOptions.length === 0
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: qsTr("宿主未提供权限目录")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 13
                        color: InkTokens.ink300
                    }
                }

                Column {
                    visible: typeof study !== "undefined" && study.settingsTab === "skills"
                    width: parent.width
                    spacing: 8
                    Repeater {
                        model: (typeof study !== "undefined") ? study.skills : []
                        Text {
                            required property var modelData
                            width: column.width - 36
                            anchors.left: parent.left
                            anchors.leftMargin: 20
                            wrapMode: Text.Wrap
                            text: "/" + modelData.name + " · " + (modelData.description ? modelData.description : qsTr("技能"))
                            font.family: InkTokens.calligraphyFamily
                            font.pixelSize: 13
                            color: InkTokens.ink700
                        }
                    }
                    Text {
                        visible: typeof study !== "undefined" && study.skills.length === 0
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: qsTr("本会话尚无技能")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 13
                        color: InkTokens.ink300
                    }
                }

                Column {
                    visible: typeof study !== "undefined" && study.settingsTab === "subagents"
                    width: parent.width
                    spacing: 8
                    Repeater {
                        model: (typeof study !== "undefined") ? study.subagents : []
                        Row {
                            required property var modelData
                            anchors.left: parent.left
                            anchors.leftMargin: 20
                            spacing: 12
                            Text {
                                text: modelData.label + " · " + (modelData.running ? qsTr("进行中") : qsTr("闲"))
                                font.family: InkTokens.calligraphyFamily
                                font.pixelSize: 13
                                color: InkTokens.ink700
                            }
                            Text {
                                visible: modelData.continuable && modelData.running
                                text: qsTr("打断")
                                font.family: InkTokens.calligraphyFamily
                                font.pixelSize: 13
                                color: InkTokens.cinnabar
                                HoverHandler {
                                    cursorShape: Qt.PointingHandCursor
                                }
                                TapHandler {
                                    acceptedButtons: Qt.LeftButton
                                    onTapped: study.interruptSubagent(modelData.id)
                                }
                            }
                        }
                    }
                    Text {
                        visible: typeof study !== "undefined" && study.subagents.length === 0
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: qsTr("本会话尚无子代理")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 13
                        color: InkTokens.ink300
                    }
                }

                Column {
                    visible: typeof study !== "undefined" && study.settingsTab === "presets"
                    width: parent.width
                    spacing: 8
                    Repeater {
                        model: (typeof study !== "undefined") ? study.agentPresets : []
                        Text {
                            required property var modelData
                            width: column.width - 36
                            anchors.left: parent.left
                            anchors.leftMargin: 20
                            wrapMode: Text.Wrap
                            text: (modelData.isDefault ? qsTr("默认 · ") : "") + modelData.name + " · " + modelData.trust
                            font.family: InkTokens.calligraphyFamily
                            font.pixelSize: 14
                            color: InkTokens.ink700
                            HoverHandler {
                                cursorShape: Qt.PointingHandCursor
                            }
                            TapHandler {
                                acceptedButtons: Qt.LeftButton
                                onTapped: study.selectPreset(modelData.id)
                            }
                        }
                    }
                    Text {
                        visible: typeof study !== "undefined" && study.agentPresets.length === 0
                        anchors.left: parent.left
                        anchors.leftMargin: 20
                        text: qsTr("尚无智能体预设")
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 13
                        color: InkTokens.ink300
                    }
                }
            }
        }

        Row {
            id: actions
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 16
            spacing: 18

            Text {
                text: qsTr("刷新")
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 14
                color: InkTokens.cinnabar
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
                TapHandler {
                    margin: 8
                    acceptedButtons: Qt.LeftButton
                    onTapped: {
                        if (typeof study !== "undefined") {
                            study.refresh()
                            study.openSettings()
                        }
                    }
                }
            }

            Text {
                text: qsTr("打开配置文书")
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 14
                color: InkTokens.cinnabar
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
                TapHandler {
                    margin: 8
                    acceptedButtons: Qt.LeftButton
                    onTapped: study.openSettingsDocument()
                }
            }
        }
    }
}
