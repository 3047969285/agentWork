import QtQuick
import QtQuick.Controls
import dsh

Item {
    id: root
    height: 92

    readonly property bool canSend: typeof connection !== "undefined"
                                    && connection.connected
                                    && typeof study !== "undefined"
                                    && study.selectedSessionId.length > 0
                                    && !study.sending
                                    && input.text.trim().length > 0

    function submit() {
        if (!root.canSend)
            return
        study.sendPrompt(input.text)
        input.text = ""
    }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: InkTokens.hairline
        opacity: 0.65
    }

    Text {
        id: notice
        visible: typeof study !== "undefined" && study.noticeText.length > 0
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.right: sendSeal.left
        anchors.rightMargin: 12
        text: (typeof study !== "undefined") ? study.noticeText : ""
        font.family: InkTokens.calligraphyFamily
        font.pixelSize: 12
        color: InkTokens.cinnabar
        elide: Text.ElideRight
    }

    TextArea {
        id: input
        anchors.left: parent.left
        anchors.right: sendSeal.left
        anchors.top: notice.visible ? notice.bottom : parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 20
        anchors.rightMargin: 16
        anchors.topMargin: notice.visible ? 4 : 12
        anchors.bottomMargin: 12
        wrapMode: TextEdit.Wrap
        placeholderText: qsTr("书于此…")
        font.family: InkTokens.calligraphyFamily
        font.pixelSize: 15
        color: InkTokens.ink900
        enabled: typeof connection !== "undefined" && connection.connected
                 && typeof study !== "undefined" && study.selectedSessionId.length > 0
        background: Item {}
        Keys.onPressed: function (event) {
            if ((event.modifiers & Qt.ControlModifier)
                    && (event.key === Qt.Key_Return || event.key === Qt.Key_Enter)) {
                root.submit()
                event.accepted = true
            }
        }
    }

    Rectangle {
        id: sendSeal
        width: 44
        height: 44
        radius: 2
        rotation: 8
        anchors.right: parent.right
        anchors.rightMargin: 22
        anchors.verticalCenter: parent.verticalCenter
        color: root.canSend ? InkTokens.cinnabar : Qt.rgba(0.651, 0.239, 0.184, 0.28)

        Text {
            anchors.centerIn: parent
            rotation: -8
            text: qsTr("发")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 18
            color: InkTokens.ink0
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: root.canSend ? Qt.PointingHandCursor : Qt.ArrowCursor
            enabled: root.canSend
            onClicked: root.submit()
        }
    }
}
