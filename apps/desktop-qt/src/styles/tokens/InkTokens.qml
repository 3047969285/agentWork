pragma Singleton
import QtQuick

QtObject {
    readonly property int sidebarWidth: 240
    readonly property int rhythm: 8
    readonly property color ink0: "#F7F1E6"
    readonly property color ink100: "#EDE4D4"
    readonly property color ink300: "#B9A990"
    readonly property color ink500: "#6B5E4E"
    readonly property color ink700: "#3F372E"
    readonly property color ink900: "#1A1612"
    readonly property color cinnabar: "#A63D2F"
    readonly property color paperVein: "#EFE6D8"
    readonly property color sidebarWash: "#F1E6D4"
    readonly property color scrollPaper: "#F8F3E8"
    readonly property color hairline: "#C9B89A"
    readonly property color connectedDot: "#3D6B4F"
    readonly property string calligraphyFamily: Qt.platform.os === "windows" ? "KaiTi" : "serif"
    readonly property string bodyFamily: Qt.platform.os === "windows" ? "SimSun" : "serif"
    property bool dark: false
    readonly property color windowBg: dark ? ink900 : ink0
    readonly property color primaryText: dark ? ink100 : ink900
}
