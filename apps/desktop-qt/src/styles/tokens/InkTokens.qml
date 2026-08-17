pragma Singleton
import QtQuick

QtObject {
    readonly property color ink0: "#F7F1E6"
    readonly property color ink100: "#E8E0D4"
    readonly property color ink300: "#B9A990"
    readonly property color ink500: "#6B5E4E"
    readonly property color ink700: "#3F372E"
    readonly property color ink900: "#1A1612"
    readonly property color cinnabar: "#A63D2F"
    readonly property color paperVein: "#EFE6D8"
    property bool dark: false
    readonly property color windowBg: dark ? ink900 : ink0
    readonly property color primaryText: dark ? ink100 : ink900
}
