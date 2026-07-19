// ---------------------------------------------------------------------------
// Notification helper functions — color resolution, icon lookup
// ---------------------------------------------------------------------------

function resolveAccentColor(icon, playbackMode, theme) {
    var ic = icon || "info";
    if (ic === "circle" || ic === "square") return theme.accentRed;
    if (ic === "save") return theme.accentGreen;
    // Playback mode-specific colors
    if (playbackMode === "RestartRetrigger") return "#378ADD";
    if (playbackMode === "ToggleStop") return "#185FA5";
    if (playbackMode === "QueuedSequential") return "#0C447C";
    if (playbackMode === "LayeredCutAll") return "#D85A30";
    if (playbackMode === "LayeredRingOut") return "#993C1D";
    return theme.accentPurple;
}

function resolveIcon(icon, playbackMode) {
    if (playbackMode === "RestartRetrigger") return "refresh-cw";
    if (playbackMode === "ToggleStop") return "toggle-left";
    if (playbackMode === "QueuedSequential") return "list-ordered";
    if (playbackMode === "LayeredCutAll") return "square-stack";
    if (playbackMode === "LayeredRingOut") return "audio-lines";
    return icon || "info";
}

function encodeColor(c) {
    return String(c).replace("#", "%23");
}
