import QtQuick

Item {
    id: root
    height: 52

    property int activeBlooms: 0

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
            text: (typeof connection !== "undefined") ? connection.statusText : qsTr("候墨")
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

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (typeof connection !== "undefined")
                        connection.retry()
                }
            }
        }
    }

    MouseArea {
        anchors.fill: brand
        onClicked: function (mouse) {
            if (MotionBudget.maxConcurrentBlooms === 0
                    || root.activeBlooms >= MotionBudget.maxConcurrentBlooms) {
                return
            }
            root.activeBlooms++
            var host = root.parent
            var bloom = bloomComponent.createObject(host, {
                x: brandRow.x + mouse.x,
                y: brandRow.y + mouse.y
            })
            if (!bloom) {
                root.activeBlooms--
                return
            }
            bloom.destroyed.connect(function () {
                root.activeBlooms--
            })
        }
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: 18
        anchors.verticalCenter: parent.verticalCenter
        spacing: 16

        Text {
            text: qsTr("册")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 13
            color: InkTokens.ink500
            MouseArea {
                anchors.fill: parent
                anchors.margins: -8
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (typeof study !== "undefined")
                        study.openSettings()
                }
            }
        }

        Text {
            id: reduceLabel
            text: MotionBudget.reduceMotion ? qsTr("静") : qsTr("动")
            font.family: InkTokens.calligraphyFamily
            font.pixelSize: 13
            color: InkTokens.ink500
            MouseArea {
                id: reduceHit
                anchors.fill: parent
                anchors.margins: -8
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true
                onClicked: MotionBudget.reduceMotion = !MotionBudget.reduceMotion
            }
        }

        Text {
            visible: reduceHit.containsMouse
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
    }

    Component {
        id: bloomComponent
        InkBloom {}
    }
}
