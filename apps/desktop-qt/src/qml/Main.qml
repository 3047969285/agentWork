import QtQuick
import QtQuick.Window
import QtQuick.Controls
import dsh
import "qrc:/dsh/src/pages/shell" as Shell
import "qrc:/dsh/src/styles/themes" as Themes

Window {
    width: 1280
    height: 800
    visible: true
    title: qsTr("DeepSeek Harness")
    color: InkTokens.windowBg

    Component.onCompleted: Themes.InkLight.activate()

    Shell.MainShell {
        anchors.fill: parent
    }

    Switch {
        id: reduceMotionSwitch
        text: qsTr("减少动态效果")
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 20
        checked: MotionBudget.reduceMotion
        onCheckedChanged: MotionBudget.reduceMotion = checked

        indicator: Rectangle {
            implicitWidth: 36
            implicitHeight: 20
            x: reduceMotionSwitch.leftPadding
            y: parent.height / 2 - height / 2
            radius: 10
            color: reduceMotionSwitch.checked ? InkTokens.ink700 : InkTokens.ink100
            border.color: InkTokens.ink300
            border.width: 1

            Rectangle {
                x: reduceMotionSwitch.checked ? parent.width - width - 2 : 2
                y: 2
                width: 16
                height: 16
                radius: 8
                color: InkTokens.ink0
                Behavior on x {
                    NumberAnimation { duration: MotionBudget.reduceMotion ? 0 : 150 }
                }
            }
        }

        contentItem: Text {
            text: reduceMotionSwitch.text
            font.family: Qt.platform.os === "windows" ? "SimSun" : "serif"
            font.pixelSize: 13
            color: InkTokens.primaryText
            verticalAlignment: Text.AlignVCenter
            leftPadding: reduceMotionSwitch.indicator.width + reduceMotionSwitch.spacing
        }
    }
}
