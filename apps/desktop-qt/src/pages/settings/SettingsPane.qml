import QtQuick

Item {
    id: root
    visible: typeof study !== "undefined" && study.settingsOpen
    enabled: visible
    anchors.fill: parent
    z: 20

    function appliesLabel(applies) {
        if (applies === "restart")
            return qsTr("须重启")
        if (applies === "hot")
            return qsTr("即时生效")
        return applies && applies.length > 0 ? applies : qsTr("即时生效")
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
        width: Math.min(460, parent.width - 48)
        height: Math.min(640, parent.height - 72)
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

        Flickable {
            id: body
            anchors.top: heading.bottom
            anchors.topMargin: 12
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
                    anchors.topMargin: 6
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
                        text: modelData.ns + " · " + root.appliesLabel(modelData.applies)
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
