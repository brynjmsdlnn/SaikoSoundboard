# SaikoSoundboard (v0.4.0 Beta)

**SaikoSoundboard** is a modern, low-latency Windows soundboard, audio recorder, and instant replay capture application built with **C++17**, **Qt 6 Quick**, and native **WASAPI** audio sub-systems.

Designed for content creators, streamers, and gamers, SaikoSoundboard provides real-time system and application audio capture, customizable hotkey triggering, audio trimming, per-slot device routing, and seamless audio replay buffers.

---

## 🌟 Key Features

- **🎙️ Flexible Audio Capture & Instant Replay**:
  - **System Output (Global)**: Capture clean Windows desktop audio via native WASAPI loopback.
  - **Multi-Track Application Capture**: Target individual running processes (Discord, Spotify, games, etc.) with custom mix controls, solo monitoring, and volume adjustments.
  - **Isolated Virtual Device Loopback**: Captures virtual audio cables (e.g. VB-Cable) as isolated tracks.
  - **Headphones Monitoring (Passthrough)**: Explicit monitoring controls ("Listen" button) to route isolated virtual device audio to headphones with dynamic volume scaling.
  - **Instant Replay Buffer**: Continuous background audio buffer with configurable duration, one-click save to WAV, and a hover-revealed circular power toggle featuring Canvas ripple animations.
  - **Replay Duration Editor**: Standalone editor with cross-fade displays, press-and-hold repeat stepping, and soft-cap limit validation.
- **🎛️ Soundboard & Playback Modes**:
  - **Configurable Playback Modes**: Choose between 6 slot-level modes (Default, Restart/Retrigger, Toggle/Stop, Queued Sequential, Layered/Cut All, and Layered/Ring Out).
  - **Slot Assignment ("Assign to Slot")**: Easily assign active recording or replay playback files to any soundboard grid slot via a dedicated selection dialog.
  - **Interactive Waveform Display**: Live waveform rendering during recording and playback, featuring type-safe visual distinction (green for replays, purple for manual recordings), and multi-track playhead overlays.
  - **Detailed Status Indicators**: Real-time loop count indicators, playback countdown timers, active queue badges, and marquee text scrolling.
- **⚡ Advanced Hotkey Dispatcher**:
  - **Numpad Key Support**: Register top-row digits and numpad digits (`VK_NUMPAD0`–`VK_NUMPAD9`) as distinct global Windows hotkeys.
  - **Hotkey Capture Lifecycle**: Duplicate keybinding detection, timeout recovery, and real-time unsaved changes warning states.
- **🎨 Custom Frameless Window & Theme UI**:
  - **DPI-Aware Frameless Windowing**: Win32 custom hit-testing (`WM_NCHITTEST` / `WM_NCCALCSIZE`), drag-to-move, and double-click to maximize with transparent resize borders.
  - **Native Title Bar Controls**: Clamped work-area maximization on multi-monitor setups, native Segoe MDL2 iconography, and utility shortcuts (Settings, About, Log Viewer).
  - **Performance-Optimized Rendering**: Automatically pauses idle visual effects and card pulse animations during window resize/move operations to prevent interface lag.
  - **SaikoFramelessPopup & SaikoDialog overlays**: Consolidated dialog layouts with backdrop dimming, entry/exit scale transitions, tabbed Settings views, and contextual warning alerts (hotkey conflicts, unsaved files).
- **📊 Real-time Logging & Storage System**:
  - **Structured Logging Engine**: Thread-safe `SaikoLogging` static library with formatted `ConsoleSink` redirects and disk session `FileSink` persistence.
  - **In-App Log Viewer**: Sortable and filterable real-time Log Viewer console overlay unlocked via developer mode easter egg.
  - **Centralized Storage paths**: Stateless `SaikoStorage` library containing centralized path resolution, and automatic file cleanup of rejected audio recordings.

---

## 🖥️ System Requirements

