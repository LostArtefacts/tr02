"""Expose live initialized COFF sections to the standalone layout pass."""

from __future__ import annotations
from pathlib import Path
import json
import struct
import coff


def add(paths: list[str]) -> None:
    manifest: list[dict[str, str | int]] = []
    for ordinal, path in enumerate(paths):
        raw = bytearray(Path(path).read_bytes())
        obj = coff.Object(path)
        symptr, nsym = struct.unpack_from("<II", raw, 8)
        end = symptr + nsym * 18
        records = bytearray(raw[symptr:end])
        strings = bytearray(raw[end:])
        for number, sec in enumerate(obj.sections, 1):
            if (
                not sec.flags & 0x40
                or not len(sec.data)
                or sec.name.startswith((".debug", ".drectve"))
                or sec.flags & 0x800
            ):
                continue
            name = "_layout_data_" + str(ordinal) + "_" + str(number)
            off = len(strings)
            strings += name.encode() + b"\0"
            records += struct.pack("<IIIhHBB", 0, off, 0, number, 0, 2, 0)
            nsym += 1
            manifest.append({"path": path, "section": number, "symbol": name})
        struct.pack_into("<I", strings, 0, len(strings))
        struct.pack_into("<I", raw, 12, nsym)
        Path(path).write_bytes(raw[:symptr] + records + strings)
    Path("build/layout/data-anchors.json").write_text(json.dumps(manifest, indent=2))
