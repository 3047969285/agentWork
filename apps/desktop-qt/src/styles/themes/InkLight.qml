import QtQuick

QtObject {
    readonly property string name: "ink-light"
    readonly property bool dark: false

    function activate() {
        InkTokens.dark = dark
    }
}
