# SaikoSoundboard (v0.1.0 Beta)

**SaikoSoundboard** is a modern, low-latency Windows soundboard, audio recorder, and instant replay capture application built with **C++17**, **Qt 6 Quick**, and native **WASAPI** audio sub-systems.

Designed for content creators, streamers, and gamers, SaikoSoundboard provides real-time system and application audio capture, customizable hotkey triggering, audio trimming, per-slot device routing, and seamless audio replay buffers.

---

## 🌟 Key Features

- **🎙️ Flexible Audio Capture Modes**:
  - **System Output (Global)**: Capture clean Windows desktop audio via native WASAPI loopback.
  - **Multi-Track Application Capture**: Target individual running applications (Discord, Spotify, games, etc.) with custom mix controls, solo monitoring, and volume adjustments.
- **⚡ Instant Replay Buffer**:
  - Continuous background audio buffer (configurable duration).
  - One-click instant save to WAV with instant preview waveforms.
- **🎛️ Soundboard & Hotkey Dispatcher**:
  - Responsive soundboard grid with customizable audio slots.
  - Global Windows hotkey registration for instant sound triggering across applications.
  - Per-slot audio customization: name, file source, start/end trimming, volume gain, and output device routing.
- **📊 Real-time Waveform Display**:
  - Live hardware-accelerated audio waveform visualization during recording and playback.
- **🎨 Premium Dark Theme UI**:
  - Sleek, polished Qt Quick interface using dark mode design principles and responsive layouts.

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
3. Package the standalone bundle in `dist/SaikoSoundboard-v0.1.0-beta/`.
4. Build `SaikoSoundboard-0.1.0-Beta-Setup.exe` if Inno Setup Compiler (`iscc.exe`) is installed.

### 2. Inno Setup Installer

You can also compile the installer script manually using [Inno Setup 6](https://jrsoftware.org/isinfo.php):

```cmd
iscc installer/SaikoSoundboard.iss
```

---

## 🏗️ Architecture Overview

The codebase strictly adheres to clean architecture principles:

```
SaikoSoundboard/
├── src/core/domain/      # Pure C++ business logic (no Qt dependencies)
├── src/core/adapters/    # Platform adapters (Windows WASAPI, Hotkeys, Process Finder)
├── src/audio/            # WASAPI audio engine (Recorder, Mixer, ReplayBuffer, SoundPlayer)
├── src/managers/         # Orchestration layer (Action, Hotkey, Settings, Soundboard)
├── src/models/           # Qt Abstract Data Models
├── src/ui/               # C++ QmlBackend singleton & Waveform QQuickPaintedItems
├── src/qml/              # Qt Quick UI components, dialogs, and themes
├── installer/            # Windows installer configurations (Inno Setup)
└── tools/                # Release & deployment scripts, hardware verification tools
```

---

## 📜 License & Acknowledgments

Built with C++17, Qt 6 Quick, and native Windows Audio Session APIs (WASAPI).
© 2026 Saiko Interactive. Released under Beta license.
