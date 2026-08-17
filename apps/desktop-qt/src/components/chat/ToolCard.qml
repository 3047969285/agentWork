import QtQuick

Item {
    id: root
    width: parent ? parent.width : 240
    height: Math.max(52, body.implicitHeight + 16)

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0.969, 0.945, 0.902, 0.72)
        border.width: 1
        border.color: Qt.rgba(0.788, 0.722, 0.604, 0.7)
        radius: 1
    }

    Rectangle {
        width: 3
        height: 14
        color: InkTokens.ink700
        opacity: 0.55
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.top: parent.top
        anchors.topMargin: 14
    }

    Column {
        id: body
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 20
        anchors.rightMargin: 12
        anchors.topMargin: 10
        spacing: 4

        Row {
            spacing: 8
            Text {
                text: title
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 13
                color: InkTokens.ink900
            }
            Text {
                text: status === "pending" ? qsTr("未竟") : (status === "error" ? qsTr("折") : qsTr("成"))
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 11
                color: status === "error" ? InkTokens.cinnabar : InkTokens.ink500
            }
        }

        Text {
            width: parent.width
            visible: bodyText.length > 0
            text: bodyText
            wrapMode: Text.Wrap
            maximumLineCount: card === "terminal" ? 12 : 6
            elide: Text.ElideRight
            font.family: card === "terminal" ? "Consolas" : InkTokens.calligraphyFamily
            font.pixelSize: 12
            lineHeight: 1.35
            color: InkTokens.ink700
        }
    }

    required property string title
    required property string bodyText
    required property string status
    required property string card
}
