import QtQuick

Item {
    id: root
    visible: typeof study !== "undefined" && study.settingsOpen
    anchors.fill: parent
    z: 20

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0.102, 0.086, 0.071, 0.18)
        MouseArea {
            anchors.fill: parent
            onClicked: study.closeSettings()
        }
    }

    Rectangle {
        id: sheet
        width: Math.min(420, parent.width - 48)
        height: Math.min(520, parent.height - 72)
        anchors.right: parent.right
        anchors.rightMargin: 24
        anchors.top: parent.top
        anchors.topMargin: 56
        color: InkTokens.scrollPaper
        border.width: 1
        border.color: InkTokens.hairline
        radius: 1

        MouseArea {
            anchors.fill: parent
        }

        Text {
            id: heading
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: 18
            anchors.leftMargin: 20
            text: qsTr("册")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 22
            color: InkTokens.ink900
        }

        Text {
            anchors.top: heading.top
            anchors.right: parent.right
            anchors.rightMargin: 18
            text: qsTr("掩")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 14
            color: InkTokens.ink500
            MouseArea {
                anchors.fill: parent
                anchors.margins: -8
                cursorShape: Qt.PointingHandCursor
                onClicked: study.closeSettings()
            }
        }

        ListView {
            id: nsList
            anchors.top: heading.bottom
            anchors.topMargin: 12
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: openDoc.top
            anchors.bottomMargin: 8
            clip: true
            reuseItems: true
            cacheBuffer: 160
            boundsBehavior: Flickable.StopAtBounds
            model: (typeof study !== "undefined") ? study.settingsNamespaces : []
            delegate: Item {
                width: nsList.width
                height: 36
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 20
                    anchors.right: parent.right
                    anchors.rightMargin: 16
                    text: modelData.ns + (modelData.applies === "restart" ? qsTr(" · 须重启") : "")
                    font.family: InkTokens.calligraphyFamily
                    font.pixelSize: 13
                    color: InkTokens.ink700
                    elide: Text.ElideRight
                }
            }
        }

        Text {
            id: openDoc
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 16
            text: qsTr("打开配置文书")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 14
            color: InkTokens.cinnabar
            MouseArea {
                anchors.fill: parent
                anchors.margins: -8
                cursorShape: Qt.PointingHandCursor
                onClicked: study.openSettingsDocument()
            }
        }
    }
}
