import QtQuick

Item {
    id: root
    visible: typeof study !== "undefined" && Object.keys(study.pendingApproval).length > 0
    height: visible ? 56 : 0

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0.651, 0.239, 0.184, 0.08)
    }

    Text {
        id: reason
        anchors.left: parent.left
        anchors.right: actions.left
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 20
        anchors.rightMargin: 12
        text: {
            if (typeof study === "undefined")
                return ""
            var a = study.pendingApproval
            var tool = a.toolName ? a.toolName : qsTr("工具")
            var why = a.reason ? a.reason : qsTr("候君一诺")
            return tool + " · " + why
        }
        font.family: InkTokens.calligraphyFamily
        font.pixelSize: 13
        color: InkTokens.ink900
        elide: Text.ElideRight
    }

    Row {
        id: actions
        anchors.right: parent.right
        anchors.rightMargin: 18
        anchors.verticalCenter: parent.verticalCenter
        spacing: 16

        Text {
            text: qsTr("允")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 16
            color: InkTokens.cinnabar
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                margin: 6
                acceptedButtons: Qt.LeftButton
                onTapped: study.answerApproval("allowed-once")
            }
        }
        Text {
            text: qsTr("却")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 16
            color: InkTokens.ink500
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                margin: 6
                acceptedButtons: Qt.LeftButton
                onTapped: study.answerApproval("rejected")
            }
        }
    }
}
