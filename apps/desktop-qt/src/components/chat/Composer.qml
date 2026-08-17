import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Item {
    id: root
    height: 108
            + (modelsOpen ? Math.min(160, modelList.contentHeight + 8) : 0)
            + (permissionsOpen ? Math.min(120, permissionList.contentHeight + 8) : 0)
            + (slashOpen ? Math.min(140, slashList.contentHeight + 8) : 0)
            + (hasAttachments ? 28 : 0)

    readonly property bool hasAttachments: typeof study !== "undefined" && study.attachments.length > 0
    readonly property bool canSend: typeof connection !== "undefined"
                                    && connection.connected
                                    && typeof study !== "undefined"
                                    && study.selectedSessionId.length > 0
                                    && !study.sending
                                    && (input.text.trim().length > 0 || root.hasAttachments)
    readonly property bool modelsOpen: typeof study !== "undefined" && study.modelsOpen
    readonly property bool permissionsOpen: typeof study !== "undefined" && study.permissionsOpen
    readonly property bool slashOpen: typeof study !== "undefined" && input.text.trim().startsWith("/")

    function submit() {
        if (!root.canSend)
            return
        study.sendPrompt(input.text)
        input.text = ""
    }

    function filteredSlash() {
        if (typeof study === "undefined")
            return []
        var needle = input.text.trim()
        var source = study.slashItems
        var out = []
        for (var i = 0; i < source.length; ++i) {
            var item = source[i]
            if (!needle || String(item.line).indexOf(needle) === 0 || String(item.title).indexOf(needle.substring(1)) >= 0)
                out.push(item)
        }
        return out
    }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: InkTokens.hairline
        opacity: 0.65
    }

    Row {
        id: chips
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.top: parent.top
        anchors.topMargin: 8
        spacing: 14

        Text {
            visible: typeof study !== "undefined" && study.planKnown
            text: (typeof study !== "undefined" && study.planActive) ? qsTr("计划中") : qsTr("计划")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 12
            color: (typeof study !== "undefined" && study.planActive) ? InkTokens.cinnabar : InkTokens.ink500
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                margin: 6
                acceptedButtons: Qt.LeftButton
                onTapped: study.togglePlan()
            }
        }

        Text {
            objectName: "composerPermission"
            text: (typeof study !== "undefined") ? study.permissionLabel : qsTr("权限")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 12
            color: InkTokens.ink500
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                margin: 6
                acceptedButtons: Qt.LeftButton
                onTapped: {
                    if (typeof study !== "undefined")
                        study.togglePermissions()
                }
            }
        }

        Text {
            objectName: "composerModel"
            text: (typeof study !== "undefined") ? study.modelLabel : qsTr("模型")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 12
            color: InkTokens.ink500
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                margin: 6
                acceptedButtons: Qt.LeftButton
                onTapped: {
                    if (typeof study !== "undefined")
                        study.toggleModels()
                }
            }
        }

        Text {
            text: qsTr("附页")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 12
            color: InkTokens.ink500
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                margin: 6
                acceptedButtons: Qt.LeftButton
                onTapped: picker.open()
            }
        }
    }

    ListView {
        id: permissionList
        visible: root.permissionsOpen
        anchors.left: parent.left
        anchors.right: sendSeal.left
        anchors.top: chips.bottom
        anchors.topMargin: 4
        height: visible ? Math.min(112, contentHeight) : 0
        clip: true
        reuseItems: true
        cacheBuffer: 80
        boundsBehavior: Flickable.StopAtBounds
        model: (typeof study !== "undefined") ? study.permissionOptions : []
        delegate: Item {
            required property var modelData
            width: permissionList.width
            height: 26
            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 24
                text: (modelData.current ? "· " : "") + modelData.label
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 12
                color: modelData.current ? InkTokens.cinnabar : InkTokens.ink700
            }
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: study.selectPermission(modelData.id)
            }
        }
    }

    ListView {
        id: modelList
        visible: root.modelsOpen
        anchors.left: parent.left
        anchors.right: sendSeal.left
        anchors.top: permissionList.visible ? permissionList.bottom : chips.bottom
        anchors.topMargin: 4
        height: visible ? Math.min(148, contentHeight) : 0
        clip: true
        reuseItems: true
        cacheBuffer: 80
        boundsBehavior: Flickable.StopAtBounds
        model: (typeof study !== "undefined") ? study.modelOptions : []
        delegate: Item {
            required property var modelData
            width: modelList.width
            height: 26
            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 24
                text: modelData.group + " · " + modelData.name
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 12
                color: InkTokens.ink700
            }
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: study.selectModel(modelData.provider, modelData.model)
            }
        }

        Text {
            visible: modelList.count === 0
            anchors.left: parent.left
            anchors.leftMargin: 24
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("暂无模型")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 12
            color: InkTokens.ink300
        }
    }

    ListView {
        id: slashList
        visible: root.slashOpen
        anchors.left: parent.left
        anchors.right: sendSeal.left
        anchors.top: modelList.visible ? modelList.bottom : (permissionList.visible ? permissionList.bottom : chips.bottom)
        anchors.topMargin: 4
        height: visible ? Math.min(132, contentHeight) : 0
        clip: true
        reuseItems: true
        cacheBuffer: 80
        boundsBehavior: Flickable.StopAtBounds
        model: root.filteredSlash()
        delegate: Item {
            required property var modelData
            width: slashList.width
            height: 26
            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 24
                anchors.right: parent.right
                anchors.rightMargin: 12
                elide: Text.ElideRight
                text: modelData.title + (modelData.detail ? " · " + modelData.detail : "")
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 12
                color: InkTokens.ink700
            }
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: {
                    if (modelData.kind === "skill")
                        input.text = modelData.line
                    else
                        study.pickSlash(modelData.line)
                }
            }
        }
    }

    Row {
        id: attachRow
        visible: root.hasAttachments
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.top: slashList.visible ? slashList.bottom : (modelList.visible ? modelList.bottom : (permissionList.visible ? permissionList.bottom : chips.bottom))
        anchors.topMargin: 4
        spacing: 10
        Repeater {
            model: (typeof study !== "undefined") ? study.attachments : []
            Text {
                required property int index
                required property var modelData
                text: (modelData.name ? modelData.name : qsTr("图")) + " ×"
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 12
                color: InkTokens.ink500
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
                TapHandler {
                    acceptedButtons: Qt.LeftButton
                    onTapped: study.removeAttachment(index)
                }
            }
        }
    }

    TextArea {
        id: input
        objectName: "composerInput"
        anchors.left: parent.left
        anchors.right: sendSeal.left
        anchors.top: attachRow.visible ? attachRow.bottom : (slashList.visible ? slashList.bottom : (modelList.visible ? modelList.bottom : (permissionList.visible ? permissionList.bottom : chips.bottom)))
        anchors.bottom: parent.bottom
        anchors.leftMargin: 20
        anchors.rightMargin: 16
        anchors.topMargin: 4
        anchors.bottomMargin: 12
        wrapMode: TextEdit.Wrap
        placeholderText: qsTr("在此输入…")
        font.family: InkTokens.calligraphyFamily
        font.pixelSize: 15
        color: InkTokens.ink900
        placeholderTextColor: InkTokens.ink300
        enabled: typeof connection !== "undefined" && connection.connected
                 && typeof study !== "undefined" && study.selectedSessionId.length > 0
        background: Item {}
        Keys.onPressed: function (event) {
            if ((event.modifiers & Qt.ControlModifier)
                    && (event.key === Qt.Key_Return || event.key === Qt.Key_Enter)) {
                root.submit()
                event.accepted = true
            }
        }
    }

    Rectangle {
        id: sendSeal
        objectName: "composerSend"
        z: 2
        width: 52
        height: 44
        radius: 2
        rotation: 8
        anchors.right: parent.right
        anchors.rightMargin: 18
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 22
        color: {
            if (typeof study !== "undefined" && study.sending)
                return InkTokens.ink700
            return root.canSend ? InkTokens.cinnabar : Qt.rgba(0.651, 0.239, 0.184, 0.28)
        }
        opacity: 1

        Text {
            anchors.centerIn: parent
            rotation: -8
            text: (typeof study !== "undefined" && study.sending) ? qsTr("停止") : qsTr("发送")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 14
            color: InkTokens.ink0
        }

        HoverHandler {
            cursorShape: Qt.PointingHandCursor
        }
        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: {
                if (typeof study !== "undefined" && study.sending) {
                    study.cancelTurn()
                    return
                }
                root.submit()
            }
            onPressedChanged: sendSeal.opacity = pressed ? 0.7 : 1
        }
        Behavior on opacity {
            enabled: MotionBudget.pressMs > 0
            NumberAnimation { duration: MotionBudget.pressMs }
        }
    }

    FileDialog {
        id: picker
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("图像 (*.png *.jpg *.jpeg *.webp *.gif)")]
        onAccepted: {
            var files = selectedFiles
            for (var i = 0; i < files.length; ++i)
                study.attachFromUrl(files[i])
        }
    }
}
