"""Recover initialized allocations without copying target payload bytes."""

from __future__ import annotations
from bisect import bisect_right
from collections import Counter
from dataclasses import dataclass, field
from pathlib import Path
import json
import re
import struct
from unittest.mock import patch
import build, coff, compare, compare_exe as ce
from .validation import require_coverage


@dataclass
class Chunk:
    old: int
    size: int
    names: list[str]
    path: str
    section: int
    offset: int
    readonly: bool
    candidates: Counter[int] = field(default_factory=Counter)
    new: int | None = None


def prepare() -> None:
    ours = ce.PE("build/layout/linked.exe")
    orig = ce.PE(ce.ORIGINAL)
    ourmap = ce.read_map("build/layout/linked.map")
    known = compare.read_symbols()
    chunks: list[Chunk] = []
    metadata = json.loads(Path("build/layout/data-anchors.json").read_text())
    parsed: dict[str, coff.Object] = {}
    for entry in metadata:
        name = entry["symbol"]
        path = entry["path"]
        number = entry["section"]
        if name not in ourmap:
            continue
        base = ourmap[name][0]
        if ours.section_of(base) not in (".data", ".rdata"):
            continue
        obj = parsed.setdefault(path, coff.Object(path))
        sec = obj.sections[number - 1]
        if sec.name == ".rdata$z":
            continue
        symbols = [
            s
            for s in obj.symbols
            if s
            and s.section == number
            and not s.name.startswith((".", "_layout_data_"))
        ]
        starts = sorted({0, *[s.value for s in symbols if s.value < len(sec.data)]})
        for index, start in enumerate(starts):
            end = starts[index + 1] if index + 1 < len(starts) else len(sec.data)
            names = [s.name for s in symbols if s.value == start]
            chunk = Chunk(
                base + start,
                end - start,
                names,
                path,
                number,
                start,
                ours.section_of(base) == ".rdata",
            )
            for symbol in names:
                if symbol in known:
                    address, size = known[symbol]
                    if (
                        orig.section_of(address) in (".data", ".rdata", ".text")
                        and orig.offset(address) is not None
                    ):
                        chunk.candidates[address] += 100
            if sec.name.startswith(".CRT$"):
                chunk.candidates[chunk.old] += 100
            chunks.append(chunk)
    # DirectX import libraries contribute GUID objects outside the game and CRT
    # inputs. Their public symbols identify the live allocations in the map.
    library_sections: list[tuple[str, int, bytes, list[tuple[int, str, int]]]] = []
    library_definitions: dict[str, tuple[int, int]] = {}
    library_chunks: list[Chunk] = []
    library_path = "build/layout/library-data.obj"
    for name, (address, owner) in ourmap.items():
        if not owner.lower().startswith("dxguid:"):
            continue
        if ours.section_of(address) != ".rdata":
            continue
        if any(c.old <= address < c.old + c.size for c in chunks):
            continue
        number = len(library_sections) + 1
        payload = ours.read(address, 16)
        library_sections.append((".rdata", coff.RDATA, payload, []))
        library_definitions[name] = (number, 0)
        chunk = Chunk(address, 16, [name], library_path, number, 0, True)
        if name in known:
            chunk.candidates[known[name][0]] += 100
        library_chunks.append(chunk)
    coff.write_object(library_path, library_sections, library_definitions, [], {})
    parsed[library_path] = coff.Object(library_path)
    chunks.extend(library_chunks)
    for section_name in (".data", ".rdata"):
        address, virtual_size, raw_offset, raw_size = ours.sections[section_name]
        size = min(virtual_size, raw_size)
        require_coverage(
            ours.read(address, size),
            address,
            ((chunk.old, chunk.size) for chunk in chunks),
        )
    # Linker-folded COMDAT aliases can expose several anchors for one allocation.
    by_old: dict[tuple[int, int], Chunk] = {}
    for chunk in chunks:
        key = (chunk.old, chunk.size)
        if key in by_old:
            by_old[key].names += chunk.names
            by_old[key].candidates += chunk.candidates
        else:
            by_old[key] = chunk
    chunks = sorted(by_old.values(), key=lambda c: c.old)
    starts = [c.old for c in chunks]

    def find(address: int) -> Chunk | None:
        index = bisect_right(starts, address) - 1
        if index >= 0 and address < chunks[index].old + chunks[index].size:
            return chunks[index]
        return None

    def add_pair(old: int, new: int, weight: int = 1) -> None:
        chunk = find(old)
        if chunk is None:
            return
        to = new - (old - chunk.old)
        destination = orig.section_of(to)
        if destination not in (".data", ".rdata") or orig.offset(to) is None:
            return
        if (destination == ".rdata") != chunk.readonly:
            return
        chunk.candidates[to] += weight

    paths = Path("build/layout/objects.txt").read_text().splitlines()
    srcs = {str(Path(obj).stem): src for src, obj in build.sources(build.EXE)}
    sources = [
        (srcs[Path(path).stem], path) for path in paths if Path(path).stem in srcs
    ]
    with patch.object(build, "sources", return_value=sources):
        matched = {r[0] for r in compare.results(known) if r[1]}
    for src, path in sources:
        obj = coff.Object(path)
        rel = str(Path(src).relative_to("src"))
        for sym in obj.symbols:
            if (
                not sym
                or sym.section <= 0
                or not obj.sections[sym.section - 1].flags & coff.CODE
            ):
                continue
            key = sym.name if sym.name in known else rel + ":" + sym.name
            if key not in matched:
                continue
            hit = ourmap.get(sym.name)
            if hit is None:
                continue
            actual = hit[0]
            original, size = known[key]
            sec = obj.sections[sym.section - 1]
            for off, index, kind in sec.relocs:
                if kind != coff.DIR32:
                    continue
                addend = struct.unpack_from("<I", sec.data, off)[0]
                old = struct.unpack("<I", ours.read(actual + off - sym.value, 4))[0]
                new = struct.unpack("<I", orig.read(original + off - sym.value, 4))[0]
                add_pair((old - addend) & 0xFFFFFFFF, (new - addend) & 0xFFFFFFFF)
    crt = coff.Object("build/obj_crt/merged.obj")
    crtbase = ourmap["__fpmath"][0]
    for block in json.loads(Path("build/layout/crt-code.json").read_text()):
        if not block["matched"]:
            continue
        begin, end = block["offset"], block["offset"] + block["size"]
        original = block["original"]
        for off, index, kind in crt.sections[0].relocs:
            if kind != coff.DIR32 or not begin <= off < end:
                continue
            addend = struct.unpack_from("<I", crt.sections[0].data, off)[0]
            old = struct.unpack("<I", ours.read(crtbase + off, 4))[0]
            new = struct.unpack("<I", orig.read(original + off - begin, 4))[0]
            add_pair((old - addend) & 0xFFFFFFFF, (new - addend) & 0xFFFFFFFF)
    # Metadata thunks in .text$x retain the original order and instruction shape.
    rva, relocsize = ours.directory(5)
    relocations: list[int] = []
    position = 0
    relocdata = ours.read(ours.base + rva, relocsize)
    while position < len(relocdata):
        page, length = struct.unpack_from("<II", relocdata, position)
        if not length:
            break
        for at in range(position + 8, position + length, 2):
            entry = struct.unpack_from("<H", relocdata, at)[0]
            if entry >> 12 == 3:
                relocations.append(ours.base + page + (entry & 0xFFF))
        position += length
    for site in relocations:
        if 0x462D50 <= site < 0x462DBA:
            old = struct.unpack("<I", ours.read(site, 4))[0]
            new = struct.unpack("<I", orig.read(site, 4))[0]
            add_pair(old, new)
    # Propagate table and exception metadata references from placed allocations.
    for iteration in range(5):
        for chunk in chunks:
            if not chunk.candidates:
                continue
            destination = chunk.candidates.most_common(1)[0][0]
            obj = parsed[chunk.path]
            sec = obj.sections[chunk.section - 1]
            for off, index, kind in sec.relocs:
                if (
                    kind != coff.DIR32
                    or not chunk.offset <= off < chunk.offset + chunk.size
                ):
                    continue
                at = off - chunk.offset
                addend = struct.unpack_from("<I", sec.data, off)[0]
                old = struct.unpack("<I", ours.read(chunk.old + at, 4))[0]
                value = orig.read(destination + at, 4)
                if len(value) != 4:
                    continue
                new = struct.unpack("<I", value)[0]
                add_pair((old - addend) & 0xFFFFFFFF, (new - addend) & 0xFFFFFFFF)
    # Compiler alignment padding is outside the declared objects.
    for chunk in chunks:
        payload = ours.read(chunk.old, chunk.size)
        sizes = [known[n][1] for n in chunk.names if n in known and known[n][1]]
        if sizes:
            size = max(sizes)
            if size < chunk.size and not any(payload[size:]):
                chunk.size = size
        if b"\0" in payload:
            size = payload.index(0) + 1
            if (
                size > 1
                and all(b in (9, 10, 13) or 32 <= b < 127 for b in payload[: size - 1])
                and not any(payload[size:])
            ):
                chunk.size = min(chunk.size, size)
    # A reference-confirmed following allocation bounds any remaining padding.
    addresses = sorted(
        {c.candidates.most_common(1)[0][0] for c in chunks if c.candidates}
    )
    for chunk in chunks:
        if not chunk.candidates:
            continue
        destination = chunk.candidates.most_common(1)[0][0]
        index = bisect_right(addresses, destination)
        if index < len(addresses):
            size = addresses[index] - destination
            if 0 < size < chunk.size and not any(
                ours.read(chunk.old + size, chunk.size - size)
            ):
                sec = parsed[chunk.path].sections[chunk.section - 1]
                if not any(
                    chunk.offset + size <= off < chunk.offset + chunk.size
                    for off, index, kind in sec.relocs
                ):
                    chunk.size = size
    # Identify remaining string and constant allocations by compiled contents.
    for chunk in chunks:
        if chunk.candidates:
            continue
        obj = parsed[chunk.path]
        sec = obj.sections[chunk.section - 1]
        payload = ours.read(chunk.old, chunk.size)
        wild = {
            off - chunk.offset + j
            for off, index, kind in sec.relocs
            if chunk.offset <= off < chunk.offset + chunk.size
            for j in range(4)
        }
        significant = bytes(b for i, b in enumerate(payload) if i not in wild)
        if len(significant) < 6 or not any(significant):
            continue
        # Include the actual allocation; do not depend on linker tail padding.
        n = len(payload)
        while n > 1 and payload[n - 1] == 0 and payload[n - 2] == 0:
            n -= 1
        n = min(n, 256)
        if n < 6:
            continue
        pattern = b"".join(
            b"." if i in wild else re.escape(bytes([b]))
            for i, b in enumerate(payload[:n])
        )
        names_to_search = [".rdata"] if chunk.readonly else [".data", ".text"]
        hits = []
        for name in names_to_search:
            va, vsize, rawptr, rawsize = orig.sections[name]
            hits.extend(
                va + m.start()
                for m in re.finditer(
                    pattern, orig.data[rawptr : rawptr + min(vsize, rawsize)], re.S
                )
            )
        if len(hits) == 1:
            chunk.candidates[hits[0]] += 1

    occupied: dict[int, Chunk] = {}
    for path in Path("build/layout/objects.txt").read_text().splitlines():
        placed = "build/obj_placed/" + Path(path).name
        obj = coff.Object(placed)
        for number, sec in enumerate(obj.sections, 1):
            if not sec.flags & coff.CODE or sec.flags & 0x800 or not sec.data:
                continue
            hit = next(
                (
                    s
                    for s in obj.externals()
                    if s.section == number and s.name in ourmap
                ),
                None,
            )
            if hit is None:
                continue
            address = ourmap[hit.name][0] - hit.value
            data = sec.data
            floor = max(
                [s.value + 1 for s in obj.symbols if s and s.section == number]
                + [off + 4 for off, index, kind in sec.relocs]
            )
            while len(data) > floor and data[-1] in (0x90, 0xCC):
                data = data[:-1]
            protected = Chunk(address, len(data), [hit.name], placed, number, 0, True)
            for x in range(address, address + len(data)):
                occupied[x] = protected
    conflicts: list[tuple[Chunk, int, Chunk]] = []
    placed: list[Chunk] = []
    for chunk in sorted(
        chunks, key=lambda c: (-max(c.candidates.values(), default=0), -c.size)
    ):
        for address, count in chunk.candidates.most_common():
            secname = orig.section_of(address)
            if secname not in (".rdata", ".data", ".text"):
                continue
            va, vsize, rawptr, rawsize = orig.sections[secname]
            if not va <= address or address + chunk.size > va + min(vsize, rawsize):
                continue
            overlap = next(
                (
                    occupied[x]
                    for x in range(address, address + chunk.size)
                    if x in occupied
                ),
                None,
            )
            if overlap is not None:
                conflicts.append((chunk, address, overlap))
                continue
            chunk.new = address
            for x in range(address, address + chunk.size):
                occupied[x] = chunk
            placed.append(chunk)
            break
    unplaced = [c for c in chunks if c.new is None]
    print(
        "DATA CHUNKS",
        len(chunks),
        "PLACED",
        len(placed),
        "BYTES",
        sum(c.size for c in placed),
        "UNPLACED",
        len(unplaced),
        "BYTES",
        sum(c.size for c in unplaced),
        flush=True,
    )
    Path("build/layout/data-conflicts.json").write_text(
        json.dumps(
            [(c.names, hex(a), p.names, c.size, p.size) for c, a, p in conflicts],
            indent=2,
        )
    )
    Path("build/layout/data-unplaced.json").write_text(
        json.dumps(
            [(c.names, c.size, hex(c.old), c.path, c.section) for c in unplaced],
            indent=2,
        )
    )
    Path("build/layout/data-plan.json").write_text(
        json.dumps(
            [
                {
                    "old": c.old,
                    "new": c.new,
                    "size": c.size,
                    "names": c.names,
                    "path": c.path,
                    "section": c.section,
                    "offset": c.offset,
                    "candidates": dict(c.candidates),
                }
                for c in chunks
            ],
            indent=2,
        )
    )

    # Preserve allocations without a recovered destination in unused initialized space.
    # These are compiler constants and metadata; their references are still relocated.
    for chunk in sorted(unplaced, key=lambda c: -c.size):
        candidates = [chunk.old]
        secname = ".rdata" if chunk.readonly else ".data"
        va, vsize, rawptr, rawsize = orig.sections[secname]
        candidates.extend(range(va, va + min(vsize, rawsize) - chunk.size + 1, 4))
        for address in candidates:
            if not va <= address or address + chunk.size > va + min(vsize, rawsize):
                continue
            if any(x in occupied for x in range(address, address + chunk.size)):
                continue
            chunk.new = address
            for x in range(address, address + chunk.size):
                occupied[x] = chunk
            break
        if chunk.new is None:
            raise ValueError(("No room for allocation", chunk))
    Path("build/layout/data-plan.json").write_text(
        json.dumps(
            [
                {
                    "old": c.old,
                    "new": c.new,
                    "size": c.size,
                    "names": c.names,
                    "path": c.path,
                    "section": c.section,
                    "offset": c.offset,
                    "candidates": dict(c.candidates),
                }
                for c in chunks
            ],
            indent=2,
        )
    )
