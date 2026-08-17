import QtQuick

Item {
    id: root

    PaperBackground {
        anchors.fill: parent
    }

    TitleBar {
        id: titleBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Sidebar {
        id: sidebar
        anchors.top: titleBar.bottom
        anchors.left: parent.left
        anchors.bottom: parent.bottom
    }

    Text {
        id: inlineStatus
        visible: typeof connection !== "undefined"
                 && (!connection.connected || connection.hasError
                     || (typeof study !== "undefined" && study.noticeText.length > 0))
        anchors.top: titleBar.bottom
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        height: visible ? 22 : 0
        verticalAlignment: Text.AlignVCenter
        font.family: InkTokens.calligraphyFamily
        font.pixelSize: 12
        color: InkTokens.cinnabar
        elide: Text.ElideRight
        text: {
            if (typeof study !== "undefined" && study.noticeText.length > 0)
                return study.noticeText
            if (typeof connection !== "undefined")
                return connection.statusText
            return ""
        }
        MouseArea {
            anchors.fill: parent
            enabled: typeof study !== "undefined" && study.noticeText.length > 0
            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: study.dismissNotice()
        }
    }

    ConversationPane {
        id: scroll
        anchors.top: inlineStatus.visible ? inlineStatus.bottom : titleBar.bottom
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.bottom: approvalStrip.top
    }

    ApprovalStrip {
        id: approvalStrip
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.bottom: questionStrip.top
    }

    QuestionStrip {
        id: questionStrip
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.bottom: composer.top
    }

    Composer {
        id: composer
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    SettingsPane {
        anchors.fill: parent
    }
}
