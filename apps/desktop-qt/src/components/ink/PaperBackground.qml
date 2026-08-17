import QtQuick
import dsh

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: InkTokens.windowBg
    }

    Rectangle {
        anchors.fill: parent
        opacity: 0.18
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: InkTokens.paperVein }
            GradientStop { position: 0.45; color: "transparent" }
            GradientStop { position: 1.0; color: InkTokens.ink100 }
        }
    }

    Rectangle {
        anchors.fill: parent
        opacity: 0.08
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 0.5; color: InkTokens.paperVein }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }
}
