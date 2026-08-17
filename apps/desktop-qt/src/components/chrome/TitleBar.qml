import QtQuick
import dsh

Item {
    id: root

    height: 56
    width: parent ? parent.width : 0

    property int activeBlooms: 0

    Text {
        text: qsTr("深卷")
        font.family: "STSong, SimSun, serif"
        font.pixelSize: 28
        color: InkTokens.primaryText
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.verticalCenter: parent.verticalCenter
    }

    Text {
        text: qsTr("DeepSeek Harness")
        font.family: "STSong, SimSun, serif"
        font.pixelSize: 14
        color: InkTokens.ink500
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.top: parent.top
        anchors.topMargin: 36
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
        InkBloom {}
    }
}
