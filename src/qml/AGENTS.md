# QML UI Layer

## Purpose

All QML UI components for the SaikoSoundboard application. This is the presentation layer — declares the UI, binds to `Backend` singletons, and orchestrates user interaction.

## Ownership

- 23 `.qml` files + 1 `utils.js`, organized into feature-based subdirectories
- All files are part of the `Saiko` 1.0 QML module (declared in root CMakeLists.txt via `qt_add_qml_module`)
- `Theme.qml` is a registered singleton (`QT_QML_SINGLETON_TYPE`)
- The JS file `utils.js` is imported as `import "../shared/utils.js" as Utils` (or `import "shared/utils.js" as Utils` from root-level files)

### Directory Structure

```
src/qml/
├── components/       # Reusable UI building blocks (SaikoButton, SaikoCheckBox, SaikoComboBox, SaikoTooltip, SaikoMenu, SaikoMenuItem)
├── shared/           # Shared singleton and utilities (Theme.qml, utils.js)
├── dialogs/          # Standalone windows & popups (SettingsDialog, HotkeyDialog, RoutingDialog, ProcessSelectionPopup)
├── recording/        # Recording panel sub-components (RecordingPanel, RecordingHeader, CaptureModeBar, ReplayBufferSection, TransportControls)
├── soundboard/       # Soundboard grid & card editor (SoundboardGridView, SoundboardGridCard, ItemEditor, HotkeyCard)
├── sources/          # Audio source management (SourcesPanel, WaveformView)
└── Main.qml          # Application entry point
```

## Local Contracts

- All QML files must use `import QtQuick 2.15` syntax (Qt5 compat imports)
- All files must `import Saiko 1.0` to access `Backend`, `SlotModel`, `Theme`, `WaveformData`, `RealtimeWaveform`
- Colors and design tokens come from `Theme.xxx` — no hardcoded color literals
- Buttons use `SaikoButton` — no custom Rectangle+MouseArea button implementations
- Card-style wrappers use `SectionCard` with `heading` property
- New standalone components that extract inline blocks must be added to `QML_FILES` in root CMakeLists.txt
- Components within the same `Saiko` module can reference each other without explicit file-path imports
- `utils.js` is the only JS file — add shared utility functions here, not in individual QML files
- When adding a `qrc:/icons/` or `image://icons/` reference in QML, verify the SVG exists in `resources/icons/` and is registered in `resources/icons/icons.qrc`. If missing, download it from Lucide (`https://raw.githubusercontent.com/lucide-icons/lucide/main/icons/<name>.svg`) and register it

## Verification

- Build succeeds at 100% — verify with `cmake --build build --parallel`
- No QML runtime errors on launch (check debug console for ReferenceError, TypeError)
- All dialogs open and close without console errors

## Child DOX Index

No children — all files organized under this directory.

### `components/`
Reusable UI primitives — SaikoButton, SaikoCheckBox, SaikoComboBox, SaikoTooltip, SaikoMenu, SaikoMenuItem. Owned by this doc.

### `shared/`
Shared singleton and utilities — Theme.qml (singleton), utils.js (JS utility functions). Owned by this doc.

### `dialogs/`
Standalone windows and popups — SettingsDialog, HotkeyDialog, RoutingDialog, ProcessSelectionPopup, AboutDialog. Owned by this doc.

### `recording/`
Recording panel sub-components — RecordingPanel, RecordingHeader, CaptureModeBar, ReplayBufferSection, TransportControls. Owned by this doc.

### `soundboard/`
Soundboard grid and card editor — SoundboardGridView, SoundboardGridCard, SlotEditor, HotkeyCard, and editor/ sub-components (SectionLabel, EditorHeader, SlotNameEditor, AudioSourceSelector, TrimEditor, VolumeSlider, RoutingSelector, HotkeyDisplay, ActionButtons). Owned by this doc.

### `sources/`
Audio source management — SourcesPanel (expandable card with live volume slider, mute/unmute toggle, solo monitor, responsive text elision), WaveformView. Owned by this doc.
