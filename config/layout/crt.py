"""Relink the selected VC5 runtime members in recovered address order."""

from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path
import re
import struct
import coff
import compare_exe as ce


@dataclass
class Block:
    path: str
    section: int
    address: int
    data: bytes
    offset: int = 0
    padding: int = 0x90


def archive_members(path: Path) -> dict[str, bytes]:
    """Read long and short member names from a Microsoft COFF archive."""
    raw = path.read_bytes()
    longnames = b""
    files: dict[str, bytes] = {}
    pos = 8
    while pos < len(raw):
        head = raw[pos : pos + 60]
        length = int(head[48:58])
        name = head[:16].decode().strip()
        data = raw[pos + 60 : pos + 60 + length]
        if name == "//":
            longnames = data
        elif name.startswith("/") and name[1:].isdigit():
            off = int(name[1:])
            name = longnames[off:].split(b"\0")[0].split(b"/\n")[0].decode()
            files[Path(name.replace("\\", "/")).name.lower()] = data
        elif name not in ["/", "//"]:
            files[name.rstrip("/").lower()] = data
        pos += 60 + length + (length & 1)
    return files


def recover_bss(
    parsed: dict[str, coff.Object], blocks: list[Block], original: ce.PE
) -> dict[tuple[str, int], int]:
    """Recover whole BSS sections and common symbols from matching CRT code."""
    externals: dict[str, tuple[str, int]] = {}
    for path, obj in parsed.items():
        for index, sym in enumerate(obj.symbols):
            if sym and sym.storage == 2 and (sym.section > 0 or sym.value > 0):
                if sym.section > 0 or sym.name not in externals:
                    externals[sym.name] = (path, index)
    bindings: dict[tuple[str, int], int] = {}
    sections: dict[tuple[str, int], int] = {}
    for block in blocks:
        obj = parsed[block.path]
        sec = obj.sections[block.section - 1]
        mask = {off + j for off, _, _ in sec.relocs for j in range(4)}
        expected = original.read(block.address, len(block.data))
        if len(expected) != len(block.data) or any(
            a != b and i not in mask
            for i, (a, b) in enumerate(zip(block.data, expected))
        ):
            continue
        for off, index, kind in sec.relocs:
            if kind != coff.DIR32:
                continue
            sym = obj.symbols[index]
            assert sym
            path = block.path
            if sym.section == 0 and sym.storage == 2:
                if sym.name not in externals:
                    continue
                path, index = externals[sym.name]
                sym = parsed[path].symbols[index]
                assert sym
            common = sym.section == 0 and sym.storage == 2 and sym.value > 0
            bss = (
                sym.section > 0 and parsed[path].sections[sym.section - 1].flags & 0x80
            )
            if not common and not bss:
                continue
            addend = struct.unpack_from("<I", sec.data, off)[0]
            address = (struct.unpack_from("<I", expected, off)[0] - addend) & 0xFFFFFFFF
            key = (path, sym.section if bss else index)
            table = sections if bss else bindings
            value = address - sym.value if bss else address
            if key in table and table[key] != value:
                raise ValueError(f"Conflicting CRT BSS binding: {path}:{sym.name}")
            table[key] = value
    for (path, number), address in sections.items():
        sec = parsed[path].sections[number - 1]
        data_va, data_size, _, raw_size = original.sections[".data"]
        if not data_va <= address or address + len(sec.data) > data_va + data_size:
            raise ValueError(f"CRT BSS section outside original data: {path}:{number}")
        initialized = max(0, min(len(sec.data), data_va + raw_size - address))
        if any(original.read(address, initialized)):
            raise ValueError(f"CRT BSS overlaps initialized contents: {path}:{number}")
        for index, sym in enumerate(parsed[path].symbols):
            if sym and sym.section == number:
                bindings[(path, index)] = address + sym.value
    for (path, index), address in bindings.items():
        sym = parsed[path].symbols[index]
        assert sym
        if sym.section == 0:
            data_va, data_size, _, raw_size = original.sections[".data"]
            if not data_va <= address or address + sym.value > data_va + data_size:
                raise ValueError(f"CRT common outside original data: {sym.name}")
            initialized = max(0, min(sym.value, data_va + raw_size - address))
            if any(original.read(address, initialized)):
                raise ValueError(
                    f"CRT common overlaps initialized contents: {sym.name}"
                )
    return bindings


