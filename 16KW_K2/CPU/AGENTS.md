# AGENTS.md

## Project Overview
- TI C2000 DSP firmware for a 16kW PCS (Power Conversion System) inverter
- Target: TMS320F28375D (dual-core C28x + CLA co-processor)
- Authored by Xi'an NovaStar Tech - comments/specs in Chinese
- **No README, no CI, no lint, no tests** - this is a bare CCS project

## File Encryption - CRITICAL
- **All `.c` and `.h` source files are encrypted** (TI DSP toolchain COFF/hex format with `%TSD-Header-###%` marker).
  - File sizes are typically 32,768 bytes (exactly 32KB), regardless of actual code length.
  - Chinese characters are present throughout (Doxygen-style headers, inline comments).
- **Read tool will NOT work** - it reports these as "binary" and refuses to read.
- **=== Commands that WORK ===**
  - `head -n N file.c` — stream raw bytes from the start (best method)
  - `sed -n "M,Np" file.c` — extract specific line ranges
- **=== Commands that FAIL (all produce garbled output) ===**
  - `type file.c` — Windows cmd built-in (triggers encoding detection)
  - `Get-Content file.c` — PowerShell (all encodings fail: UTF8, Default, Byte)
  - `Get-Content -Tail N file.c` — PowerShell tail mode
  - `Select-String -Pattern "..." file.c` — PowerShell grep (returns nothing)
  - `findstr /n "..." file.c` — Windows cmd grep (returns nothing)
  - `[System.IO.File]::ReadAllText()` — .NET method
  - `iconv` — encoding conversion fails
  - `more file.c` — Windows more pager
  - `tail -n N file.c` — reads from file end (32KB tail padding is garbled)
- **Practical consequence**: Searching/grepping source code is impossible on encrypted files.
  Only streaming via `head`/`sed` can read their raw content.
- When writing to files, preserve the original encoding - do NOT convert between GBK and UTF-8 unless explicitly requested.

## Build System
- **Code Composer Studio (CCS) IDE only** - cannot be built from command line
- Compiler: TI C2000 v22.6.1.LTS, COFF ABI (not EABI), C99 dialect
- Two active build configurations (in `.cproject`):
  - `FLASH` (primary): defines `CPU1` + `_FLASH`, linker `cmd/2837xD_FLASH_lnk_cpu1.cmd`
  - `RAM`: defines `CPU1` only, linker `cmd/2837xD_RAM_lnk_cpu1.cmd`
- Post-build step (FLASH only): `tool/fill` converts `.out` → `.bin` via ofd2000 + hex2000 + mkhex4bin
- Clean build artifacts (Windows): `keilkilll.bat`

## Directory Structure
| Dir | Purpose |
|-----|---------|
| `app/` | Application layer (main, state machine, modbus, fault, fan, relay, firmware upgrade, etc.) |
| `algo/` | Control algorithms (power compute `*.c`, CLA co-processor tasks `*.cla`, product specs) |
| `driver/` | Hardware drivers (ADC, PWM, SCI, SPI, CAN, GPIO, timers, interrupt) |
| `device/` | Device-specific drivers (CH390 Ethernet, SPI flash, LED, NTC thermistor) |
| `middleware/` | CRC, queue, communication service |
| `source/` | TI-provided F2837xD peripheral library source code |
| `include/` | TI-provided F2837xD peripheral library headers + `F28x_Project.h` |
| `cmd/` | Linker command files (`.cmd`) |
| `tool/` | Post-build utilities (hex2000.exe, mkhex4bin.exe, fill.bat, tiobj2bin.bat) |
| `FLASH/` | Build output directory (git-ignored) |

## Key Conventions
- Every `.c` has a matching `.h` in the same directory
- CLA co-processor files (`.cla`) run on the Control Law Accelerator, compiled with different flags
- Preprocessor guard: `#ifdef _FLASH` distinguishes flash vs RAM behavior
- Linked external repo at `Z:/wsl-proj/Algorithm_Framework` (algorithm framework, mounted via WSL)
- Debug probe: XDS100v3 USB, configured in `TMS320F28375D.ccxml`
- Structured file headers with Doxygen-like `@copyright @file @author @version @date @brief` tags (in Chinese)

## Editing Rules
- Do not add English comments to existing files - repo convention is Chinese
- Do not modify `.cproject`, `.ccsproject`, or `.project` metadata unless specifically requested
- Do not commit `FLASH/` directory content (git-ignored)
- Preserve original file encoding when editing
- `.clangd` config is minimal and non-functional for real intellisense - do not rely on it
