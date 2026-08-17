import QtQuick
import dsh

Item {
    id: root
    width: 228

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
            width: parent.width
            text: (typeof study !== "undefined") ? study.workspaceTitle : qsTr("未入席")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 18
            color: InkTokens.primaryText
            elide: Text.ElideMiddle
        }
    }

    ListView {
        id: sessionList
        anchors.top: head.bottom
        anchors.topMargin: 14
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: newSessionBtn.top
        anchors.bottomMargin: 10
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        spacing: 2
        model: (typeof study !== "undefined") ? study.sessions : []

        delegate: Item {
            id: rowRoot
            width: sessionList.width
            height: 36

            readonly property bool selected: typeof study !== "undefined"
                                             && study.selectedSessionId === modelData.sessionId

            Rectangle {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                color: rowRoot.selected ? Qt.rgba(0.651, 0.239, 0.184, 0.10) : "transparent"
                border.width: rowRoot.selected ? 1 : 0
                border.color: Qt.rgba(0.651, 0.239, 0.184, 0.45)
                radius: 1
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

            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 24
                anchors.right: parent.right
                anchors.rightMargin: 16
                text: modelData.title
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 14
                color: rowRoot.selected ? InkTokens.ink900 : InkTokens.ink700
                elide: Text.ElideRight
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (typeof study !== "undefined")
                        study.selectSession(modelData.sessionId)
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

        MouseArea {
            anchors.fill: parent
            anchors.margins: -8
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (typeof study !== "undefined")
                    study.createSession()
            }
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
