# Tests

## Purpose

Automated test suite for SaikoSoundboard. Runs via CTest and Qt Test framework.

## Ownership

- `unit/` — Unit tests (isolated component tests, no external dependencies)
- `integration/` — Integration tests (cross-component, real backends)

## Local Contracts

- Test files use the `add_saiko_unit_test` / `add_saiko_integration_test` CMake macros defined in `tests/CMakeLists.txt`
- All tests link `SaikoSoundboardCore` + `Qt::Test`
- Unit tests go in `tests/unit/`, integration tests in `tests/integration/`
- New test CMake registration happens in `tests/CMakeLists.txt` using the appropriate macro
- Tests use `QT_TEST` macros for assertions, not custom assert frameworks

## Verification

- `ctest --test-dir build` runs all registered tests
- Individual tests can be run: `build/Desktop_Qt_6_11_1_MinGW_64_bit_Debug/<TestName>.exe`

## Child DOX Index

- `unit/` — Isolated unit tests for single components
- `integration/` — Cross-component integration tests
