import QtQuick
import dsh
import "qrc:/dsh/src/components/ink" as InkComp

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
            font.pixelSize: 22
            color: InkTokens.primaryText
            anchors.verticalCenter: parent.verticalCenter
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
            width: Math.min(320, root.width - 160)
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

    Text {
        id: reduceLabel
        anchors.right: parent.right
        anchors.rightMargin: 18
        anchors.verticalCenter: parent.verticalCenter
        text: MotionBudget.reduceMotion ? qsTr("静") : qsTr("动")
        font.family: InkTokens.calligraphyFamily
        font.pixelSize: 13
        color: InkTokens.ink500

        MouseArea {
            anchors.fill: parent
            anchors.margins: -8
            cursorShape: Qt.PointingHandCursor
            onClicked: MotionBudget.reduceMotion = !MotionBudget.reduceMotion
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
        InkComp.InkBloom {}
    }
}
