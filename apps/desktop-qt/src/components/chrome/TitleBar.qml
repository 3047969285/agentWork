import QtQuick

Item {
    id: root
    height: 52

    property int activeBlooms: 0

    function spawnBloom(localPos) {
        if (MotionBudget.maxConcurrentBlooms === 0
                || root.activeBlooms >= MotionBudget.maxConcurrentBlooms) {
            return
        }
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
        anchors.leftMargin: 20
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        Text {
            id: brand
            text: qsTr("深卷")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 24
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
            width: 18
            height: 18
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
                font.pixelSize: 11
                color: InkTokens.cinnabar
            }
        }

        Rectangle {
            id: seal
            width: 10
            height: 10
            radius: 1
            rotation: 12
            anchors.verticalCenter: parent.verticalCenter
            color: {
                if (typeof connection === "undefined")
                    return InkTokens.ink300
                if (connection.connected)
                    return InkTokens.ink900
                if (connection.hasError)
                    return InkTokens.cinnabar
                return InkTokens.ink300
            }
        }

        Text {
            id: statusInline
            width: Math.min(280, root.width - 220)
            elide: Text.ElideRight
            text: (typeof connection !== "undefined") ? connection.statusText : qsTr("等待连接…")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 12
            color: (typeof connection !== "undefined" && connection.hasError)
                   ? InkTokens.cinnabar : InkTokens.ink500
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            visible: typeof connection !== "undefined" && connection.hasError
            text: qsTr("重连")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 13
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
        anchors.rightMargin: 18
        anchors.verticalCenter: parent.verticalCenter
        spacing: 16

        Text {
            objectName: "titleRefresh"
            text: qsTr("刷新")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 13
            color: InkTokens.ink500
            HoverHandler {
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
        }

        Text {
            objectName: "titleSettings"
            text: qsTr("设置")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 13
            color: InkTokens.ink500
            HoverHandler {
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
        }

        Text {
            id: reduceLabel
            text: MotionBudget.reduceMotion ? qsTr("动效关") : qsTr("动效开")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 13
            color: InkTokens.ink500
            HoverHandler {
                id: reduceHover
                cursorShape: Qt.PointingHandCursor
            }
            TapHandler {
                margin: 8
                acceptedButtons: Qt.LeftButton
                onTapped: MotionBudget.reduceMotion = !MotionBudget.reduceMotion
            }
        }

        Text {
            visible: reduceHover.hovered
            text: MotionBudget.reduceMotion ? qsTr("已减动效") : qsTr("减少动态效果")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 11
            color: InkTokens.ink300
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: InkTokens.hairline
        opacity: 0.55
        enabled: false
    }

    Component {
        id: bloomComponent
        InkBloom {}
    }
}
