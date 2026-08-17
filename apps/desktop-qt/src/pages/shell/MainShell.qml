import QtQuick

Item {
    id: root

    PaperBackground {
        anchors.fill: parent
        z: -1
    }

    TitleBar {
        id: titleBar
        z: 2
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Sidebar {
        id: sidebar
        z: 1
        anchors.top: titleBar.bottom
        anchors.left: parent.left
        anchors.bottom: parent.bottom
    }

    Text {
        id: inlineStatus
        z: 1
        enabled: visible
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
        TapHandler {
            enabled: typeof study !== "undefined" && study.noticeText.length > 0
            acceptedButtons: Qt.LeftButton
            onTapped: study.dismissNotice()
        }
    }

    ConversationPane {
        id: scroll
        z: 0
        anchors.top: inlineStatus.visible ? inlineStatus.bottom : titleBar.bottom
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.bottom: approvalStrip.top
    }

    ApprovalStrip {
        id: approvalStrip
        z: 1
        enabled: visible
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.bottom: questionStrip.top
    }

    QuestionStrip {
        id: questionStrip
        z: 1
        enabled: visible
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.bottom: jobStrip.top
    }

    JobStrip {
        id: jobStrip
        z: 1
        enabled: visible
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.bottom: composer.top
    }

    Composer {
        id: composer
        z: 1
        objectName: "composerRoot"
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }

    SettingsPane {
        z: 20
        enabled: visible
        anchors.left: parent.left
        anchors.top: parent.top
    }

    OnboardingPane {
        z: 30
        enabled: visible
        anchors.left: parent.left
        anchors.top: parent.top
    }
}
