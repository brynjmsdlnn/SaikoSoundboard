# Storage Subsystem

## Purpose

Single source of truth for all filesystem path construction in Saiko Soundboard. Provides stateless utility functions that every other subsystem (audio, logging, managers, UI) uses to resolve file locations without hardcoded paths.

## Ownership

- `StoragePaths.h/.cpp` — All-static class with no mutable state. Methods for settings paths, user-visible directories (recordings, replays), temporary file paths under a dedicated temp directory, and log storage location.

## Local Contracts

- **All methods are static** — no instances, no state.
- **Never hardcode filesystem paths** — always use `StoragePaths` methods.
- **No file I/O** — only path construction and directory creation (`QDir::mkpath`).
- **No knowledge of audio, logging, or UI** — purely a filesystem path utility.
- **Directory creation** is a side effect of `temporaryDirectory()` and `settingsFilePath()` (both need the parent directory to exist). Other path methods like `logDirectory()` return paths without creating them — the caller is responsible.
- **Portable-mode detection** (future) should be implemented here and only here.

## Verification

- Build succeeds: `cmake --build build --target SaikoStorage`
- All path methods return expected values matching the patterns documented in the header

## Child DOX Index

No children — all files organized under this directory.
