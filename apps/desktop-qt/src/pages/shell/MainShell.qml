import QtQuick
import QtQuick.Controls
import dsh
import "qrc:/dsh/src/components/ink" as InkComp
import "qrc:/dsh/src/components/chrome" as ChromeComp

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

    Column {
        anchors.centerIn: parent
        spacing: 16

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8

            Rectangle {
                width: 10
                height: 10
                radius: 5
                anchors.verticalCenter: parent.verticalCenter
                color: (typeof connection !== "undefined" && connection.connected)
                       ? InkTokens.ink900
                       : ((typeof connection !== "undefined" && connection.hasError)
                          ? InkTokens.cinnabar
                          : InkTokens.ink300)
            }

            Text {
                id: statusLabel
                text: (typeof connection !== "undefined") ? connection.statusText : qsTr("等待连接…")
                font.family: Qt.platform.os === "windows" ? "SimSun" : "serif"
                font.pixelSize: 18
                color: (typeof connection !== "undefined" && connection.hasError) ? InkTokens.cinnabar : InkTokens.ink500
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Button {
            id: retryBtn
            visible: typeof connection !== "undefined" && connection.hasError && !connection.connected
            text: qsTr("重新连接")
            anchors.horizontalCenter: parent.horizontalCenter
            font.family: Qt.platform.os === "windows" ? "SimSun" : "serif"
            font.pixelSize: 14

            contentItem: Text {
                text: retryBtn.text
                font: retryBtn.font
                color: InkTokens.cinnabar
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            background: Rectangle {
                implicitWidth: 100
                implicitHeight: 32
                color: retryBtn.down ? InkTokens.ink100 : InkTokens.ink0
                border.color: InkTokens.cinnabar
                border.width: 1
                radius: 4
            }

            onClicked: {
                if (typeof connection !== "undefined") {
                    connection.retry()
                }
            }
        }
    }
}
