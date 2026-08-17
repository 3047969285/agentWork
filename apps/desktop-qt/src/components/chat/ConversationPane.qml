import QtQuick

Item {
    id: root

    readonly property bool empty: typeof study === "undefined" || study.messages.length === 0
    readonly property bool hasSession: typeof study !== "undefined" && study.selectedSessionId.length > 0

    Rectangle {
        anchors.fill: parent
        color: InkTokens.scrollPaper
        opacity: 0.35
    }

    ListView {
        id: transcript
        visible: !root.empty
        anchors.fill: parent
        anchors.margins: 28
        clip: true
        spacing: 18
        boundsBehavior: Flickable.StopAtBounds
        model: (typeof study !== "undefined") ? study.messages : []

        delegate: Column {
            width: transcript.width
            spacing: 6

            Text {
                text: modelData.role === "user" ? qsTr("问") : qsTr("答")
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 11
                color: modelData.role === "user" ? InkTokens.cinnabar : InkTokens.ink500
            }

            Text {
                width: parent.width * 0.92
                text: modelData.text
                wrapMode: Text.Wrap
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 15
                lineHeight: 1.45
                color: InkTokens.ink900
            }
        }

        onCountChanged: {
            if (count > 0)
                positionViewAtEnd()
        }
    }

    Column {
        visible: root.empty
        anchors.centerIn: parent
        spacing: 8

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("空")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 88
            color: InkTokens.ink900
            opacity: 0.10
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: {
                if (typeof connection !== "undefined" && !connection.connected)
                    return qsTr("纸未通 · 候墨")
                if (!root.hasSession)
                    return qsTr("择一卷 · 或新开")
                return qsTr("纸净 · 待君落墨")
            }
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 16
            color: InkTokens.ink500
        }
    }
}
