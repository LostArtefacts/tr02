"""Tomb2's original-address standalone link, using compiled COFF payloads."""

from __future__ import annotations

from pathlib import Path
import struct

import build

from . import anchors, bss, crt, data, game, image


def tail_imports() -> str:
    """Keep the runtime's imported call thunks after its compiled code."""
    source = Path(".wine/drive_c/msvc5sp3/lib/KERNEL32.LIB")
    archive = bytearray(source.read_bytes())
    position = 8
    while position < len(archive):
        size = int(archive[position + 48 : position + 58])
        start = position + 60
        if archive[start : start + 2] == b"\x4c\x01":
            count = struct.unpack_from("<H", archive, start + 2)[0]
            for index in range(count):
                at = start + 20 + index * 40
                flags = struct.unpack_from("<I", archive, at + 36)[0]
                if flags & 0x20:
                    archive[at : at + 8] = b".text$d\0"
        position += 60 + size + (size & 1)
    output = Path("build/obj_placed/kernel32-layout.lib")
    output.write_bytes(archive)
    return build.win(str(output))


def link(objects: list[str]) -> None:
    """Resolve archive selection, then apply the recovered code and data layout."""
    Path("build/layout").mkdir(exist_ok=True)
    Path("build/layout/objects.txt").write_text("\n".join(objects) + "\n")
    old_exe, old_map = build.EXE, build.EXE_MAP
    options = build.EXE_CONFIG
    old_flags = options["ldflags"]
    old_libs = options["libs"]
    old_order = options["link_order"]
    try:
        bss.recover()
        placed = bss.place(game.place(objects))
        options["link_order"] = "source"
        options["ldflags"] = [
            *old_flags,
            "/ORDER:@build\\layout\\order.txt",
            "/OPT:NOICF",
        ]
        build.EXE = "build/layout/preliminary.exe"
        build.EXE_MAP = "build/layout/preliminary.map"
        build.link_exe(placed, customize=False)
        runtime, code = crt.merge()
        final_objects = [*placed, *runtime, code]
        anchors.add(final_objects)
        kernel = tail_imports()
        options["libs"] = [
            kernel if name.lower() == "kernel32.lib" else name
            for name in old_libs
            if name.lower() != "libc.lib"
        ]
        options["ldflags"] = [
            flag for flag in options["ldflags"] if flag.upper() != "/FIXED"
        ]
        options["ldflags"] += ["/NODEFAULTLIB:libc", "/FIXED:NO"]
        build.EXE = "build/layout/linked.exe"
        build.EXE_MAP = "build/layout/linked.map"
        build.link_exe(final_objects, customize=False)
        data.prepare()
        image.finish()
    finally:
        build.EXE, build.EXE_MAP = old_exe, old_map
        options["ldflags"] = old_flags
        options["libs"] = old_libs
        options["link_order"] = old_order
