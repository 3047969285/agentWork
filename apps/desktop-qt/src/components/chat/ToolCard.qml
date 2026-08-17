import QtQuick

Item {
    id: root
    width: parent ? parent.width : 240
    implicitHeight: Math.max(52, body.implicitHeight + 18)
    height: implicitHeight

    required property string title
    required property string bodyText
    required property string status
    required property string card

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0.969, 0.945, 0.902, 0.72)
        border.width: 1
        border.color: Qt.rgba(0.788, 0.722, 0.604, 0.7)
        radius: 1
    }

    Repeater {
        model: 3
        Rectangle {
            width: 1
            height: Math.max(12, root.height - 20)
            color: InkTokens.cinnabar
            opacity: 0.18 - index * 0.04
            x: 10 + index * 3
            y: 10
        }
    }

    Column {
        id: body
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 24
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
                text: status === "pending" ? qsTr("进行中") : (status === "error" ? qsTr("失败") : qsTr("完成"))
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 11
                color: status === "error" ? InkTokens.cinnabar : InkTokens.ink500
            }
        }

        Text {
            width: parent.width
            visible: bodyText.length > 0
            text: bodyText
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            maximumLineCount: card === "terminal" ? 12 : 6
            elide: Text.ElideRight
            font.family: card === "terminal" ? "Consolas" : InkTokens.calligraphyFamily
            font.pixelSize: 12
            lineHeight: 1.35
            color: InkTokens.ink700
        }
    }
}
