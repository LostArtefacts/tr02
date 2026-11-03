# tr02

A matching decompilation of Tomb Raider II (Core Design, 1997), the
Steam Windows release, built and verified on Linux through Wine.

The goal is C and C++ source that compiles into the original machine
code. During development, a DLL replaces recovered functions in a copy
of the original executable, so they can be exercised in the game.

![Progress](docs/progress.svg)

## Sources

This is not a clean-room project. The references are:

- `refs/core`: Core's 1997 source tree, which continued towards Tomb
  Raider III. `BACKUP` is generally closer to the released TR2 code.
- [TRX](https://github.com/LostArtefacts/TRX): the target's function and
  global names, addresses and types in `docs/symbols/tr2-steam.txt`.
- [TR2Main](https://github.com/Arsunt/TR2Main): a decompilation that hooks
  into the same executable, function by function.

Ghidra provides disassembly and pseudocode for the original executable.

## Target

The Steam release's `Tomb2.exe`, linked 1997-12-10, a patch after the
October retail release. TRX's Steam manifest targets this build.

| Property | Value |
| --- | --- |
| Format | 32-bit PE |
| Image base | `0x400000` |
| Entry point | `0x459ff0` |
| Text size | `0x61dba` bytes |
| Data size | `0xc3514` in memory, `0x8400` on disk |
| Game code | `0x401000`–`0x459030` |

The game ends with `3DSYSTEM`'s hand-written assembly, from
`phd_PushMatrix` through `phd_sqrt`; the statically linked C library
follows. Imports include DirectInput, DirectSound, DirectDraw, WinMM,
Windows system libraries and Eidos' `WINPLAY.DLL` FMV player.

Game files are not distributed. Supply a copy at `orig/tomb2`; see
[`orig/README.md`](orig/README.md).

## Toolchain

Visual C++ 5.0 SP3 with the DirectX 5 SDK, run under Wine.

| Component | Version or configuration |
| --- | --- |
| Compiler driver (`cl.exe`) | 11.00.7022 |
| C frontend (`c1.dll`) | 11.00.7113 |
| C++ frontend (`c1xx.dll`) | 11.00.7303 |
| Optimizer (`c2.exe`) | 11.00.7303 |
| DLL linker | 5.10.7303, VC5 SP3 |
| Standalone linker | 5.00.7022, VC5 RTM |
| DirectX SDK | DirectX 5, July 1997 |
| Release flags | `/G5 /Zp1 /W3 /O2` |

The original has no Rich header and reports linker version 5.02.
`TOMBRAID.DSP` is a Visual C++ 5.00 project; its release flags select
Pentium scheduling, byte-packed structures and speed optimizations.
The compiler reproduces the original code. The standalone build uses
the RTM linker for its import-section layout; this is distinct from
identifying the original linker's exact build.

Core's project contains C game modules and C++ platform/rendering
modules. Shared headers use `extern "C"`, so most function symbols have
C decoration such as `_Name`. Build configuration lives in `project.toml`.

## Setup

Clone with submodules, or run `git submodule update --init`.
Requirements: Python 3.11+, `just`, Wine, Ghidra and Java 21. Running the
game headlessly also needs Xvfb, xdotool and ImageMagick.

Place local reference directories or symlinks at `refs/core`, `refs/trx`
and `refs/tr2main`. Initialize the prefix with
`WINEPREFIX="$PWD/.wine" wineboot -i` and supply these directories:

| Wine path | Contents |
| --- | --- |
| `C:\msvc5sp3` | VC5 SP3 `bin`, `include` and `lib` |
| `C:\msvc5` | VC5 RTM tools used by the standalone linker |
| `C:\dx5` | DirectX 5 SDK `include` and `lib` |

Game files, references, `.wine/` and `build/` are ignored by Git.
See `orig/README.md` for toolchain media and supporting DLLs.
Set `GHIDRA` if its installation is outside the configured search paths.

## Workflow

| Command | Purpose |
| --- | --- |
| `just analyze` | Analyze the original and record function sizes |
| `just manifest` | Apply names and types from TRX's symbol manifest |
| `just decompile ADDRESS` | Export Ghidra pseudocode |
| `just fn NAME` | Look up a function in the reference source |
| `just compare [NAME]` | Build and check byte parity; optionally show a function diff |
| `just progress` | Rebuild the progress chart |
| `just game` | Prepare the game with the replacement DLL |
| `just run wait:10 key:Return wait:2 shot:menu` | Run under Wine and capture a screenshot |
| `just exe` | Link the standalone executable |
| `just compare-exe` | Compare the standalone executable with the original |

Write recovered functions in `src/`, following the original module
layout. Record addresses and decorated symbols in `config/symbols.csv`:
`_Name` for C, `@Name@8` for `__fastcall`, `_Name@8` for `__stdcall`.
The DLL's generated hook table redirects compiled functions, while other
symbols resolve to the original executable's addresses.

Use one newly recovered function per commit, with a subject such as
`Decompile NAME (0xADDRESS)`. Keep byte parity and the progress chart
current; intentional differences are listed in `hooks.allow_diff` in
`project.toml`.

The build tools are a submodule of
[decomp-tools](https://github.com/LostArtefacts/decomp-tools), shared with
other Tomb Raider decompilations. `just` lists all available commands.
