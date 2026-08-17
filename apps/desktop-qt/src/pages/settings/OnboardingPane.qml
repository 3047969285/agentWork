import QtQuick
import QtQuick.Controls

Item {
    id: root
    visible: typeof study !== "undefined" && study.onboardingOpen
    enabled: visible
    anchors.fill: parent
    z: 30

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0.102, 0.086, 0.071, 0.28)
        TapHandler {
            acceptedButtons: Qt.LeftButton
        }
    }

    Rectangle {
        width: Math.min(420, parent.width - 48)
        height: Math.min(320, column.implicitHeight + 48)
        anchors.centerIn: parent
        color: InkTokens.scrollPaper
        border.width: 1
        border.color: InkTokens.hairline
        radius: 1

        Column {
            id: column
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 22
            spacing: 12

            Text {
                text: qsTr("入门")
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 22
                color: InkTokens.ink900
            }

            Text {
                width: parent.width
                wrapMode: Text.Wrap
                text: qsTr("尚未配置深度求索密钥，模型无法应答。把密钥写进宿主即可。")
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 14
                color: InkTokens.ink700
            }

            TextField {
                id: keyField
                width: parent.width
                echoMode: TextInput.Password
                placeholderText: qsTr("在此写入密钥…")
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 14
                color: InkTokens.ink900
                background: Rectangle {
                    color: "transparent"
                    border.width: 1
                    border.color: InkTokens.hairline
                }
            }

            Row {
                spacing: 18

                Text {
                    text: qsTr("写入密钥")
                    font.family: InkTokens.calligraphyFamily
                    font.pixelSize: 16
                    color: InkTokens.cinnabar
                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }
                    TapHandler {
                        margin: 8
                        acceptedButtons: Qt.LeftButton
                        onTapped: study.submitApiKey(keyField.text)
                    }
                }

                Text {
                    text: qsTr("稍后")
                    font.family: InkTokens.calligraphyFamily
                    font.pixelSize: 16
                    color: InkTokens.ink500
                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }
                    TapHandler {
                        margin: 8
                        acceptedButtons: Qt.LeftButton
                        onTapped: study.dismissOnboarding()
                    }
                }
            }
        }
    }
}
