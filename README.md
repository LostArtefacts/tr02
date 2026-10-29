# tr02

A matching decompilation of Tomb Raider II (Core Design, 1997), the
Windows version, built and verified on Linux through Wine.

The goal is C and C++ source that compiles, with the original
toolchain, into a binary that is byte-for-byte identical to the
original executable.

This repository contains no game code or data. You need your own copy of
the game, see [`orig/README.md`](orig/README.md).

## Sources

This is not a clean-room project. Three references are used for names,
structures and logic:

- Core's own TR2 source tree, from 1997. It is a working tree, which
  went on towards Tomb Raider III, not the released build.
- [TRX](https://github.com/LostArtefacts/TRX), whose symbol manifest
  `docs/symbols/tr2-steam.txt` names the target's functions and
  globals, with types.
- [TR2Main](https://github.com/Arsunt/TR2Main), a decompilation that
  hooks into this executable, function by function.

![Progress](docs/progress.svg)

`just progress` renders it: every game function, by address and as a
treemap by size, coloured by whether it is decompiled and matching.

## Target

The Steam release's `Tomb2.exe`, linked 1997-12-10, a patch after the
October retail release. TRX's manifest is for this build.

- 32-bit PE, image base `0x400000`, entry point `0x459ff0`, `.text` is
  0x61dba bytes
- `.data` is 0xc3514 bytes in memory but only 0x8400 on disk, so most of
  it is zero-initialized globals
- Imports `DINPUT`, `DSOUND`, `DDRAW`, `WINMM`, `USER32`, `COMCTL32`,
  `ADVAPI32`, `GDI32`, `winplay` (Eidos' FMV player, shipped as
  `WINPLAY.DLL`) and `KERNEL32`, in the order of `TOMBRAID.DSP`'s
  libraries

The game code comes first, at `0x401000`–`0x459030`. It ends with
`3DSYSTEM`'s hand-written assembly (`phd_PushMatrix` to `phd_sqrt`),
and the statically linked C library follows.
Ghidra finds 1247 functions in the game code, 338,304 bytes; with
TRX's manifest all but 123 of them have names.

```sh
just analyze    # writes build/functions_tomb2.csv, seeds config/symbols.csv
just manifest   # names its rows from TRX's manifest
```

## Toolchain

Visual C++ 5.0 with the DirectX 5 SDK:

- The executable has no Rich header, and its linker version is 5.02:
  Visual C++ 5.0 (Visual Studio 97), whose own linker is 5.00, after
  an update. The compiler, `cl` 11.00.7022, matches the game's code.
- Core's `TOMBRAID.DSP` is a Developer Studio 5.00 project.
- The DirectX 5 SDK is from July 1997. The game uses `IDirect3D2` and
  `IDirectDrawSurface3`, which DirectX 5 introduced.

Its release configuration compiles with `/G5 /Zp1 /W3 /O2`: Pentium
scheduling, byte-packed structures and speed optimizations. The DSP
builds 78 C files (`GAME`) and 33 C++ files (`SPECIFIC`, `3DSYSTEM`).
The shared headers declare their functions `extern "C"`, so most
symbols are undecorated C names (`_Name`).

The Wine prefix needs (bring your own copies, see
[`orig/README.md`](orig/README.md)):

| Directory | Holds |
| --- | --- |
| `C:\msvc5` | Visual C++ 5.0's `bin`, `include` and `lib` |
| `C:\dx5` | the DirectX 5 SDK's `include` and `lib` (`just dxsdk`) |

## Workflow

Everything runs headless. `just` lists the recipes. The tools are a
git submodule, [tr-decomp-tools](tools/README.md), shared with the
other Tomb Raider games; after a clone, `just tools-init hooks`.

1. Analyze the original (`just analyze`, then `just manifest`).
   Headless Ghidra imports it, creates the functions that
   `config/symbols.csv` lists but the analysis missed (those only
   reached through pointer tables) and exports the function list.
   New functions are added to `config/symbols.csv` as `FuncXXXXXX`, and
   the manifest names them.
2. Write the function in `src/`, one file per module of the original:
   `.c` where Core's is C, `.cpp` where it is C++, four-space
   indented and without comments. `just decompile [ADDR...]` writes
   Ghidra's pseudocode to `build/decomp/<address>.c`.
3. Record the function and every function and global it uses in
   [`config/symbols.csv`](config/symbols.csv): address, size, name and
   the decorated symbol (`_Name` for C, `@Name@8` for `__fastcall`,
   `_Name@8` for `__stdcall`).
4. Compare (`just compare`): builds with VC5 under Wine and checks
   each function against the original byte for byte, relocations
   resolved through `symbols.csv`. `just compare SYMBOL` shows a
   disassembly diff.
5. Play (`just run`): the compiled functions go into a DLL that the
   patched executable loads, which overwrites each original function
   with a jump to its new one, so the game runs the new code whether it
   matches yet or not. Steps drive it and take screenshots, e.g.
   `just run wait:10 key:Return wait:2 shot:menu` writes
   `build/shots/menu.png`.
