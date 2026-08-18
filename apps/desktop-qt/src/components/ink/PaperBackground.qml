import QtQuick

Item {
    id: root
    enabled: false
    z: -1

    Rectangle {
        anchors.fill: parent
        color: InkTokens.windowBg
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: Qt.rgba(0.945, 0.918, 0.875, 0.55) }
            GradientStop { position: 0.45; color: Qt.rgba(0.969, 0.945, 0.902, 0.0) }
            GradientStop { position: 1.0; color: Qt.rgba(0.937, 0.906, 0.855, 0.35) }
        }
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: Qt.rgba(0.949, 0.925, 0.882, 0.12) }
            GradientStop { position: 0.5; color: Qt.rgba(0.969, 0.945, 0.902, 0.0) }
            GradientStop { position: 1.0; color: Qt.rgba(0.949, 0.925, 0.882, 0.12) }
        }
    }
}
