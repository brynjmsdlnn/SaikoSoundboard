# Verification Tools

## Purpose

Manual verification executables for hardware- or system-dependent features that cannot be tested automatically. Each tool exercises a specific subsystem and requires human judgment to verify correctness (audio playback, hotkey registration, action dispatch).

## Ownership

- 3 verification tools: `SoundPlayerVerify`, `ActionManagerVerify`, `HotkeyManagerVerify`
- Controlled by CMake option `BUILD_VERIFICATION_TOOLS` (default ON)

## Local Contracts

- Use the `add_saiko_verification` CMake macro in `tools/CMakeLists.txt`
- Each tool links `SaikoSoundboardCore`
- Tools are NOT automated tests — they require manual launch and operator verification
- Keep tools focused on one subsystem each

## Verification

- Build: `cmake --build build --parallel` succeeds when `BUILD_VERIFICATION_TOOLS=ON`
- Manual: launch each `.exe` from `build/Desktop_Qt_6_11_1_MinGW_64_bit_Debug/` and verify output

## Child DOX Index

No children — all files are flat in this directory and owned by this doc.
