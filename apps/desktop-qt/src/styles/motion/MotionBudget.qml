pragma Singleton
import QtQuick

QtObject {
    property bool reduceMotion: false
    readonly property int bloomMs: reduceMotion ? 0 : 420
    readonly property real bloomMaxRadius: reduceMotion ? 0 : 48
    readonly property int maxConcurrentBlooms: reduceMotion ? 0 : 2
    readonly property int hoverMs: reduceMotion ? 0 : 140
    readonly property int pressMs: reduceMotion ? 0 : 90
}
