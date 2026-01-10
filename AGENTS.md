# Repository Guidelines

## Project Structure & Module Organization

- `src/`: Application sources (`src/main.c`).
- `boards/`: Board-specific overlays (e.g., `boards/nrf54l15dk_nrf54l15_cpuapp.overlay`).
- `CMakeLists.txt`: Zephyr app build entry; adds `src/main.c`.
- `prj.conf`: Kconfig options for the app (display, GPIO, SPI, ST7789V).
- `west.yml`: Zephyr workspace manifest pinned to Zephyr v4.2.0.

## Build, Test, and Development Commands

- `west init -l .` and `west update`: Initialize and fetch Zephyr dependencies
from `west.yml`.
- `west build -b nrf54l15dk/nrf54l15/cpuapp`: Configure and build the app for
the default board.
- `west build -p always -b nrf54l15dk/nrf54l15/cpuapp`: Clean rebuild when
changing Kconfig or overlays.
- `west flash`: Flash the last build to the connected board.

## Coding Style & Naming Conventions

- Language: C (Zephyr APIs). Use 4-space indentation and K&R-style braces as in `src/main.c`.
- Naming: follow Zephyr conventions (e.g., `snake_case` for locals, `CONFIG_*`
for Kconfig).
- Keep includes minimal and prefer Zephyr headers (`<zephyr/...>`).

## Testing Guidelines

- No automated tests are defined in this repository.
- If you add tests, document the framework and provide a `west test` or `ctest`
command in this file.

## Commit & Pull Request Guidelines

- Commit messages in history use a Zephyr-style prefix when relevant, e.g.,
`boards: nrf54l15dk: add st7789 display`.
- Prefer concise, imperative summaries; include a scope prefix when touching
a specific area.
- Pull requests should include: a short summary, hardware/board tested,
and any relevant logs or screenshots.

## Configuration Tips

- This repository uses a T2 topology layout.
- The default board is set in `CMakeLists.txt` via `set(BOARD ...)`.
- Display configuration is driven by `prj.conf` and the board overlay in `boards/`.
- Zephyr SDK is available one directory up from this repository (`../zephyr`).
- DK board VDD is set to 3.3V
