import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Item {
    id: root
    width: InkTokens.sidebarWidth

    Connections {
        target: typeof study !== "undefined" ? study : null
        function onWorkspacePickFallbackRequested() {
            folderPicker.open()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: InkTokens.sidebarWash
        opacity: 0.55
        enabled: false
    }

    Column {
        id: head
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: InkTokens.rhythm * 2
        anchors.leftMargin: InkTokens.rhythm * 2 + 2
        anchors.rightMargin: InkTokens.rhythm * 2 + 2
        spacing: 2

        Text {
            text: qsTr("工作区")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 11
            color: InkTokens.ink500
        }

        Row {
            width: parent.width
            spacing: 6

            Text {
                id: workspaceName
                objectName: "sidebarWorkspaceTitle"
                width: parent.width - chevron.width - 6
                text: (typeof study !== "undefined") ? study.workspaceTitle : qsTr("未入席")
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 16
                color: InkTokens.primaryText
                elide: Text.ElideMiddle
                opacity: workspaceHover.hovered ? 0.72 : 1
                Behavior on opacity {
                    enabled: MotionBudget.hoverMs > 0
                    NumberAnimation { duration: MotionBudget.hoverMs }
                }
            }

            Text {
                id: chevron
                text: (typeof study !== "undefined" && study.workspacePickerOpen) ? "▾" : "▸"
                font.pixelSize: 11
                color: InkTokens.ink500
                anchors.verticalCenter: workspaceName.verticalCenter
            }
        }

        HoverHandler {
            id: workspaceHover
            cursorShape: Qt.PointingHandCursor
        }
        MouseArea {
            anchors.fill: head
            anchors.topMargin: 14
            anchors.bottomMargin: -2
            acceptedButtons: Qt.LeftButton
            preventStealing: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (typeof study !== "undefined")
                    study.toggleWorkspacePicker()
            }
        }
    }

    Row {
        id: featureChips
        anchors.top: head.bottom
        anchors.topMargin: InkTokens.rhythm
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: InkTokens.rhythm * 2 + 2
        anchors.rightMargin: InkTokens.rhythm * 2 + 2
        spacing: 6
        clip: true

        Repeater {
            model: {
                var chips = []
                if (typeof study === "undefined")
                    return chips
                if (study.modelLabel.length > 0 && study.modelLabel !== qsTr("模型"))
                    chips.push({ label: study.modelLabel, kind: "model" })
                else if (study.selectedSessionId.length > 0)
                    chips.push({ label: study.modelLabel, kind: "model" })
                if (study.planKnown)
                    chips.push({ label: study.planActive ? qsTr("计划") : qsTr("计划"), kind: "plan", active: study.planActive })
                if (study.permissionLabel.length > 0)
                    chips.push({ label: study.permissionLabel, kind: "perm" })
                if (study.attachments.length > 0)
                    chips.push({ label: qsTr("附页") + " " + study.attachments.length, kind: "attach" })
                return chips
            }
            delegate: Rectangle {
                required property var modelData
                height: 20
                width: chipLabel.implicitWidth + 12
                radius: 1
                color: Qt.rgba(0.651, 0.239, 0.184, modelData.active ? 0.12 : 0.05)
                border.width: modelData.active ? 1 : 0
                border.color: Qt.rgba(0.651, 0.239, 0.184, 0.35)

                Text {
                    id: chipLabel
                    anchors.centerIn: parent
                    text: modelData.label
                    font.family: InkTokens.calligraphyFamily
                    font.pixelSize: 10
                    color: modelData.active ? InkTokens.cinnabar : InkTokens.ink700
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
                        if (modelData.kind === "model")
                            study.toggleModels()
                        else if (modelData.kind === "plan")
                            study.togglePlan()
                        else if (modelData.kind === "perm")
                            study.togglePermissions()
                        else if (modelData.kind === "attach")
                            attachPicker.open()
                    }
                }
            }
        }
    }

    Text {
        id: sessionHeading
        anchors.top: featureChips.bottom
        anchors.left: parent.left
        anchors.leftMargin: InkTokens.rhythm * 2 + 2
        anchors.topMargin: InkTokens.rhythm + 4
        text: qsTr("会话")
        font.family: InkTokens.calligraphyFamily
        font.pixelSize: 11
        color: InkTokens.ink500
    }

    ListView {
        id: sessionList
        anchors.top: sessionHeading.bottom
        anchors.topMargin: InkTokens.rhythm
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: newSessionBtn.top
        anchors.bottomMargin: InkTokens.rhythm
        clip: true
        reuseItems: true
        cacheBuffer: 280
        pixelAligned: true
        boundsBehavior: Flickable.StopAtBounds
        flickDeceleration: 2500
        spacing: 2
        model: (typeof study !== "undefined") ? study.sessions : []

        delegate: Item {
            id: rowRoot
            required property string sessionId
            required property string title
            required property bool running
            width: sessionList.width
            height: 36

            readonly property bool selected: typeof study !== "undefined"
                                             && study.selectedSessionId === sessionId

            Rectangle {
                id: wash
                anchors.fill: parent
                anchors.leftMargin: InkTokens.rhythm + 2
                anchors.rightMargin: InkTokens.rhythm + 2
                color: rowRoot.selected ? Qt.rgba(0.651, 0.239, 0.184, 0.10) : "transparent"
                border.width: rowRoot.selected ? 1 : 0
                border.color: Qt.rgba(0.651, 0.239, 0.184, 0.45)
                radius: 1
                opacity: rowHover.hovered && !rowRoot.selected ? 0.55 : 1
                Behavior on opacity {
                    enabled: MotionBudget.hoverMs > 0
                    NumberAnimation { duration: MotionBudget.hoverMs }
                }
            }

            Rectangle {
                visible: rowRoot.selected
                width: 3
                height: 16
                radius: 1
                color: InkTokens.cinnabar
                anchors.left: parent.left
                anchors.leftMargin: InkTokens.rhythm + 6
                anchors.verticalCenter: parent.verticalCenter
            }

            Rectangle {
                visible: running
                width: 5
                height: 5
                radius: 1
                rotation: 12
                color: InkTokens.ink900
                anchors.right: parent.right
                anchors.rightMargin: InkTokens.rhythm * 2
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: InkTokens.rhythm * 3
                anchors.right: parent.right
                anchors.rightMargin: InkTokens.rhythm * 2 + 6
                text: title
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 13
                color: rowRoot.selected ? InkTokens.ink900 : InkTokens.ink700
                elide: Text.ElideRight
            }

            HoverHandler {
                id: rowHover
                cursorShape: Qt.PointingHandCursor
            }
            MouseArea {
                objectName: "sidebarSessionRow"
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                preventStealing: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (typeof study !== "undefined")
                        study.selectSession(sessionId)
                }
            }
        }

        Text {
            visible: sessionList.count === 0
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: InkTokens.rhythm * 3
            text: qsTr("尚无会话")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 12
            color: InkTokens.ink300
        }
    }

    Text {
        id: newSessionBtn
        objectName: "sidebarNewSession"
        anchors.left: parent.left
        anchors.leftMargin: InkTokens.rhythm * 2 + 2
        anchors.bottom: parent.bottom
        anchors.bottomMargin: InkTokens.rhythm * 2
        text: qsTr("新会话")
        font.family: InkTokens.calligraphyFamily
        font.pixelSize: 13
        color: InkTokens.cinnabar
        opacity: newHover.hovered ? 0.72 : 1

        HoverHandler {
            id: newHover
            cursorShape: Qt.PointingHandCursor
        }
        MouseArea {
            anchors.fill: parent
            anchors.margins: -8
            acceptedButtons: Qt.LeftButton
            preventStealing: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (typeof study !== "undefined")
                    study.createSession()
            }
            onPressedChanged: newSessionBtn.opacity = pressed ? 0.55 : (newHover.hovered ? 0.72 : 1)
        }
        Behavior on opacity {
            enabled: MotionBudget.pressMs > 0
            NumberAnimation { duration: MotionBudget.pressMs }
        }
    }

    Popup {
        id: workspacePicker
        modal: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 0
        width: root.width - InkTokens.rhythm * 2
        x: InkTokens.rhythm
        y: head.y + head.height + 2
        visible: typeof study !== "undefined" && study.workspacePickerOpen

        onClosed: {
            if (typeof study !== "undefined" && study.workspacePickerOpen)
                study.closeWorkspacePicker()
        }

        background: Rectangle {
            color: InkTokens.scrollPaper
            border.width: 1
            border.color: InkTokens.hairline
            radius: 1
        }

        contentItem: Column {
            spacing: 0
            width: workspacePicker.width

            ListView {
                id: workspaceList
                width: parent.width
                height: Math.min(168, Math.max(28, count * 32 + (count > 0 ? 0 : 28)))
                clip: true
                reuseItems: true
                cacheBuffer: 80
                boundsBehavior: Flickable.StopAtBounds
                model: (typeof study !== "undefined") ? study.workspaces : []
                delegate: Item {
                    required property var modelData
                    width: workspaceList.width
                    height: 32
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: InkTokens.rhythm * 2
                        anchors.right: parent.right
                        anchors.rightMargin: InkTokens.rhythm * 2
                        text: modelData.title
                        font.family: InkTokens.calligraphyFamily
                        font.pixelSize: 13
                        color: (typeof study !== "undefined" && study.workspaceId === modelData.workspaceId)
                               ? InkTokens.cinnabar : InkTokens.ink700
                        elide: Text.ElideRight
                        opacity: rowHover.hovered ? 0.72 : 1
                    }
                    HoverHandler {
                        id: rowHover
                        cursorShape: Qt.PointingHandCursor
                    }
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton
                        preventStealing: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: study.selectWorkspace(modelData.workspaceId)
                    }
                }

                Text {
                    visible: workspaceList.count === 0
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("尚无工作区")
                    font.family: InkTokens.calligraphyFamily
                    font.pixelSize: 12
                    color: InkTokens.ink300
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: InkTokens.hairline
                opacity: 0.6
            }

            Item {
                width: parent.width
                height: 36
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: InkTokens.rhythm * 2
                    text: qsTr("新建工作区…")
                    font.family: InkTokens.calligraphyFamily
                    font.pixelSize: 13
                    color: InkTokens.cinnabar
                    opacity: createHover.hovered ? 0.72 : 1
                }
                HoverHandler {
                    id: createHover
                    cursorShape: Qt.PointingHandCursor
                }
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    preventStealing: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (typeof study !== "undefined")
                            study.pickAndCreateWorkspace()
                    }
                }
            }
        }
    }

    FolderDialog {
        id: folderPicker
        title: qsTr("选择工作区文件夹")
        onAccepted: {
            if (typeof study !== "undefined")
                study.createWorkspaceFromPath(selectedFolder.toLocalFile())
        }
    }

    FileDialog {
        id: attachPicker
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("图像 (*.png *.jpg *.jpeg *.webp *.gif)")]
        onAccepted: {
            if (typeof study === "undefined")
                return
            var files = selectedFiles
            for (var i = 0; i < files.length; ++i)
                study.attachFromUrl(files[i])
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 1
        color: InkTokens.hairline
        opacity: 0.7
    }
}
