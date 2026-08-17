import QtQuick

Item {
    id: root
    enabled: false
    z: -1

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

    Canvas {
        id: fibers
        anchors.fill: parent
        renderStrategy: Canvas.Cooperative
        layer.enabled: true
        layer.smooth: false
        layer.mipmap: false
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.strokeStyle = "rgba(201, 184, 154, 0.22)"
            ctx.lineWidth = 1
            var rows = 9
            for (var i = 0; i < rows; ++i) {
                var y = height * (0.08 + i * 0.1)
                ctx.beginPath()
                ctx.moveTo(width * 0.04, y)
                ctx.bezierCurveTo(width * 0.28, y + 3, width * 0.62, y - 2, width * 0.96, y + 1)
                ctx.stroke()
            }
            ctx.strokeStyle = "rgba(63, 55, 46, 0.05)"
            for (var j = 0; j < 5; ++j) {
                var x = width * (0.12 + j * 0.18)
                ctx.beginPath()
                ctx.moveTo(x, height * 0.06)
                ctx.lineTo(x + 8, height * 0.94)
                ctx.stroke()
            }
        }
    }

    Rectangle {
        width: 18
        height: 18
        rotation: 14
        color: InkTokens.cinnabar
        opacity: 0.12
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 22
        anchors.bottomMargin: 18
    }
}
