import QtQuick
import dsh

Item {
    id: root

    property string connectionStatusText: qsTr("等待连接…")

    PaperBackground {
        anchors.fill: parent
    }

    TitleBar {
        id: titleBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Text {
        anchors.centerIn: parent
        text: root.connectionStatusText
        font.family: Qt.platform.os === "windows" ? "SimSun" : "serif"
        font.pixelSize: 18
        color: InkTokens.ink500
    }
}
