import QtQuick
import dsh

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: InkTokens.windowBg
    }

    // Qt interpolates GradientStop RGB toward black when a stop is "transparent".
    // Keep every stop on the xuan-paper hue and vary only alpha.
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: Qt.rgba(0.937, 0.902, 0.847, 0.45) }
            GradientStop { position: 0.42; color: Qt.rgba(0.969, 0.945, 0.902, 0.0) }
            GradientStop { position: 1.0; color: Qt.rgba(0.929, 0.894, 0.831, 0.28) }
        }
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: Qt.rgba(0.945, 0.910, 0.855, 0.18) }
            GradientStop { position: 0.5; color: Qt.rgba(0.969, 0.945, 0.902, 0.0) }
            GradientStop { position: 1.0; color: Qt.rgba(0.945, 0.910, 0.855, 0.18) }
        }
    }
}
