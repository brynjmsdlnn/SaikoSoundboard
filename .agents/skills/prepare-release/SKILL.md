---
name: prepare-release
description: Read recent git changes, update version configurations across the project, run packaging builds, and draft release notes.
---

# Prepare Release Skill

This skill outlines the step-by-step protocol for preparing a new project release, bumping version configurations across the workspace, compiling the distribution installer, and drafting comprehensive release notes.

## Step-by-Step Release Protocol

### Step 1: Identify Release Changes
Locate the start commit (usually the previous release version tag or release commit) and list all commits up to the current `HEAD` to extract new features, optimizations, bugfixes, and refactors:
```bash
git log --oneline <start-commit-sha>..HEAD
git log --format="%h - %s%n%b" <start-commit-sha>..HEAD
```

---

### Step 2: Version Configuration Checklist
Ensure the new version number is bumped consistently across all project files:
1. **CMake Build System**:
   - Update `project(SaikoSoundboard VERSION <new-version> LANGUAGES CXX)` in `CMakeLists.txt`.
2. **PowerShell Deployment Script**:
   - Update staging directory `$StagingDir = "$DistDir\SaikoSoundboard-v<new-version>-beta"` in `tools/deploy_release.ps1`.
   - Update zip path `$ZipPath = "$DistDir\SaikoSoundboard-v<new-version>-beta-portable.zip"`.
   - Update logging titles.
3. **Inno Setup Script**:
   - Update `#define MyAppVersion "<new-version>-Beta"` in `installer/SaikoSoundboard.iss`.
   - Update `#define SourceDir "..\dist\SaikoSoundboard-v<new-version>-beta"`.
   - Update `OutputBaseFilename=SaikoSoundboard-<new-version>-Beta-Setup`.
4. **QML UI Components**:
   - Update application version text in `src/qml/dialogs/AboutDialog.qml` (e.g. `v<new-version> Beta`).
   - Update header/TitleBar badge labels (e.g. `v<new-version>`) in `src/qml/components/TitleBar.qml`.

---

### Step 3: Run Packaging & Compiler Validation
1. **Clean Stale Caches**:
   - Always delete the release build directory to ensure CMake refreshes its compiler path and tool checks (like Ninja paths):
     ```powershell
     Remove-Item -Recurse -Force build_release
     ```
2. **Execute Deploy Script**:
   - Run the automated deployment script to configure CMake, build release targets, execute `windeployqt`, optimize folder sizes, and compile the setup installer:
     ```powershell
     & powershell -File tools/deploy_release.ps1
     ```
3. **Verify Installer & Archives**:
   - Verify that the Setup wizard executable and portable ZIP are created in `dist/`.
   - Run the compliance check target to verify architecture and guidelines:
     ```powershell
     cmake --build build --target verify-project
     ```

---

### Step 4: Draft Release Notes
Generate release notes using the standard template:

1. **🌟 Key Features (General)**:
   - High-level overview of core application features (system capture, multitrack, replay buffer, hotkeys, waveforms, dark UI).
2. **🆕 What's New in v<version>-beta**:
   - Categorized details of features added in this release range:
     - **🎙️ Isolated Device Loopback & Headphones Monitoring**
     - **🖥️ Custom DPI-Aware Frameless Window**
     - **📊 Structured Logging Engine**
     - **📁 Centralized Storage Paths**
     - **💬 Consolidated Frameless Overlays**
     - **🧪 Codebase Verification**
     - **🐛 Key Bugfixes**
3. **📦 Download Artifacts**:
   - Links to setup installer and portable zip filenames.
