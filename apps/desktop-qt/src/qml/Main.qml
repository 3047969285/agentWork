import QtQuick
import QtQuick.Window

Window {
    width: 1280
    height: 800
    visible: true
    title: qsTr("深卷")
    color: InkTokens.windowBg

    InkLight {
        id: inkLight
    }

    Component.onCompleted: inkLight.activate()

    MainShell {
        anchors.fill: parent
    }
}
