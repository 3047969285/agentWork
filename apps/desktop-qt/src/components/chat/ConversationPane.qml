import QtQuick

Item {
    id: root

    readonly property bool empty: typeof study === "undefined" || study.transcript.count === 0
    readonly property bool hasSession: typeof study !== "undefined" && study.selectedSessionId.length > 0
    property bool stickToEnd: true

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
        reuseItems: true
        cacheBuffer: 420
        pixelAligned: true
        boundsBehavior: Flickable.StopAtBounds
        model: (typeof study !== "undefined") ? study.transcript : []

        delegate: Item {
            width: transcript.width
            height: model.kind === "tool" ? toolCard.height : textCol.height

            ToolCard {
                id: toolCard
                visible: model.kind === "tool"
                width: parent.width * 0.92
                title: model.title ? model.title : ""
                bodyText: model.body ? model.body : ""
                status: model.status ? model.status : ""
                card: model.card ? model.card : "generic"
            }

            Column {
                id: textCol
                visible: model.kind !== "tool"
                width: parent.width
                spacing: 6

                Text {
                    text: model.role === "user" ? qsTr("问") : qsTr("答")
                    font.family: InkTokens.calligraphyFamily
                    font.pixelSize: 11
                    color: model.role === "user" ? InkTokens.cinnabar : InkTokens.ink500
                }

                Text {
                    width: parent.width * 0.92
                    text: model.text ? model.text : ""
                    wrapMode: Text.Wrap
                    font.family: InkTokens.calligraphyFamily
                    font.pixelSize: 15
                    lineHeight: 1.45
                    color: InkTokens.ink900
                    opacity: model.streaming ? 0.82 : 1
                }
            }
        }

        onContentYChanged: {
            if (!moving && !dragging)
                return
            stickToEnd = atYEnd
        }
        onCountChanged: {
            if (stickToEnd && count > 0)
                positionViewAtEnd()
        }
        onContentHeightChanged: {
            if (stickToEnd)
                positionViewAtEnd()
        }
    }

    Column {
        visible: root.empty
        anchors.centerIn: parent
        spacing: 8

        Text {
            id: emptyGlyph
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("空")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 88
            color: InkTokens.ink900
            opacity: 0.10

            MouseArea {
                anchors.fill: parent
                onClicked: function (mouse) {
                    if (MotionBudget.maxConcurrentBlooms === 0)
                        return
                    var bloom = bloomComponent.createObject(root, {
                        x: emptyGlyph.x + mouse.x,
                        y: emptyGlyph.y + mouse.y
                    })
                    if (bloom)
                        bloom.z = 2
                }
            }
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

    Component {
        id: bloomComponent
        InkBloom {}
    }
}