def merge() -> tuple[list[str], str]:
    original = ce.PE(ce.ORIGINAL)
    ourmap = ce.read_map("build/layout/preliminary.map")
    # The original retained this helper; current game code no longer selects it.
    ourmap["__allshl"] = (0x460BC0, "libc:llshl.obj")
    files = archive_members(Path(".wine/drive_c/msvc5sp3/lib/LIBC.LIB"))
    rtm = archive_members(Path(".wine/drive_c/msvc5/lib/LIBC.LIB"))
    # These two RTM members match the target; their SP3 replacements do not.
    for name in ("output.obj", "gmtime.obj"):
        files[name] = rtm[name]
    objects = sorted(
        set(
            obj.split(":", 1)[1].lower()
            for a, obj in ourmap.values()
            if obj.lower().startswith("libc:")
        ),
        key=lambda obj: min(
            a for a, o in ourmap.values() if o.lower() == "libc:" + obj
        ),
    )
    out = Path("build/obj_crt")
    out.mkdir(exist_ok=True)
    known: dict[str, int] = {}
    for addr, size, name, syms in ce.read_symbols():
        for sym in syms:
            known[sym] = addr
    start, span, rawptr, _ = original.sections[".text"]
    blob = original.data[rawptr : rawptr + span]
    parsed: dict[str, coff.Object] = {}
    blocks: list[Block] = []
    unknown: list[tuple[str, int, list[str]]] = []
    aliases_outside: dict[str, int] = {"__fpclear": 0x42A120}
    manual: dict[tuple[str, int], int] = {
        ("hooks.obj", 3): 0x45C080,
        ("hooks.obj", 7): 0x45C110,
        ("mbctype.obj", 9): 0x45E9C0,
        ("intrncvt.obj", 18): 0x45F5B0,
        ("intrncvt.obj", 20): 0x45F5D0,
        ("intrncvt.obj", 24): 0x45F5F0,
        ("intrncvt.obj", 28): 0x45F630,
        ("validate.obj", 1): 0x45FF40,
        ("validate.obj", 3): 0x45FF60,
    }
    for name in objects:
        path = str(out / name)
        Path(path).write_bytes(files[name])
        obj = coff.Object(path)
        parsed[path] = obj
        for secnum, sec in enumerate(obj.sections, 1):
            if not sec.flags & coff.CODE or not len(sec.data):
                continue
            live = [
                s
                for s in obj.symbols
                if s
                and s.section == secnum
                and s.name in ourmap
                and (
                    ourmap[s.name][1].lower() == "libc:" + name
                    or (name == "aw_map.obj" and secnum == 10)
                )
            ]
            if not live or all(s.name in aliases_outside for s in live):
                continue
            mask = {off + j for off, idx, kind in sec.relocs for j in range(4)}
            size = min(len(sec.data), 128)
            pat = b"".join(
                b"." if i in mask else re.escape(bytes([b]))
                for i, b in enumerate(sec.data[:size])
            )
            hits = [
                start + m.start()
                for m in re.finditer(pat, blob, re.S)
                if start + m.start() >= 0x459030
            ]
            anchors = {
                known[s.name] - s.value
                for s in live
                if s.name in known and 0x459030 <= known[s.name] < 0x462DBA
            }
            address = (
                hits[0]
                if len(hits) == 1
                else next(iter(anchors))
                if len(anchors) == 1
                else None
            )
            if name == "mbctype.obj" and secnum == 3:
                address = 0x45E710
            address = manual.get((name, secnum), address)
            if address is None:
                unknown.append((name, secnum, [s.name for s in live]))
                continue
            data = sec.data
            floor = max(
                [s.value + 1 for s in obj.symbols if s and s.section == secnum]
                + [off + 4 for off, _, _ in sec.relocs]
            )
            while len(data) > floor and data[-1] in (0x90, 0xCC):
                data = data[:-1]
            padding = 0x90 if sec.data[len(data) :].startswith(b"\x90") else 0xCC
            blocks.append(Block(path, secnum, address, data, padding=padding))
    if unknown:
        raise ValueError(("Unplaced CRT", unknown))
    bss_bindings = recover_bss(parsed, blocks, original)
    by_addr: dict[int, list[Block]] = {}
    for block in blocks:
        by_addr.setdefault(block.address, []).append(block)
    unique = [group[0] for address, group in sorted(by_addr.items())]
    code = bytearray()
    origin = 0x459030
    offsets: dict[tuple[str, int], int] = {}
    for idx, block in enumerate(unique):
        cursor = origin + len(code)
        if cursor < block.address:
            padding = unique[idx - 1].padding if idx else 0x90
            code += bytes([padding]) * (block.address - cursor)
        if cursor > block.address:
            print(
                "CRT OVERFLOW",
                Path(block.path).name,
                hex(block.address),
                hex(cursor),
                flush=True,
            )
        block.offset = len(code)
        code += block.data
        for alias in by_addr[block.address]:
            assert alias.data == block.data, (
                alias.path,
                alias.section,
                block.path,
                block.section,
            )
            offsets[(alias.path, alias.section)] = block.offset
    # The final library import thunks follow .text$c and precede .text$x.
    print("CRT CODE", len(unique), hex(origin + len(code)), flush=True)
    import json

    layout = []
    for block in unique:
        sec = parsed[block.path].sections[block.section - 1]
        mask = {off + j for off, index, kind in sec.relocs for j in range(4)}
        expected = original.read(block.address, len(block.data))
        same = all(
            i in mask or a == b for i, (a, b) in enumerate(zip(block.data, expected))
        )
        layout.append(
            {
                "path": block.path,
                "section": block.section,
                "original": block.address,
                "offset": block.offset,
                "size": len(block.data),
                "matched": same,
            }
        )
    Path("build/layout/crt-code.json").write_text(json.dumps(layout, indent=2))
    defined: dict[str, tuple[int, int]] = {}
    absolute: dict[str, int] = dict(aliases_outside)
    undefined: set[str] = set()
    names: dict[tuple[str, int], str] = {}
    promote: dict[str, dict[int, str]] = {}
    for path, obj in parsed.items():
        for i, sym in enumerate(obj.symbols):
            if not sym:
                continue
            if (path, i) in bss_bindings:
                name = (
                    sym.name
                    if sym.storage == 2
                    else "_layout_crt_data_" + Path(path).stem + "_" + str(i)
                )
                names[(path, i)] = name
                address = bss_bindings[(path, i)]
                if name in absolute and absolute[name] != address:
                    raise ValueError(f"Conflicting CRT absolute symbol: {name}")
                absolute[name] = address
            if (
                sym.section > 0
                and obj.sections[sym.section - 1].flags & coff.CODE
                and not sym.name.startswith(".")
            ):
                name = (
                    sym.name
                    if sym.storage == 2
                    else "_layout_crt_" + Path(path).stem + "_" + str(i)
                )
                names[(path, i)] = name
                if (path, sym.section) in offsets:
                    value = (1, offsets[(path, sym.section)] + sym.value)
                    if name in defined:
                        assert defined[name] == value, (name, defined[name], value)
                    defined[name] = value

    provenance = {
        name: Path(path).name
        for (path, index), name in names.items()
        if name in defined
    }
    Path("build/layout/crt-symbols.json").write_text(json.dumps(provenance, indent=2))
    Path("build/layout/crt-bss.json").write_text(
        json.dumps(
            {names[key]: address for key, address in bss_bindings.items()}, indent=2
        )
    )

    def target_name(path: str, index: int) -> str:
        obj = parsed[path]
        sym = obj.symbols[index]
        assert sym
        if (path, index) in names:
            return names[(path, index)]
        if sym.storage == 2:
            return sym.name
        name = "_layout_crt_data_" + Path(path).stem + "_" + str(index)
        if sym.section == -1:
            absolute[name] = sym.value
        elif sym.section > 0:
            promote.setdefault(path, {})[index] = name
        else:
            raise ValueError((path, sym.name, sym.section, sym.storage))
        return name

    relocs: list[tuple[int, str, int]] = []
    for block in unique:
        obj = parsed[block.path]
        sec = obj.sections[block.section - 1]
        for off, index, kind in sec.relocs:
            name = target_name(block.path, index)
            relocs.append((block.offset + off, name, kind))
            undefined.add(name)
    outputs: list[str] = []
    for path, obj in parsed.items():
        raw = bytearray(Path(path).read_bytes())
        symptr, nsym = struct.unpack_from("<II", raw, 8)
        records = bytearray(raw[symptr : symptr + nsym * 18])
        strings = bytearray(raw[symptr + nsym * 18 :])
        used_indices = {
            index
            for sec in obj.sections
            if not sec.flags & coff.CODE and not sec.name.startswith(".debug")
            for off, index, kind in sec.relocs
        }
        for i, sym in enumerate(obj.symbols):
            if not sym:
                continue
            if (path, i) in bss_bindings:
                name = names[(path, i)]
                off = len(strings)
                strings += name.encode() + b"\0"
                struct.pack_into("<II", records, i * 18, 0, off)
                struct.pack_into("<Ih", records, i * 18 + 8, 0, 0)
                records[i * 18 + 16] = 2
                continue
            if (
                sym.section == 0
                and sym.storage == 2
                and sym.value == 0
                and i not in used_indices
            ):
                struct.pack_into("<Ih", records, i * 18 + 8, 0, -1)
                records[i * 18 + 16] = 3
            if (
                sym.section > 0
                and obj.sections[sym.section - 1].flags & coff.CODE
                and not sym.name.startswith(".")
            ):
                name = names[(path, i)]
                off = len(strings)
                strings += name.encode() + b"\0"
                struct.pack_into("<II", records, i * 18, 0, off)
                if name in defined or name in absolute:
                    struct.pack_into("<Ih", records, i * 18 + 8, 0, 0)
                    records[i * 18 + 16] = 2
                else:
                    struct.pack_into("<Ih", records, i * 18 + 8, 0, -1)
                    records[i * 18 + 16] = 3
        for index, name in promote.get(path, {}).items():
            sym = obj.symbols[index]
            assert sym
            off = len(strings)
            strings += name.encode() + b"\0"
            records += struct.pack("<IIIhHBB", 0, off, sym.value, sym.section, 0, 2, 0)
            nsym += 1
        for n, sec in enumerate(obj.sections):
            bound_bss = sec.flags & 0x80 and any(
                (path, i) in bss_bindings and sym and sym.section == n + 1
                for i, sym in enumerate(obj.symbols)
            )
            if sec.flags & coff.CODE or bound_bss:
                at = 20 + n * 40
                struct.pack_into("<IIIIHH", raw, at + 16, 0, 0, 0, 0, 0, 0)
                raw[at : at + 8] = b".drectve"
                struct.pack_into("<I", raw, at + 36, 0x100A00)
        struct.pack_into("<I", raw, 12, nsym)
        struct.pack_into("<I", strings, 0, len(strings))
        Path(path).write_bytes(raw[:symptr] + records + strings)
        outputs.append(path)
    merged = str(out / "merged.obj")
    coff.write_object(
        merged,
        [
            (".text$c", coff.TEXT, bytes(code), relocs),
            (".rdata$z", coff.RDATA, bytes(0x88), []),
        ],
        defined,
        sorted(undefined - defined.keys() - absolute.keys()),
        absolute,
    )
    return outputs, merged
