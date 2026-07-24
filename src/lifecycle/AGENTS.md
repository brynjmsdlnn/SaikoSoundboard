# Application Lifecycle Layer

## Purpose

Application lifetime coordination, system tray icon management, window state control, and application shutdown orchestration for Saiko Soundboard.

## Ownership

- `ApplicationLifecycleManager.h` & `.cpp` — Core manager for application states, system tray, window restoration, and idempotent teardown sequence.
- Owned by this doc.

## Local Contracts

- **Lifetime Independence**: The application's lifetime is completely independent of the visibility of its windows.
- **State Control**: `ApplicationState` transitions occur exclusively inside `ApplicationLifecycleManager`.
- **Termination Control**: Never call `QCoreApplication::quit()` or `Qt.quit()` directly outside `ApplicationLifecycleManager`. All application termination must flow through `exitApplication()`.
- **Idempotency**: `exitApplication()` must be strictly idempotent.
- **Window Ownership**: `ApplicationLifecycleManager` holds a non-owning reference to `QQuickWindow` via `attachMainWindow(QQuickWindow*)`. It does not delete or destroy window pointers.
- **Tray Icon Ownership**: `ApplicationLifecycleManager` owns `QSystemTrayIcon` and its context `QMenu` / `QAction` objects.

## Work Guidance

- When modifying application shutdown, ensure every subsystem (recording, replay buffer, playback, hotkeys, settings) is safely flushed and stopped in the canonical teardown order before `QCoreApplication::quit()`.

## Verification

- Build succeeds at 100% — verify with `cmake --build build --parallel`
- Automated tests pass.

## Child DOX Index

No children — all files organized directly under `src/lifecycle/`.
