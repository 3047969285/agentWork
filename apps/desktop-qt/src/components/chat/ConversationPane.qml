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

    Text {
        id: scrollTitle
        visible: root.hasSession
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 10
        anchors.leftMargin: 32
        anchors.rightMargin: 32
        height: visible ? 22 : 0
        text: (typeof study !== "undefined") ? study.selectedTitle : ""
        font.family: InkTokens.calligraphyFamily
        font.pixelSize: 13
        color: InkTokens.ink500
        elide: Text.ElideRight
        opacity: 0.85
    }

    ListView {
        id: transcript
        visible: !root.empty
        anchors.top: scrollTitle.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 28
        anchors.topMargin: scrollTitle.visible ? 4 : 28
        clip: true
        spacing: 18
        reuseItems: true
        cacheBuffer: 420
        pixelAligned: true
        boundsBehavior: Flickable.StopAtBounds
        flickDeceleration: 2500
        model: (typeof study !== "undefined") ? study.transcript : []

        delegate: Item {
            id: rowRoot
            required property string kind
            required property string role
            required property string text
            required property string title
            required property string body
            required property string status
            required property string card
            required property bool streaming

            width: transcript.width
            height: kind === "tool" ? toolCard.height : textCol.height

            ToolCard {
                id: toolCard
                visible: rowRoot.kind === "tool"
                width: parent.width * 0.92
                height: visible ? implicitHeight : 0
                title: rowRoot.title
                bodyText: rowRoot.body
                status: rowRoot.status
                card: rowRoot.card.length > 0 ? rowRoot.card : "generic"
            }

            Column {
                id: textCol
                visible: rowRoot.kind !== "tool"
                width: parent.width
                spacing: 6

                Text {
                    text: rowRoot.role === "user" ? qsTr("问") : qsTr("答")
                    font.family: InkTokens.calligraphyFamily
                    font.pixelSize: 11
                    color: rowRoot.role === "user" ? InkTokens.cinnabar : InkTokens.ink500
                }

                Text {
                    width: parent.width * 0.92
                    text: rowRoot.text
                    textFormat: Text.PlainText
                    wrapMode: Text.Wrap
                    font.family: InkTokens.calligraphyFamily
                    font.pixelSize: 15
                    lineHeight: 1.45
                    color: InkTokens.ink900
                    opacity: rowRoot.streaming ? 0.82 : 1
                }
            }
        }

        onContentYChanged: {
            if (!moving && !dragging)
                return
            stickToEnd = atYEnd
        }
        onCountChanged: stickTimer.restart()
        onContentHeightChanged: stickTimer.restart()
    }

    Timer {
        id: stickTimer
        interval: 16
        repeat: false
        onTriggered: {
            if (root.stickToEnd && transcript.count > 0)
                transcript.positionViewAtEnd()
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
