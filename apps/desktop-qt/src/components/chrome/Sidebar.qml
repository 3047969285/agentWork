import QtQuick

Item {
    id: root
    width: 228

    property bool workspaceOpen: false

    Rectangle {
        anchors.fill: parent
        color: InkTokens.sidebarWash
        opacity: 0.55
    }

    Column {
        id: head
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 16
        anchors.leftMargin: 18
        anchors.rightMargin: 18
        spacing: 4

        Text {
            text: qsTr("书房")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 12
            color: InkTokens.ink500
        }

        Text {
            id: workspaceName
            width: parent.width
            text: (typeof study !== "undefined") ? study.workspaceTitle : qsTr("未入席")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 18
            color: InkTokens.primaryText
            elide: Text.ElideMiddle
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true
                onClicked: root.workspaceOpen = !root.workspaceOpen
                onContainsMouseChanged: {
                    if (MotionBudget.hoverMs === 0)
                        return
                    workspaceName.opacity = containsMouse ? 0.72 : 1
                }
            }
            Behavior on opacity {
                enabled: MotionBudget.hoverMs > 0
                NumberAnimation { duration: MotionBudget.hoverMs }
            }
        }
    }

    ListView {
        id: workspaceList
        visible: root.workspaceOpen
        anchors.top: head.bottom
        anchors.topMargin: 8
        anchors.left: parent.left
        anchors.right: parent.right
        height: visible ? Math.min(120, count * 28) : 0
        clip: true
        reuseItems: true
        cacheBuffer: 80
        boundsBehavior: Flickable.StopAtBounds
        model: (typeof study !== "undefined") ? study.workspaces : []
        delegate: Item {
            width: workspaceList.width
            height: 28
            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 22
                anchors.right: parent.right
                anchors.rightMargin: 12
                text: modelData.title
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 13
                color: (typeof study !== "undefined" && study.workspaceId === modelData.workspaceId)
                       ? InkTokens.cinnabar : InkTokens.ink700
                elide: Text.ElideRight
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    study.selectWorkspace(modelData.workspaceId)
                    root.workspaceOpen = false
                }
            }
        }
    }

    ListView {
        id: sessionList
        anchors.top: workspaceList.visible ? workspaceList.bottom : head.bottom
        anchors.topMargin: 14
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: newSessionBtn.top
        anchors.bottomMargin: 10
        clip: true
        reuseItems: true
        cacheBuffer: 280
        pixelAligned: true
        boundsBehavior: Flickable.StopAtBounds
        spacing: 2
        model: (typeof study !== "undefined") ? study.sessions : []

        delegate: Item {
            id: rowRoot
            width: sessionList.width
            height: 36

            readonly property bool selected: typeof study !== "undefined"
                                             && study.selectedSessionId === sessionId

            Rectangle {
                id: wash
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                color: rowRoot.selected ? Qt.rgba(0.651, 0.239, 0.184, 0.10) : "transparent"
                border.width: rowRoot.selected ? 1 : 0
                border.color: Qt.rgba(0.651, 0.239, 0.184, 0.45)
                radius: 1
                Behavior on color {
                    enabled: MotionBudget.hoverMs > 0
                    ColorAnimation { duration: MotionBudget.hoverMs }
                }
            }

            Rectangle {
                visible: rowRoot.selected
                width: 3
                height: 16
                radius: 1
                color: InkTokens.cinnabar
                anchors.left: parent.left
                anchors.leftMargin: 14
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
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 24
                anchors.right: parent.right
                anchors.rightMargin: 22
                text: title
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 14
                color: rowRoot.selected ? InkTokens.ink900 : InkTokens.ink700
                elide: Text.ElideRight
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true
                onClicked: {
                    if (typeof study !== "undefined")
                        study.selectSession(sessionId)
                }
                onContainsMouseChanged: {
                    if (rowRoot.selected || MotionBudget.hoverMs === 0)
                        return
                    wash.color = containsMouse ? Qt.rgba(0.651, 0.239, 0.184, 0.05) : "transparent"
                }
            }
        }

        Text {
            visible: sessionList.count === 0
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 28
            text: qsTr("尚无卷宗")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 13
            color: InkTokens.ink300
        }
    }

    Text {
        id: newSessionBtn
        anchors.left: parent.left
        anchors.leftMargin: 18
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16
        text: qsTr("新会话")
        font.family: InkTokens.calligraphyFamily
        font.pixelSize: 14
        color: InkTokens.cinnabar
        opacity: 1

        MouseArea {
            anchors.fill: parent
            anchors.margins: -8
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true
            onClicked: {
                if (typeof study !== "undefined")
                    study.createSession()
            }
            onPressedChanged: newSessionBtn.opacity = pressed ? 0.55 : 1
        }
        Behavior on opacity {
            enabled: MotionBudget.pressMs > 0
            NumberAnimation { duration: MotionBudget.pressMs }
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