- **Operating System**: Windows 10 / Windows 11 (64-bit)
- **Audio Subsystem**: WASAPI compatible sound device
- **Build Requirements** (for building from source):
  - Qt 6.11.1 (`mingw_64`)
  - MinGW 13.1 (64-bit) / GCC C++17 compiler
  - CMake 3.16 or newer
  - Ninja build generator (recommended)

---

## 🔨 Building from Source

To compile SaikoSoundboard locally:

```powershell
# Clone repository
git clone https://github.com/your-username/SaikoSoundboard.git
cd SaikoSoundboard

# Configure CMake build directory
cmake -B build -G "Ninja" `
  -DCMAKE_PREFIX_PATH="C:/Qt/6.11.1/mingw_64" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_AUTOMOC_COMPILER_PREDEFINES=OFF

# Build executable
cmake --build build --config Release
```

The compiled executable will be located at `build/SaikoSoundboard.exe`.

---

## 🧪 Running Tests & Verification Tools

SaikoSoundboard includes automated tests and manual hardware verification utilities:

```powershell
# Run automated CTest suite
ctest --test-dir build --output-on-failure

# Run automated architecture and guidelines compliance check
cmake --build build --target verify-project

# Launch hardware verification tools (when BUILD_VERIFICATION_TOOLS=ON)
.\build\tools\SoundPlayerVerify.exe
.\build\tools\HotkeyManagerVerify.exe
.\build\tools\ActionManagerVerify.exe
```

---

## 📦 Packaging & Beta Release

To build a standalone distribution folder and Windows installer executable:

### 1. Automated Packaging Script

Run the provided PowerShell deployment script:

```powershell
.\tools\deploy_release.ps1
```

This script will:
1. Compile SaikoSoundboard in `Release` mode.
2. Run `windeployqt` to bundle required Qt libraries and QML modules.
3. Package the standalone bundle in `dist/SaikoSoundboard-v0.4.0-beta/`.
4. Build `SaikoSoundboard-0.4.0-Beta-Setup.exe` if Inno Setup Compiler (`iscc.exe`) is installed.

### 2. Inno Setup Installer

You can also compile the installer script manually using [Inno Setup 6](https://jrsoftware.org/isinfo.php):

```cmd
iscc installer/SaikoSoundboard.iss
```

---

## 🏗️ Architecture Overview

The codebase strictly adheres to clean architecture principles, modularized into static library targets linked under `SaikoSoundboardCore`:

```
SaikoSoundboard/
├── src/core/domain/      # SaikoDomain: Pure C++ business logic (no Qt or platform dependencies)
├── src/core/adapters/    # SaikoAdapters: Native Windows platform integrations (API, hotkeys, process muting)
├── src/audio/            # SaikoAudio: WASAPI audio engine (recorder, mixer, replay buffer, WAV writer)
├── src/logging/          # SaikoLogging: Structured thread-safe logging with file and console sinks
├── src/storage/          # SaikoStorage: Centralized stateless application directory and path resolver
├── src/managers/         # SaikoManagers: Orchestration layer managing slots, hotkeys, settings, and recordings
├── src/models/           # SaikoModels: Qt QAbstractListModel implementations (slots, active devices)
├── src/ui/               # SaikoUI: C++ QmlBackend singleton and Waveform quick items
├── src/qml/              # UI representation: Qt Quick files, themes, popups, and components
├── installer/            # Standalone Windows installer configuration (Inno Setup)
└── tools/                # Release packaging script and compliance check scripts (verify_project.ps1)
```

### ⚡ Build Performance Optimizations
To support rapid iterations and fast build times under **MinGW 13.1** and **CMake**:
- **Precompiled Headers (PCH)**: Heavy common headers (Qt, standard libraries, Win32 APIs) are precompiled via `COMMON_PCH_HEADERS` to eliminate compile cascades.
- **Unity Builds**: Source files in compilation-heavy libraries (`SaikoAudio` and `SaikoManagers`) are grouped into single compilation units to optimize link times and compile passes.

---

## 📜 License & Acknowledgments

Built with C++17, Qt 6 Quick, and native Windows Audio Session APIs (WASAPI).
© 2026 Saiko Interactive. Released under Beta license.
