# orig/

Put your own copy of the game here. Nothing in this directory except this
file and the `.sha1` list is tracked.

The target is the Steam release's `Tomb2.exe`. Copy the whole install
directory:

```
orig/tomb2/              the Steam install directory
orig/tomb2/Tomb2.exe     the game, linked 1997-12-10
orig/tomb2/WINPLAY.DLL   Eidos' FMV player, which it imports
orig/tomb2/tombpc.dat    the gameflow script
orig/tomb2/data/         the levels
```

```sh
cd orig && sha1sum -c tomb2.sha1
```

| File | SHA-1 |
| --- | --- |
| `Tomb2.exe` | `2fe685bdef9b08fe43fd2028f20f2079c04b5dcf` |

Its SHA-256 is
`fd9c91d14d54540b3e00c9bedafeef9e4ddb1e9542adc9f29170c7f19b1a657b`,
the one TRX's manifest names.

## Compiler

Visual C++ 5.0, which is not redistributable: `cl` 11.00.7022 and
`link` 5.00.7022. It is on disc 3 of Visual Studio 97 Enterprise
(`VS97ENT3.iso`), under `DEVSTUDIO`, uncompressed. Copy `VC\BIN`,
`VC\INCLUDE` and `VC\LIB` into the Wine prefix as `C:\msvc5\bin`,
`include` and `lib`, and next to them in `bin` the DLLs the linker
loads: `MSPDB50.DLL` and `MSDIS100.DLL` from `SHAREDIDE\BIN`, and
`MSVCP50.DLL` from `VC\REDIST`:

```sh
7z x -oiso VS97ENT3.iso DEVSTUDIO/VC/BIN DEVSTUDIO/VC/INCLUDE DEVSTUDIO/VC/LIB \
    DEVSTUDIO/SHAREDIDE/BIN DEVSTUDIO/VC/REDIST/MSVCP50.DLL
WINEPREFIX=$PWD/.wine wineboot -i
mkdir -p .wine/drive_c/msvc5
cp -r iso/DEVSTUDIO/VC/BIN .wine/drive_c/msvc5/bin
cp -r iso/DEVSTUDIO/VC/INCLUDE .wine/drive_c/msvc5/include
cp -r iso/DEVSTUDIO/VC/LIB .wine/drive_c/msvc5/lib
cp iso/DEVSTUDIO/SHAREDIDE/BIN/MSPDB50.DLL iso/DEVSTUDIO/SHAREDIDE/BIN/MSDIS100.DLL \
    iso/DEVSTUDIO/VC/REDIST/MSVCP50.DLL .wine/drive_c/msvc5/bin/
```

## DirectX 5 SDK

The headers and import libraries, from the SDK's download,
`idx5sdk.exe`:

```sh
just dxsdk idx5sdk.exe   # installs C:\dx5\include and C:\dx5\lib
```

## MASM

3DSYSTEM's hand-written assembly is assembled with MASM 6.11, as Core
did (`ml /c /coff`); 6.11d refuses Core's `GROUP` with `/coff`. From
the MASM 6.11 disks, `DISK1/BIN/ML.EX$` and `ML.ER$` are KWAJ-compressed,
which libmspack expands. Put them in the prefix as
`C:\masm611\bin\ML.EXE` and `ML.ERR`.

## Running

`just run` starts the game under Wine on a fresh Xvfb display. On its
first start, without settings in the prefix's registry, the game shows
its setup dialog; `key:Return` accepts it:

```sh
just run wait:10 key:Return wait:20 shot:title
```
