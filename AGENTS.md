# DOX framework

- DOX is highly performant AGENTS.md hierarchy installed here
- Agent must follow DOX instructions across any edits

## Core Contract

- AGENTS.md files are binding work contracts for their subtrees
- Work products, source materials, instructions, records, assets, and durable docs must stay understandable from the nearest applicable AGENTS.md plus every parent AGENTS.md above it

## Read Before Editing

1. Read the root AGENTS.md
2. Identify every file or folder you expect to touch
3. Walk from the repository root to each target path
4. Read every AGENTS.md found along each route
5. If a parent AGENTS.md lists a child AGENTS.md whose scope contains the path, read that child and continue from there
6. Use the nearest AGENTS.md as the local contract and parent docs for repo-wide rules
7. If docs conflict, the closer doc controls local work details, but no child doc may weaken DOX

Do not rely on memory. Re-read the applicable DOX chain in the current session before editing.

## Update After Editing

Every meaningful change requires a DOX pass before the task is done.

Update the closest owning AGENTS.md when a change affects:

- purpose, scope, ownership, or responsibilities
- durable structure, contracts, workflows, or operating rules
- required inputs, outputs, permissions, constraints, side effects, or artifacts
- user preferences about behavior, communication, process, organization, or quality
- AGENTS.md creation, deletion, move, rename, or index contents

Update parent docs when parent-level structure, ownership, workflow, or child index changes. Update child docs when parent changes alter local rules. Remove stale or contradictory text immediately. Small edits that do not change behavior or contracts may leave docs unchanged, but the DOX pass still must happen.

## Hierarchy

- Root AGENTS.md is the DOX rail: project-wide instructions, global preferences, durable workflow rules, and the top-level Child DOX Index
- Child AGENTS.md files own domain-specific instructions and their own Child DOX Index
- Each parent explains what its direct children cover and what stays owned by the parent
- The closer a doc is to the work, the more specific and practical it must be

## Child Doc Shape

- Create a child AGENTS.md when a folder becomes a durable boundary with its own purpose, rules, responsibilities, workflow, materials, or quality standards
- Work Guidance must reflect the current standards of the project or user instructions; if there are no specific standards or instructions yet, leave it empty
- Verification must reflect an existing check; if no verification framework exists yet, leave it empty and update it when one exists

Default section order:
- Purpose
- Ownership
- Local Contracts
- Work Guidance
- Verification
- Child DOX Index

## Style

- Keep docs concise, current, and operational
- Document stable contracts, not diary entries
- Put broad rules in parent docs and concrete details in child docs
- Prefer direct bullets with explicit names
- Do not duplicate rules across many files unless each scope needs a local version
- Delete stale notes instead of explaining history
- Trim obvious statements, repeated rules, misplaced detail, and warnings for risks that no longer exist

## Closeout

1. Re-check changed paths against the DOX chain
2. Update nearest owning docs and any affected parents or children
3. Refresh every affected Child DOX Index
4. Remove stale or contradictory text
5. Run existing verification when relevant
6. Report any docs intentionally left unchanged and why

## User Preferences

- Build system: MinGW 13.1, Qt 6.11.1 mingw_64, CMake with `Ninja` generator (preferred) or `MinGW Makefiles`
- CMake prefix path: `C:\Qt\6.11.1\mingw_64`
- Build directory: `build/` (out-of-source)
- Entry point: `src/qml/Main.qml` loaded via `qrc:/qt/qml/Saiko/src/qml/Main.qml`
- QML module URI: `Saiko` version `1.0` (C++ types registered as `Saiko 1.0`)
- C++ standard: C++17 with `-std=gnu++17`
- `QT_QUICK_CONTROLS_STYLE = "Basic"` set at startup
- `CMAKE_AUTOMOC_COMPILER_PREDEFINES OFF` required for MinGW 13.1
- `CMAKE_CXX_COMPILER_LAUNCHER` set to `ccache` when available (configured in CMakeLists.txt)

## Child DOX Index

### `src/qml/`
QML UI layer — 20 QML files + 1 JS utility. All part of the `Saiko 1.0` module. Uses `Theme` singleton, `SaikoButton`, `utils.js`, `AboutDialog`. See `src/qml/AGENTS.md` for conventions.

### `src/logging/`
Logging subsystem — Logger singleton, ConsoleSink, FileSink (session log with QElapsedTimer timing), LogModel (QML-bound log buffer), LogRecord, LogLevel, LogCategory, LogMacros. See `src/logging/AGENTS.md` for architecture and lifecycle.

### `src/core/`
Core layer — pure business logic in `domain/` (no Qt, no platform deps) and platform adapters in `adapters/` (Windows API). See `src/core/AGENTS.md` for purity rules.

### `src/storage/`
Filesystem path utility — stateless `StoragePaths` class for all path construction (settings, recordings, replays, temp files, logs). No file I/O. See `src/storage/AGENTS.md` for contracts.

### `src/platform/`
Platform-specific windowing — `WindowMetrics.h` (cross-platform named constants for frameless window dimensions), `windows/WindowsFramelessWindow.h/.cpp` (Win32 WM_NCHITTEST/WM_NCCALCSIZE/WM_GETMINMAXINFO hit-testing + DPI scaling). Owned by root.

### `src/audio/`
Audio engine — mixer, WASAPI recorder, WASAPI passthrough, sound player, replay buffer, waveform generator, WAV writer. Standard C++ with Qt Multimedia and WASAPI. No child AGENTS.md — owned by root.

### `src/managers/`
Orchestration layer — ActionManager, HotkeyManager, RecordingManager, SettingsManager, SoundboardManager. Bridges domain logic to UI. No child AGENTS.md — owned by root.

### `src/models/`
Data models — `QAbstractListModel` subclasses (`SoundPlayerSlotModel`, `AudioSourceListModel`) and value types (`AudioSource`, `SoundPlayerSlot`, `CaptureState`). No child AGENTS.md — owned by root.

### `src/ui/`
C++ UI backend — QmlBackend (singleton exposed to QML), WaveformItem (QQuickPaintedItem), RealtimeWaveformItem. No child AGENTS.md — owned by root.

### `tests/`
Automated test suite — unit tests in `tests/unit/`, integration tests in `tests/integration/`. CTest + Qt Test framework. See `tests/AGENTS.md` for registration conventions.

### `tools/`
Manual verification tools — SoundPlayerVerify, ActionManagerVerify, HotkeyManagerVerify, plus `deploy_release.ps1` release packaging script. See `tools/AGENTS.md` for conventions.

### `installer/`
Windows installer configuration — `SaikoSoundboard.iss` Inno Setup script for building standalone setup installers. No child AGENTS.md — owned by root.