import QtQuick
import dsh
import "qrc:/dsh/src/components/ink" as InkComp
import "qrc:/dsh/src/components/chrome" as ChromeComp
import "qrc:/dsh/src/components/chat" as ChatComp

Item {
    id: root

    InkComp.PaperBackground {
        anchors.fill: parent
    }

    ChromeComp.TitleBar {
        id: titleBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    ChromeComp.Sidebar {
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
    }

    ChatComp.ConversationPane {
        id: scroll
        anchors.top: inlineStatus.visible ? inlineStatus.bottom : titleBar.bottom
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.bottom: composer.top
    }

    ChatComp.Composer {
        id: composer
        anchors.left: sidebar.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}
