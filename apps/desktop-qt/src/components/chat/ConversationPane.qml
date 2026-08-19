import QtQuick

Item {
    id: root

    readonly property bool empty: typeof study === "undefined" || study.transcript.count === 0
    readonly property bool hasSession: typeof study !== "undefined" && study.selectedSessionId.length > 0
    property bool stickToEnd: true

    Rectangle {
        anchors.fill: parent
        color: InkTokens.scrollPaper
        opacity: 0.22
        enabled: false
    }

    ListView {
        id: transcript
        visible: !root.empty
        anchors.fill: parent
        anchors.margins: InkTokens.rhythm * 2
        clip: true
        spacing: 6
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
            height: kind === "tool" ? toolCard.height : bubble.height

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

            Item {
                id: bubble
                visible: rowRoot.kind !== "tool"
                width: parent.width
                height: messageBody.y + messageBody.height

                Rectangle {
                    visible: rowRoot.role === "user"
                    width: Math.min(parent.width * 0.88, messageBody.implicitWidth + 20)
                    height: messageBody.implicitHeight + 16
                    x: parent.width - width
                    anchors.verticalCenter: messageBody.verticalCenter
                    color: Qt.rgba(0.651, 0.239, 0.184, 0.06)
                    radius: 1
                }

                Text {
                    id: messageBody
                    width: parent.width * (rowRoot.role === "user" ? 0.88 : 0.94)
                    x: rowRoot.role === "user" ? parent.width - width : 0
                    text: rowRoot.role === "user"
                          ? rowRoot.text
                          : ((typeof study !== "undefined")
                             ? study.formatAssistantText(rowRoot.text)
                             : rowRoot.text)
                    textFormat: rowRoot.role === "user" ? Text.PlainText : Text.RichText
                    wrapMode: Text.Wrap
                    font.family: InkTokens.bodyFamily
                    font.pixelSize: 15
                    lineHeight: 1.4
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
        spacing: InkTokens.rhythm

        Text {
            id: emptyGlyph
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("空")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 72
            color: InkTokens.ink900
            opacity: 0.08

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: function (eventPoint) {
                    if (MotionBudget.maxConcurrentBlooms === 0)
                        return
                    var mapped = emptyGlyph.mapToItem(root, eventPoint.position.x, eventPoint.position.y)
                    var bloom = bloomComponent.createObject(root, {
                        x: mapped.x,
                        y: mapped.y
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
                    return qsTr("尚未连接")
                if (!root.hasSession)
                    return qsTr("请选择或新建会话")
                return qsTr("纸净，待你写下第一句")
            }
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 15
            color: InkTokens.ink500
        }
    }

    Component {
        id: bloomComponent
        InkBloom {}
    }
}
