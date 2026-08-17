import QtQuick
import QtQuick.Window
import QtQuick.Controls
import dsh

Window {
    width: 1280
    height: 800
    visible: true
    title: qsTr("DeepSeek Harness")
    color: InkTokens.windowBg

    Component.onCompleted: InkLight.activate()

    MainShell {
        anchors.fill: parent
    }

    Switch {
        id: reduceMotionSwitch
        text: qsTr("减少动态效果")
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 16
        checked: MotionBudget.reduceMotion
        onCheckedChanged: MotionBudget.reduceMotion = checked

        contentItem: Text {
            text: reduceMotionSwitch.text
            font.family: "STSong, SimSun, serif"
            font.pixelSize: 13
            color: InkTokens.primaryText
            verticalAlignment: Text.AlignVCenter
            leftPadding: reduceMotionSwitch.indicator.width + reduceMotionSwitch.spacing
        }
    }
}
