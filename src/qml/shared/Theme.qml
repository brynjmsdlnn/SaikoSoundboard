pragma Singleton
import QtQuick

QtObject {
    readonly property color appBackground: "#0f0f0f"
    readonly property color cardBackground: "#131313"
    readonly property color inputBackground: "#121212"
    readonly property color recessedBackground: "#0a0a0a"

    readonly property color borderDefault: "#1c1c1c"
    readonly property color borderHover: "#333333"
    readonly property color borderFocus: "#BB86FC"

    readonly property color accentPurple: "#BB86FC"
    readonly property color accentTeal: "#03DAC6"
    readonly property color accentGreen: "#4caf50"
    readonly property color accentRed: "#e35d5d"
    readonly property color destructiveRed: "#ff5555"

    readonly property color textPrimary: "#ffffff"
    readonly property color textSecondary: "#b0b0b0"
    readonly property color textDim: "#888888"

    readonly property int animDuration: 150

    readonly property int borderRadius: 6
    readonly property int cardRadius: 8

    readonly property int fontSizeSmall: 10
    readonly property int fontSizeNormal: 12
    readonly property int fontSizeHeading: 14
}
