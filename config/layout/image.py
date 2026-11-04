"""Apply initialized data, import and private-runtime address relocations."""

from __future__ import annotations
from bisect import bisect_right
from pathlib import Path
import json
import re
import struct
import coff
import compare_exe as ce
from .validation import require_payload, require_unchanged


def finish() -> None:
    ours = ce.PE("build/layout/linked.exe")
    original = ce.PE(ce.ORIGINAL)
    mapping = ce.read_map("build/layout/linked.map")
    for name in (".text", ".rdata", ".data"):
        if ours.sections[name][0] != original.sections[name][0]:
            raise ValueError(f"The linked {name} section left its recovered address")
    if ours.sections[".text"][1] != original.sections[".text"][1]:
        raise ValueError("The linked code exceeded its recovered layout")
    plan = json.loads(Path("build/layout/data-plan.json").read_text())
    plan.sort(key=lambda c: c["old"])
    starts = [c["old"] for c in plan]
    newdata = bytearray(ours.data)
    # Globals explicitly fixed by the linker must remain fixed even when their
    # addresses happen to fall inside another allocation's previous position.
    bss = json.loads(Path("build/layout/bss.json").read_text())
    absolute_names: set[str] = set()
    absolute_bindings: dict[str, int] = {}
    for path, entries in bss.items():
        obj = coff.Object(path)
        for idx in entries:
            i = int(idx)
            sym = obj.symbols[i]
            assert sym
            absolute_names.add(
                sym.name
                if sym.storage != 3
                else "_layout_bss_" + Path(path).stem + "_" + str(i)
            )
            name = (
                sym.name
                if sym.storage != 3
                else "_layout_bss_" + Path(path).stem + "_" + str(i)
            )
            absolute_bindings[name] = entries[idx]

    runtime_bindings = json.loads(Path("build/layout/crt-bss.json").read_text())
    absolute_names.update(runtime_bindings)
    absolute_bindings.update(runtime_bindings)

    addends: dict[int, int] = {}
    fixed: set[int] = set()
    anchors = json.loads(Path("build/layout/data-anchors.json").read_text())
    section_bases: dict[tuple[str, int], int] = {}
    for entry in anchors:
        if entry["symbol"] in mapping:
            section_bases[(entry["path"], entry["section"])] = mapping[entry["symbol"]][
                0
            ]
    paths = sorted({entry["path"] for entry in anchors})
    paths.append("build/obj_crt/merged.obj")
    paths.extend(
        "build/obj_placed/" + Path(p).name
        for p in Path("build/layout/objects.txt").read_text().splitlines()
    )
    for path in dict.fromkeys(paths):
        obj = coff.Object(path)
        for num, sec in enumerate(obj.sections, 1):
            if sec.flags & 0x800:
                continue
            base = section_bases.get((path, num))
            if sec.flags & coff.CODE:
                if Path(path).name == "merged.obj":
                    base = mapping["__fpmath"][0]
                else:
                    candidates = [
                        s
                        for s in obj.externals()
                        if s.section == num and s.name in mapping
                    ]
                    if candidates:
                        base = mapping[candidates[0].name][0] - candidates[0].value
            if base is None:
                continue
            for off, index, kind in sec.relocs:
                if kind != coff.DIR32:
                    continue
                addends[base + off] = struct.unpack_from("<I", sec.data, off)[0]
                target = obj.symbols[index]
                assert target
                if target.name in absolute_names or target.section == -1:
                    fixed.add(base + off)

    # Match imports by identity, retaining the target's complete import ABI.
    imports: dict[int, int] = {}
    oldimports = {
        (dll.upper(), name): address + 4 * i
        for dll, address, names in ours.imports()
        for i, name in enumerate(names)
    }
    target_imports = {
        (dll.upper(), name) for dll, _, names in original.imports() for name in names
    }
    if oldimports.keys() - target_imports:
        raise ValueError("The compiled game requires imports outside the recovered ABI")
    for dll, address, names in original.imports():
        for i, name in enumerate(names):
            old = oldimports.get((dll.upper(), name))
            if old is not None:
                imports[old] = address + 4 * i
    # The extra CRT state is private to the recompiled runtime, outside game globals.
    old_bss_start = mapping["_layout_game_bss_end"][0]
    old_bss_end = ours.sections[".data"][0] + ours.sections[".data"][1]
    private_bss_size = old_bss_end - old_bss_start
    extra_base = original.base + original.sizeof_image

    def translate(address: int) -> int:
        if address in imports:
            return imports[address]
        if old_bss_start <= address < old_bss_end:
            return extra_base + address - old_bss_start
        index = bisect_right(starts, address) - 1
        if index >= 0:
            chunk = plan[index]
            if address < chunk["old"] + chunk["size"]:
                return chunk["new"] + address - chunk["old"]
        return address

    # Scatter only compiled contents, then relocate all addresses recorded by link.
    for name in [".rdata", ".data"]:
        va, vsize, rawptr, rawsize = ours.sections[name]
        newdata[rawptr : rawptr + rawsize] = bytes(rawsize)
    for chunk in plan:
        off = ours.offset(chunk["new"])
        assert off is not None
        newdata[off : off + chunk["size"]] = ours.read(chunk["old"], chunk["size"])
    # The single zero initialized game field emitted before the BSS is a real zero.
    for path, entries in bss.items():
        obj = coff.Object(path)
        for idx, address in entries.items():
            if address < 0x46C2D0:
                sym = obj.symbols[int(idx)]
                assert sym
                off = ours.offset(address)
                assert off is not None
                newdata[off : off + 2] = b"\0\0"

    rva, size = ours.directory(5)
    table = ours.read(ours.base + rva, size)
    at = 0
    count = 0
    writes: dict[int, int] = {}
    while at < len(table):
        page, length = struct.unpack_from("<II", table, at)
        if not length:
            break
        for index in range(at + 8, at + length, 2):
            entry = struct.unpack_from("<H", table, index)[0]
            if entry >> 12 != 3:
                continue
            site = ours.base + page + (entry & 0xFFF)
            value = struct.unpack("<I", ours.read(site, 4))[0]
            if site in fixed:
                newvalue = value
            else:
                addend = addends.get(site, 0)
                target = (value - addend) & 0xFFFFFFFF
                newvalue = (translate(target) + addend) & 0xFFFFFFFF
            newsite = (
                translate(site)
                if ours.section_of(site) in (".data", ".rdata")
                else site
            )
            off = ours.offset(newsite)
            assert off is not None, (hex(site), hex(newsite))
            struct.pack_into("<I", newdata, off, newvalue)
            writes[site] = newvalue
            count += 1
        at += length

    for chunk in plan:
        source, size = chunk["old"], chunk["size"]
        destination = ours.offset(chunk["new"])
        assert destination is not None
        operands = {
            site - source: value
            for site, value in writes.items()
            if source <= site < source + size
        }
        require_payload(
            ours.read(source, size),
            bytes(newdata[destination : destination + size]),
            operands,
        )
    text_va, text_size, text_raw, _ = ours.sections[".text"]
    allowed = [
        (site - text_va, 4) for site in writes if text_va <= site < text_va + text_size
    ]
    allowed += [
        (chunk["new"] - text_va, chunk["size"])
        for chunk in plan
        if text_va <= chunk["new"] < text_va + text_size
    ]
    require_unchanged(
        ours.read(text_va, text_size),
        bytes(newdata[text_raw : text_raw + text_size]),
        allowed,
    )

    # Serialize structured import records; no executable bytes come from the target.
    idata_va, idata_vsize, idata_ptr, idata_raw = original.sections[".idata"]
    idata = bytearray(idata_raw)
    orig_rva, orig_size = original.directory(1)
    pos = original.offset(original.base + orig_rva)
    assert pos is not None
    index = 0
    while True:
        fields = struct.unpack_from("<IIIII", original.data, pos + index * 20)
        oft, timestamp, forwarder, name_rva, ft = fields
        if not name_rva:
            break
        struct.pack_into(
            "<IIIII", idata, orig_rva - (idata_va - original.base) + index * 20, *fields
        )
        dll = original.cstr(original.base + name_rva).encode() + b"\0"
        start = original.base + name_rva - idata_va
        idata[start : start + len(dll)] = dll
        n = 0
        while True:
            token = struct.unpack(
                "<I", original.read(original.base + (oft or ft) + n * 4, 4)
            )[0]
            struct.pack_into(
                "<I", idata, original.base + (oft or ft) + n * 4 - idata_va, token
            )
            iat_token = struct.unpack(
                "<I", original.read(original.base + ft + n * 4, 4)
            )[0]
            struct.pack_into(
                "<I", idata, original.base + ft + n * 4 - idata_va, iat_token
            )
            if not token:
                break
            if token & 0x80000000 == 0:
                hint = struct.unpack("<H", original.read(original.base + token, 2))[0]
                func = original.cstr(original.base + token + 2).encode() + b"\0"
                start = original.base + token - idata_va
                struct.pack_into("<H", idata, start, hint)
                idata[start + 2 : start + 2 + len(func)] = func
            n += 1
        index += 1
    _, _, rawptr, rawsize = ours.sections[".idata"]
    assert rawsize == idata_raw
    newdata[rawptr : rawptr + rawsize] = idata

    # Resources use RVAs rather than VA relocations; move their directory and payload.
    rsrc_old, rsrc_size, rsrc_ptr, rsrc_raw = ours.sections[".rsrc"]
    rsrc_new = original.sections[".rsrc"][0]

    def resources(offset: int) -> None:
        named, ids = struct.unpack_from("<HH", newdata, rsrc_ptr + offset + 12)
        # Resource timestamps are metadata, set to the target's directory metadata.
        corresponding = original.sections[".rsrc"][2] + offset
        timestamp = struct.unpack_from("<I", original.data, corresponding + 4)[0]
        struct.pack_into("<I", newdata, rsrc_ptr + offset + 4, timestamp)
        for i in range(named + ids):
            key, ptr = struct.unpack_from(
                "<II", newdata, rsrc_ptr + offset + 16 + i * 8
            )
            if ptr & 0x80000000:
                resources(ptr & 0x7FFFFFFF)
            else:
                value = struct.unpack_from("<I", newdata, rsrc_ptr + ptr)[0]
                struct.pack_into(
                    "<I", newdata, rsrc_ptr + ptr, value + rsrc_new - rsrc_old
                )

    resources(0)
    # Reuse the discarded .reloc header for zero initialized CRT state. It has no
    # file payload, so the source code and assets determine every output file byte.
    for number, name in enumerate(ours.order):
        header = ours.opt + ours.optsz + number * 40
        if name in original.sections:
            original_header = (
                original.opt + original.optsz + original.order.index(name) * 40
            )
            characteristics = struct.unpack_from(
                "<I", original.data, original_header + 36
            )[0]
            struct.pack_into("<I", newdata, header + 36, characteristics)
        if name == ".data":
            struct.pack_into("<I", newdata, header + 8, original.sections[".data"][1])
        elif name == ".idata":
            struct.pack_into(
                "<II", newdata, header + 8, idata_vsize, idata_va - ours.base
            )
        elif name == ".rsrc":
            struct.pack_into("<I", newdata, header + 12, rsrc_new - ours.base)
        elif name == ".reloc":
            if not private_bss_size:
                if number != len(ours.order) - 1:
                    raise ValueError(
                        "Only a trailing relocation section can be removed"
                    )
                newdata[header : header + 40] = bytes(40)
                struct.pack_into("<H", newdata, ours.pe + 6, len(ours.order) - 1)
                continue
            struct.pack_into(
                "<8sIIIIIIHHI",
                newdata,
                header,
                b".crt_bss",
                private_bss_size,
                extra_base - ours.base,
                0,
                0,
                0,
                0,
                0,
                0,
                0xC0000080,
            )
    struct.pack_into("<II", newdata, ours.opt + 96 + 8, orig_rva, orig_size)
    struct.pack_into("<II", newdata, ours.opt + 96 + 2 * 8, *original.directory(2))
    struct.pack_into("<II", newdata, ours.opt + 96 + 5 * 8, 0, 0)
    struct.pack_into("<II", newdata, ours.opt + 96 + 12 * 8, *original.directory(12))
    flags = struct.unpack_from("<H", newdata, ours.pe + 22)[0]
    struct.pack_into("<H", newdata, ours.pe + 22, flags | 1)
    struct.pack_into(
        "<I",
        newdata,
        ours.opt + 56,
        ((extra_base - ours.base + old_bss_end - old_bss_start + 0xFFF) // 0x1000)
        * 0x1000,
    )
    file_alignment = struct.unpack_from("<I", newdata, ours.opt + 36)[0]
    initialized_size = sum(
        ((original.sections[name][1] + file_alignment - 1) // file_alignment)
        * file_alignment
        for name in (".rdata", ".data", ".idata", ".rsrc")
    )
    struct.pack_into("<I", newdata, ours.opt + 8, initialized_size)
    struct.pack_into(
        "<I", newdata, ours.opt + 12, (old_bss_end - old_bss_start + 0x1FF) & ~0x1FF
    )
    newdata = newdata[: rsrc_ptr + rsrc_raw]
    output = Path("build/tomb2.exe")
    output.write_bytes(newdata)
    # Keep the map's symbol addresses synchronized with physical placement.
    maptext = Path("build/layout/linked.map").read_text(encoding="latin-1")
    line_re = re.compile(
        r"^(\s*)([0-9a-f]{4}):([0-9a-f]{8})(\s+)(\S+)(\s+)([0-9a-f]{8})(.*)$"
    )
    result = []
    provenance = json.loads(Path("build/layout/crt-symbols.json").read_text())
    final = ce.PE(output)
    for line in maptext.splitlines():
        match = line_re.match(line)
        if match:
            name = match[5]
            address = int(match[7], 16)
            if name in absolute_names:
                newaddr = address
            elif name.startswith("__imp__"):
                newaddr = imports.get(address, address)
            elif final.section_of(address) == ".text":
                newaddr = address
            else:
                newaddr = translate(address)
            section = final.section_of(newaddr)
            if section is not None and int(match[2], 16) != 0:
                secidx = final.order.index(section) + 1
                offset = newaddr - final.sections[section][0]
                line = f"{match[1]}{secidx:04x}:{offset:08x}{match[4]}{name}{match[6]}{newaddr:08x}{match[8]}"
        if match and match[5] in provenance:
            line = re.sub(r"merged\.obj$", "libc:" + provenance[match[5]], line)
        result.append(line)
    for name, address in sorted(
        absolute_bindings.items(), key=lambda entry: (entry[1], entry[0])
    ):
        if name not in mapping:
            section = final.order.index(".data") + 1
            offset = address - final.sections[".data"][0]
            result.append(
                f" {section:04x}:{offset:08x}       {name} {address:08x}   000_globals.obj"
            )
    Path("build/tomb2_exe.map").write_text("\n".join(result) + "\n", encoding="latin-1")
    print(
        "FINISHED",
        len(newdata),
        "bytes;",
        len(plan),
        "allocations;",
        count,
        "relocations",
    )
