import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Item {
    id: root
    height: composerBase.height

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
        if (typeof study === "undefined")
            return
        var shouldClear = root.canSend
        study.sendPrompt(input.text)
        if (shouldClear)
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

    function closeOverlays() {
        if (typeof study === "undefined")
            return
        if (study.modelsOpen)
            study.toggleModels()
        if (study.permissionsOpen)
            study.togglePermissions()
    }

    Item {
        id: composerBase
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 96 + (hasAttachments ? 24 : 0)

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: InkTokens.hairline
            opacity: 0.55
        }

        Row {
            id: chips
            anchors.left: parent.left
            anchors.leftMargin: InkTokens.rhythm * 3
            anchors.top: parent.top
            anchors.topMargin: InkTokens.rhythm
            spacing: InkTokens.rhythm + 4

            Repeater {
                model: {
                    var items = []
                    if (typeof study === "undefined")
                        return items
                    if (study.planKnown)
                        items.push({ text: study.planActive ? qsTr("计划中") : qsTr("计划"), action: "plan", active: study.planActive })
                    items.push({ text: study.permissionLabel, action: "perm", active: false })
                    items.push({ text: study.modelLabel, action: "model", active: study.modelsOpen })
                    items.push({ text: qsTr("附页"), action: "attach", active: root.hasAttachments })
                    return items
                }
                delegate: Text {
                    required property var modelData
                    text: modelData.text
                    font.family: InkTokens.calligraphyFamily
                    font.pixelSize: 11
                    color: modelData.active ? InkTokens.cinnabar : InkTokens.ink500
                    opacity: chipHover.hovered ? 0.72 : 1
                    HoverHandler {
                        id: chipHover
                        cursorShape: Qt.PointingHandCursor
                    }
                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: -4
                        acceptedButtons: Qt.LeftButton
                        preventStealing: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (typeof study === "undefined")
                                return
                            if (modelData.action === "plan")
                                study.togglePlan()
                            else if (modelData.action === "perm")
                                study.togglePermissions()
                            else if (modelData.action === "model")
                                study.toggleModels()
                            else if (modelData.action === "attach")
                                picker.open()
                        }
                    }
                    Behavior on opacity {
                        enabled: MotionBudget.hoverMs > 0
                        NumberAnimation { duration: MotionBudget.hoverMs }
                    }
                }
            }
        }

        Row {
            id: attachRow
            visible: root.hasAttachments
            anchors.left: parent.left
            anchors.leftMargin: InkTokens.rhythm * 3
            anchors.top: chips.bottom
            anchors.topMargin: 4
            spacing: InkTokens.rhythm
            Repeater {
                model: (typeof study !== "undefined") ? study.attachments : []
                Text {
                    required property int index
                    required property var modelData
                    text: (modelData.name ? modelData.name : qsTr("图")) + " ×"
                    font.family: InkTokens.calligraphyFamily
                    font.pixelSize: 11
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
            anchors.top: attachRow.visible ? attachRow.bottom : chips.bottom
            anchors.bottom: parent.bottom
            anchors.leftMargin: InkTokens.rhythm * 2 + 4
            anchors.rightMargin: InkTokens.rhythm * 2
            anchors.topMargin: 4
            anchors.bottomMargin: InkTokens.rhythm + 4
            wrapMode: TextEdit.Wrap
            placeholderText: qsTr("在此输入…")
            font.family: InkTokens.bodyFamily
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
            width: 48
            height: 40
            radius: 2
            rotation: 8
            anchors.right: parent.right
            anchors.rightMargin: InkTokens.rhythm * 2 + 2
            anchors.bottom: parent.bottom
            anchors.bottomMargin: InkTokens.rhythm * 2
            color: {
                if (typeof study !== "undefined" && study.sending)
                    return InkTokens.ink700
                return root.canSend ? InkTokens.cinnabar : Qt.rgba(0.651, 0.239, 0.184, 0.28)
            }
            opacity: sendHover.hovered ? 0.85 : 1

            Text {
                anchors.centerIn: parent
                rotation: -8
                text: (typeof study !== "undefined" && study.sending) ? qsTr("停止") : qsTr("发送")
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 13
                color: InkTokens.ink0
            }

            HoverHandler {
                id: sendHover
                cursorShape: Qt.PointingHandCursor
            }
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                preventStealing: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (typeof study !== "undefined" && study.sending) {
                        study.cancelTurn()
                        return
                    }
                    root.submit()
                }
                onPressedChanged: sendSeal.opacity = pressed ? 0.65 : (sendHover.hovered ? 0.85 : 1)
            }
            Behavior on opacity {
                enabled: MotionBudget.pressMs > 0
                NumberAnimation { duration: MotionBudget.pressMs }
            }
        }
    }

    Popup {
        id: overlayMenu
        modal: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: InkTokens.rhythm
        width: Math.min(360, root.width - InkTokens.rhythm * 6)
        x: InkTokens.rhythm * 3
        y: -implicitHeight - InkTokens.rhythm
        visible: root.modelsOpen || root.permissionsOpen || root.slashOpen

        onClosed: root.closeOverlays()

        background: Rectangle {
            color: InkTokens.scrollPaper
            border.width: 1
            border.color: InkTokens.hairline
            radius: 1
        }

        contentItem: ListView {
            id: overlayList
            implicitHeight: Math.min(180, contentHeight)
            width: overlayMenu.availableWidth
            clip: true
            reuseItems: true
            cacheBuffer: 80
            boundsBehavior: Flickable.StopAtBounds
            model: root.slashOpen ? root.filteredSlash()
                  : (root.permissionsOpen ? ((typeof study !== "undefined") ? study.permissionOptions : [])
                     : ((typeof study !== "undefined") ? study.modelOptions : []))
            delegate: Item {
                required property var modelData
                width: overlayList.width
                height: 28
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.rightMargin: InkTokens.rhythm
                    elide: Text.ElideRight
                    font.family: InkTokens.calligraphyFamily
                    font.pixelSize: 12
                    color: InkTokens.ink700
                    text: {
                        if (root.slashOpen)
                            return modelData.title + (modelData.detail ? " · " + modelData.detail : "")
                        if (root.permissionsOpen)
                            return (modelData.current ? "· " : "") + modelData.label
                        return modelData.group + " · " + modelData.name
                    }
                }
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    preventStealing: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (typeof study === "undefined")
                            return
                        if (root.slashOpen) {
                            if (modelData.kind === "skill")
                                input.text = modelData.line
                            else
                                study.pickSlash(modelData.line)
                            overlayMenu.close()
                            return
                        }
                        if (root.permissionsOpen) {
                            study.selectPermission(modelData.id)
                            return
                        }
                        study.selectModel(modelData.provider, modelData.model)
                    }
                }
            }

            Text {
                visible: overlayList.count === 0
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                text: root.modelsOpen ? qsTr("暂无模型") : qsTr("暂无选项")
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 12
                color: InkTokens.ink300
            }
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
