import QtQuick
import dsh
import "qrc:/dsh/src/components/ink" as InkComp

Item {
    id: root

    height: 56
    width: parent ? parent.width : 0

    property int activeBlooms: 0

    Column {
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.verticalCenter: parent.verticalCenter
        spacing: 0

        Text {
            text: qsTr("深卷")
            font.family: Qt.platform.os === "windows" ? "SimSun" : "serif"
            font.pixelSize: 22
            color: InkTokens.primaryText
        }

        Text {
            text: qsTr("DeepSeek Harness")
            font.family: Qt.platform.os === "windows" ? "SimSun" : "serif"
            font.pixelSize: 12
            color: InkTokens.ink500
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: InkTokens.ink100
        opacity: 0.6
    }

    MouseArea {
        anchors.fill: parent
        onClicked: function (mouse) {
            if (MotionBudget.maxConcurrentBlooms === 0
                    || root.activeBlooms >= MotionBudget.maxConcurrentBlooms) {
                return
            }
            root.activeBlooms++
            var host = root.parent
            var bloom = bloomComponent.createObject(host, {
                x: root.x + mouse.x,
                y: root.y + mouse.y
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

    Component {
        id: bloomComponent
        InkComp.InkBloom {}
    }
}
