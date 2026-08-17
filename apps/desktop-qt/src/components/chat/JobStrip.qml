import QtQuick

Item {
    id: root
    visible: typeof study !== "undefined" && study.jobs.length > 0
    height: visible ? Math.min(72, column.implicitHeight + 12) : 0

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0.969, 0.945, 0.902, 0.88)
        border.width: 1
        border.color: InkTokens.hairline
        opacity: 0.95
    }

    Column {
        id: column
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 20
        anchors.rightMargin: 18
        spacing: 4

        Text {
            text: qsTr("差事")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 11
            color: InkTokens.ink500
        }

        Repeater {
            model: (typeof study !== "undefined") ? study.jobs : []
            Text {
                required property var modelData
                width: column.width
                elide: Text.ElideRight
                text: (modelData.statusLabel ? modelData.statusLabel : "") + " · " + (modelData.label ? modelData.label : modelData.kind)
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 13
                color: modelData.live ? InkTokens.cinnabar : InkTokens.ink700
            }
        }
    }
}
