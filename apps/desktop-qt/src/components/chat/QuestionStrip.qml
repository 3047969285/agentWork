import QtQuick
import QtQuick.Controls

Item {
    id: root
    visible: typeof study !== "undefined" && Object.keys(study.pendingQuestion).length > 0
    height: visible ? Math.min(220, column.implicitHeight + 16) : 0

    readonly property var q: (typeof study !== "undefined") ? study.pendingQuestion : ({})

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0.969, 0.945, 0.902, 0.92)
        border.width: 1
        border.color: InkTokens.hairline
        opacity: 0.95
    }

    Column {
        id: column
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 14
        spacing: 8

        Text {
            text: {
                if (root.q.intentKind === "plan-review")
                    return qsTr("计划待审") + (root.q.index ? " · " + root.q.index + "/" + root.q.total : "")
                return (root.q.header ? root.q.header + " · " : "") + (root.q.index ? (root.q.index + "/" + root.q.total) : "")
            }
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 11
            color: InkTokens.ink500
        }

        Text {
            width: parent.width
            text: root.q.question ? root.q.question : ""
            wrapMode: Text.Wrap
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 14
            color: InkTokens.ink900
        }

        Text {
            width: parent.width
            visible: !!(root.q.detail)
            text: root.q.detail ? root.q.detail : ""
            wrapMode: Text.Wrap
            maximumLineCount: 4
            elide: Text.ElideRight
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 12
            color: InkTokens.ink700
        }

        Flow {
            width: parent.width
            spacing: 8
            Repeater {
                model: root.q.options ? root.q.options : []
                Text {
                    text: modelData.label
                    font.family: InkTokens.calligraphyFamily
                    font.pixelSize: 13
                    color: InkTokens.cinnabar
                    leftPadding: 6
                    rightPadding: 6
                    topPadding: 2
                    bottomPadding: 2
                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }
                    TapHandler {
                        acceptedButtons: Qt.LeftButton
                        onTapped: study.pickQuestionOption(modelData.label)
                    }
                }
            }
        }

        TextField {
            id: custom
            visible: !root.q.options || root.q.options.length === 0
            width: parent.width
            placeholderText: qsTr("在此作答…")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 13
            color: InkTokens.ink900
            background: Rectangle {
                color: "transparent"
                border.width: 1
                border.color: InkTokens.hairline
            }
            Keys.onReturnPressed: {
                study.submitQuestionCustom(custom.text)
                custom.text = ""
            }
        }
    }
}
