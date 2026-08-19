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
        function onWorkspacePickerOpenChanged() {
            if (typeof study === "undefined")
                return
            if (study.workspacePickerOpen) {
                if (!workspacePicker.opened)
                    workspacePicker.open()
            } else if (workspacePicker.opened) {
                workspacePicker.close()
            }
        }
    }

    Timer {
        id: openWorkspacePickerSoon
        interval: 0
        repeat: false
        onTriggered: {
            if (typeof study !== "undefined" && !study.workspacePickerOpen)
                study.toggleWorkspacePicker()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: InkTokens.sidebarWash
        opacity: 0.55
        enabled: false
    }

    Item {
        id: head
        objectName: "sidebarWorkspaceHit"
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: InkTokens.rhythm * 2
        anchors.leftMargin: InkTokens.rhythm * 2 + 2
        anchors.rightMargin: InkTokens.rhythm * 2 + 2
        height: headColumn.height
        z: 2

        Column {
            id: headColumn
            width: parent.width
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
                    opacity: workspaceHit.containsMouse ? 0.72 : 1
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
        }

        // Overlay hit target (not a Column child) so layout cannot zero-size or bury the MouseArea.
        MouseArea {
            id: workspaceHit
            anchors.fill: parent
            z: 10
            acceptedButtons: Qt.LeftButton
            preventStealing: true
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (typeof study === "undefined")
                    return
                // Same click that opens must not be treated as PressOutside; defer open.
                if (study.workspacePickerOpen || workspacePicker.opened) {
                    openWorkspacePickerSoon.stop()
                    study.closeWorkspacePicker()
                    workspacePicker.close()
                    return
                }
                openWorkspacePickerSoon.start()
            }
        }
    }

    Text {
        id: sessionHeading
        anchors.top: head.bottom
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
        parent: root
        modal: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 0
        width: root.width - InkTokens.rhythm * 2
        x: InkTokens.rhythm
        y: head.y + head.height + 2

        onClosed: {
            openWorkspacePickerSoon.stop()
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

    Rectangle {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 1
        color: InkTokens.hairline
        opacity: 0.7
        enabled: false
    }
}
