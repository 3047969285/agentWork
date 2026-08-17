import QtQuick

Item {
    id: root
    enabled: false

    width: 0
    height: 0
    visible: MotionBudget.bloomMs > 0
    z: 100

    onVisibleChanged: {
        if (!visible)
            destroy()
    }

    property real bloomRadius: 4
    property real bloomOpacity: 0.35

    Rectangle {
        anchors.centerIn: parent
        width: root.bloomRadius * 2
        height: root.bloomRadius * 2
        radius: root.bloomRadius
        color: InkTokens.ink700
        opacity: root.bloomOpacity
    }

    SequentialAnimation {
        running: root.visible
        ParallelAnimation {
            NumberAnimation {
                target: root
                property: "bloomRadius"
                from: 4
                to: MotionBudget.bloomMaxRadius
                duration: MotionBudget.bloomMs
                easing.type: Easing.OutCubic
            }
            NumberAnimation {
                target: root
                property: "bloomOpacity"
                from: 0.35
                to: 0
                duration: MotionBudget.bloomMs
                easing.type: Easing.OutCubic
            }
        }
        ScriptAction {
            script: root.destroy()
        }
    }
}
