import QtQuick
import QtQuick.Window
import dsh
import "qrc:/dsh/src/pages/shell" as Shell
import "qrc:/dsh/src/styles/themes" as Themes

Window {
    width: 1280
    height: 800
    visible: true
    title: qsTr("深卷")
    color: InkTokens.windowBg

    Component.onCompleted: Themes.InkLight.activate()

    Shell.MainShell {
        anchors.fill: parent
    }
}
