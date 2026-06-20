# Core Layer

## Purpose

Pure business logic and platform-specific adapters. This is the foundation of the architecture — all higher layers (managers, UI) depend on it. It must never depend on them.

## Ownership

- `domain/` — Pure C++ entities and data structures (no Qt, no platform APIs)
- `adapters/` — Platform-specific implementations (Windows API, COM)

## Local Contracts

- **Domain layer (`domain/`)**: MUST remain pure. No Qt includes, no platform APIs, no dependency on managers, UI, or adapters. Uses only standard C++17.
- **Adapters layer (`adapters/`)**: MAY use platform APIs (Win32, COM) and Qt wrappers. MUST NOT depend on managers or UI.
- No file outside `core/` may have a circular dependency back into core.
- Header-only domain types are preferred; `.cpp` files in domain are for non-trivial logic only.

## Verification

- Static analysis: no includes from managers/, ui/, or audio/ in domain/ files
- Build succeeds: verify with `cmake --build build --parallel`

## Child DOX Index

- `domain/` — Pure business logic. No Qt, no platform deps.
- `adapters/` — Platform-specific code. May use Qt + Win32.
