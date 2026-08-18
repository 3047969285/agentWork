import QtQuick

Item {
    id: root
    height: 48

    property int activeBlooms: 0
    property int lastBloomMs: 0

    function spawnBloom(localPos) {
        if (MotionBudget.maxConcurrentBlooms === 0
                || root.activeBlooms >= MotionBudget.maxConcurrentBlooms) {
            return
        }
        var now = Date.now()
        if (now - root.lastBloomMs < MotionBudget.bloomCooldownMs)
            return
        root.lastBloomMs = now
        var host = root.parent
        if (!host)
            return
        var mapped = brand.mapToItem(host, localPos.x, localPos.y)
        root.activeBlooms++
        var bloom = bloomComponent.createObject(host, {
            x: mapped.x,
            y: mapped.y
        })
        if (!bloom) {
            root.activeBlooms--
            return
        }
        bloom.destroyed.connect(function () {
            root.activeBlooms--
        })
    }

    Row {
        id: brandRow
        anchors.left: parent.left
        anchors.leftMargin: InkTokens.rhythm * 2 + 4
        anchors.verticalCenter: parent.verticalCenter
        spacing: InkTokens.rhythm

        Text {
            id: brand
            text: qsTr("深卷")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 22
            color: InkTokens.primaryText
            anchors.verticalCenter: parent.verticalCenter

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: function (eventPoint) {
                    root.spawnBloom(eventPoint.position)
                }
            }
        }

        Rectangle {
            id: chop
            width: 16
            height: 16
            radius: 1
            rotation: 7
            anchors.verticalCenter: parent.verticalCenter
            color: "transparent"
            border.width: 1
            border.color: Qt.rgba(0.651, 0.239, 0.184, 0.7)
            Text {
                anchors.centerIn: parent
                rotation: -7
                text: qsTr("卷")
                font.family: InkTokens.calligraphyFamily
                font.pixelSize: 10
                color: InkTokens.cinnabar
            }
        }

        Rectangle {
            id: seal
            width: 8
            height: 8
            radius: 4
            anchors.verticalCenter: parent.verticalCenter
            color: {
                if (typeof connection === "undefined")
                    return InkTokens.ink300
                if (connection.connected)
                    return InkTokens.connectedDot
                if (connection.hasError)
                    return InkTokens.cinnabar
                return InkTokens.ink300
            }
            opacity: (typeof connection !== "undefined" && connection.connecting) ? 0.55 : 1
        }

        Text {
            visible: typeof connection !== "undefined" && connection.hasError
            text: qsTr("重连")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 12
            color: InkTokens.cinnabar
            anchors.verticalCenter: parent.verticalCenter

            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                margin: 8
                acceptedButtons: Qt.LeftButton
                onTapped: {
                    if (typeof connection !== "undefined")
                        connection.retry()
                }
            }
        }
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: InkTokens.rhythm * 2 + 2
        anchors.verticalCenter: parent.verticalCenter
        spacing: InkTokens.rhythm * 2

        Text {
            objectName: "titleRefresh"
            text: qsTr("刷新")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 12
            color: InkTokens.ink500
            opacity: refreshHover.hovered ? 0.72 : 1
            HoverHandler {
                id: refreshHover
                cursorShape: Qt.PointingHandCursor
            }
            MouseArea {
                anchors.fill: parent
                anchors.margins: -8
                acceptedButtons: Qt.LeftButton
                preventStealing: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (typeof study !== "undefined")
                        study.refresh()
                }
            }
            Behavior on opacity {
                enabled: MotionBudget.hoverMs > 0
                NumberAnimation { duration: MotionBudget.hoverMs }
            }
        }

        Text {
            objectName: "titleSettings"
            text: qsTr("设置")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 12
            color: InkTokens.ink500
            opacity: settingsHover.hovered ? 0.72 : 1
            HoverHandler {
                id: settingsHover
                cursorShape: Qt.PointingHandCursor
            }
            MouseArea {
                anchors.fill: parent
                anchors.margins: -8
                acceptedButtons: Qt.LeftButton
                preventStealing: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (typeof study !== "undefined")
                        study.openSettings()
                }
            }
            Behavior on opacity {
                enabled: MotionBudget.hoverMs > 0
                NumberAnimation { duration: MotionBudget.hoverMs }
            }
        }

        Text {
            id: reduceLabel
            text: MotionBudget.reduceMotion ? qsTr("动效关") : qsTr("动效开")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 12
            color: InkTokens.ink500
            opacity: reduceHover.hovered ? 0.72 : 1
            HoverHandler {
                id: reduceHover
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                margin: 8
                acceptedButtons: Qt.LeftButton
                onTapped: MotionBudget.reduceMotion = !MotionBudget.reduceMotion
            }
            Behavior on opacity {
                enabled: MotionBudget.hoverMs > 0
                NumberAnimation { duration: MotionBudget.hoverMs }
            }
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: InkTokens.hairline
        opacity: 0.45
        enabled: false
    }

    Component {
        id: bloomComponent
        InkBloom {}
    }
}
