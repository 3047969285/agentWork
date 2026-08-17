pragma Singleton
import QtQuick

QtObject {
    property bool reduceMotion: false
    readonly property int bloomMs: reduceMotion ? 0 : 420
    readonly property real bloomMaxRadius: reduceMotion ? 0 : 48
    readonly property int maxConcurrentBlooms: reduceMotion ? 0 : 2
}
