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

Visual C++ 5.0, which is not redistributable. Disc 1 of Visual Studio
97 Professional holds only Visual Basic and Visual FoxPro; Visual C++
is on another disc. Copy its `VC\BIN`, `VC\INCLUDE` and `VC\LIB` into
the Wine prefix as `C:\msvc5\bin`, `include` and `lib`, with the DLLs
`cl.exe` and `link.exe` load (`MSPDB50.DLL`, from `SharedIDE\BIN`) next
to them in `bin`.

## DirectX 5 SDK

The headers and import libraries, from the SDK's download,
`idx5sdk.exe`:

```sh
just dxsdk idx5sdk.exe   # installs C:\dx5\include and C:\dx5\lib
```
