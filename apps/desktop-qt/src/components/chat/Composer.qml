import QtQuick
import QtQuick.Controls

Item {
    id: root
    height: 108 + (modelsOpen ? Math.min(160, modelList.contentHeight + 8) : 0)

    readonly property bool canSend: typeof connection !== "undefined"
                                    && connection.connected
                                    && typeof study !== "undefined"
                                    && study.selectedSessionId.length > 0
                                    && !study.sending
                                    && input.text.trim().length > 0
    readonly property bool modelsOpen: typeof study !== "undefined" && study.modelsOpen

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
        id: modelChip
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.top: parent.top
        anchors.topMargin: 8
        text: (typeof study !== "undefined") ? study.modelLabel : qsTr("模型")
        font.family: InkTokens.calligraphyFamily
        font.pixelSize: 12
        color: InkTokens.ink500
        MouseArea {
            anchors.fill: parent
            anchors.margins: -6
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (typeof study !== "undefined")
                    study.toggleModels()
            }
        }
    }

    ListView {
        id: modelList
        visible: root.modelsOpen
        anchors.left: parent.left
        anchors.right: sendSeal.left
        anchors.top: modelChip.bottom
        anchors.topMargin: 4
        height: visible ? Math.min(148, contentHeight) : 0
        clip: true
        reuseItems: true
        cacheBuffer: 80
        boundsBehavior: Flickable.StopAtBounds
        model: (typeof study !== "undefined") ? study.modelOptions : []
        delegate: Item {
            required property var modelData
            width: modelList.width
            height: 26
            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 24
                text: modelData.group + " · " + modelData.name
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 12
                color: InkTokens.ink700
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: study.selectModel(modelData.provider, modelData.model)
            }
        }
    }

    TextArea {
        id: input
        anchors.left: parent.left
        anchors.right: sendSeal.left
        anchors.top: modelList.visible ? modelList.bottom : modelChip.bottom
        anchors.bottom: parent.bottom
        anchors.leftMargin: 20
        anchors.rightMargin: 16
        anchors.topMargin: 4
        anchors.bottomMargin: 12
        wrapMode: TextEdit.Wrap
        placeholderText: qsTr("书于此…")
        font.family: InkTokens.calligraphyFamily
        font.pixelSize: 15
        color: InkTokens.ink900
        placeholderTextColor: InkTokens.ink300
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
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 22
        color: {
            if (typeof study !== "undefined" && study.sending)
                return InkTokens.ink700
            return root.canSend ? InkTokens.cinnabar : Qt.rgba(0.651, 0.239, 0.184, 0.28)
        }
        opacity: 1

        Text {
            anchors.centerIn: parent
            rotation: -8
            text: (typeof study !== "undefined" && study.sending) ? qsTr("止") : qsTr("发")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 18
            color: InkTokens.ink0
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true
            onClicked: {
                if (typeof study !== "undefined" && study.sending) {
                    study.cancelTurn()
                    return
                }
                root.submit()
            }
            onPressedChanged: sendSeal.opacity = pressed ? 0.7 : 1
        }
        Behavior on opacity {
            enabled: MotionBudget.pressMs > 0
            NumberAnimation { duration: MotionBudget.pressMs }
        }
    }
}
