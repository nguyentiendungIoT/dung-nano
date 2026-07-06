# Repository Guidelines

## Project Structure & Module Organization

This is a PlatformIO Arduino firmware project for `nanoatmega328`.

- `platformio.ini` defines the target board, framework, and build environment.
- `src/` contains firmware source files. `src/main.cpp` is the current entry point.
- `include/` is for shared project headers, such as pin maps, constants, and function declarations.
- `lib/` is for project-specific libraries that should not be installed globally.
- `test/` is reserved for PlatformIO unit tests.
- `.pio/` is generated build output and should not be edited by hand.
- `.vscode/` contains local editor integration for PlatformIO and C/C++ IntelliSense.

## Build, Test, and Development Commands

Use the standard PlatformIO command if it is on `PATH`:

```powershell
pio run -e nanoatmega328
pio test -e nanoatmega328
pio run -e nanoatmega328 -t upload
pio device monitor
```

On this machine, `pio` may not be on `PATH`; use the local executable instead:

```powershell
C:\Users\tiend\.platformio\penv\Scripts\pio.exe run -e nanoatmega328
```

`pio run` compiles the firmware, `pio test` runs PlatformIO tests, `-t upload`
flashes the board, and `pio device monitor` opens the serial monitor.

## Coding Style & Naming Conventions

Use Arduino C++ with two-space indentation and keep lines under 120 characters.
Prefer small helper functions over long `setup()` or `loop()` bodies. Put shared
constants and declarations in `include/*.h`, and implementation code in `src/*.cpp`.

Use `snake_case` for variables and functions, `PascalCase` for classes, and
`UPPER_SNAKE` for constants/macros. Avoid commented-out code and unused imports.
Comments should explain hardware intent, timing assumptions, or protocol details.

## Testing Guidelines

Place PlatformIO tests under `test/`. Name test files by behavior, for example
`test_serial_protocol.cpp` or `test_motor_limits.cpp`. For firmware logic that can
be isolated from hardware, prefer unit tests over manual board-only checks. Always
run a full compile before handing off changes.

## Commit & Pull Request Guidelines

No local Git history is available in this checkout, so use clear imperative commit
messages such as `Add serial command parser` or `Fix motor timeout handling`.

Pull requests should include a short summary, tested command output, affected
hardware or board revision, and any serial protocol or pin mapping changes. Attach
logs or screenshots when behavior is verified through the serial monitor.

## Agent-Specific Instructions

Do not overwrite generated PlatformIO files in `.pio/`. Keep changes focused on
source, headers, tests, and project configuration. Verify with PlatformIO whenever
the change can affect compilation or firmware behavior.
