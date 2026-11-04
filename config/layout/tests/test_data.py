"""Allocation boundaries and complete payloads constrain inferred destinations."""

from __future__ import annotations

from pathlib import Path
from tempfile import TemporaryDirectory
import struct
import sys
from typing import cast
import unittest
from unittest.mock import Mock

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools"))
sys.path.insert(0, str(ROOT))

import coff
import compare_exe as ce
from config.layout.data import (
    Chunk,
    fixed_reference,
    recover_crt_sections,
    recover_unused_pointers,
    recover_vftable_prefixes,
)


class UnusedPointerTests(unittest.TestCase):
    def setUp(self) -> None:
        self.payload = struct.pack("<I", 0x401000)
        self.ours = Mock(spec=ce.PE)
        self.ours.read.return_value = self.payload
        self.original = Mock(spec=ce.PE)
        self.original.section_of.return_value = ".text"
        self.original.data = self.payload + bytes(4) + self.payload
        self.original.sections = {".data": (0x460000, 12, 0, 12)}
        self.chunks: list[Chunk] = []
        self.objects: dict[str, coff.Object] = {}
        for index in range(2):
            path = f"build/obj_crt/filter{index}.obj"
            name = f"_filter{index}"
            obj = Mock(spec=coff.Object)
            obj.sections = [
                coff.Section(".data", bytes(4), [(0, 1, coff.DIR32)], coff.RDATA)
            ]
            obj.symbols = [
                coff.Symbol(name, 0, 1, 2),
                coff.Symbol("_handler", 0, 0, 2),
            ]
            self.objects[path] = cast(coff.Object, obj)
            self.chunks.append(Chunk(0x470000 + index * 4, 4, [name], path, 1, 0, False))

    def recover(self) -> list[Chunk]:
        return recover_unused_pointers(
            self.chunks, self.objects, self.ours, self.original, {}
        )

    def test_indistinguishable_unused_pointers_fill_their_remaining_slots(self) -> None:
        self.assertEqual(self.recover(), self.chunks)
        self.assertEqual([chunk.new for chunk in self.chunks], [0x460000, 0x460008])

    def test_extra_possible_destination_keeps_identity_ambiguous(self) -> None:
        self.original.data += self.payload
        self.original.sections[".data"] = (0x460000, 16, 0, 16)
        self.assertEqual(self.recover(), [])

    def test_referenced_pointer_needs_an_identity_binding(self) -> None:
        self.objects[self.chunks[0].path].sections.append(
            coff.Section(".text", bytes(4), [(0, 0, coff.DIR32)], coff.TEXT)
        )
        self.assertEqual(self.recover(), [])


class ReferenceOwnershipTests(unittest.TestCase):
    def test_fixed_state_is_not_an_initialized_allocation_hint(self) -> None:
        obj = Mock(spec=coff.Object)
        obj.sections = [
            coff.Section(".data", bytes(16), [], coff.RDATA),
            coff.Section(".bss", bytes(16), [], 0x80),
        ]
        obj.symbols = [
            coff.Symbol("_absolute", 0x46C2F0, coff.SYM_ABSOLUTE, 2),
            coff.Symbol("_local_state", 0, 2, 3),
            coff.Symbol("_external_state", 0, coff.SYM_UNDEFINED, 2),
            coff.Symbol("_common_state", 4, coff.SYM_UNDEFINED, 2),
            coff.Symbol("_initialized", 0, 1, 2),
        ]
        for index in range(4):
            with self.subTest(symbol=obj.symbols[index].name):
                self.assertTrue(
                    fixed_reference(cast(coff.Object, obj), index, {"_external_state"})
                )
        self.assertFalse(
            fixed_reference(cast(coff.Object, obj), 4, {"_external_state"})
        )


class VftablePrefixTests(unittest.TestCase):
    def setUp(self) -> None:
        directory = TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.path = str(Path(directory.name) / "vftable.obj")

    def recover(self, target: str) -> Chunk:
        coff.write_object(
            self.path,
            [(".rdata", coff.RDATA, bytes(8), [(0, target, coff.DIR32)])],
            {"??_7Example@@6B@": (1, 4)},
            [target],
            {},
        )
        prefix = Chunk(0x2000, 4, [], self.path, 1, 0, True)
        table = Chunk(0x2004, 4, ["??_7Example@@6B@"], self.path, 1, 4, True)
        table.candidates[0x3004] = 7
        recover_vftable_prefixes(
            [prefix, table], {self.path: coff.Object(self.path)}
        )
        return prefix

    def test_locator_follows_confirmed_vftable_destination(self) -> None:
        prefix = self.recover("??_R4Example@@6B@")
        self.assertEqual(prefix.candidates, {0x3000: 7})

    def test_unrelated_preceding_pointer_is_not_bound_by_adjacency(self) -> None:
        prefix = self.recover("_unrelated")
        self.assertEqual(prefix.candidates, {})


class CRTSectionTests(unittest.TestCase):
    def setUp(self) -> None:
        self.path = "build/obj_crt/sample.obj"
        self.payload = b"HEADANCHTAIL"
        self.obj = Mock(spec=coff.Object)
        self.obj.sections = [coff.Section(".data", self.payload, [], coff.RDATA)]
        self.ours = Mock(spec=ce.PE)
        self.ours.read.return_value = self.payload
        self.chunks = [
            Chunk(0x2000 + offset, 4, [name], self.path, 1, offset, False)
            for offset, name in ((0, "_head"), (4, "_anchor"), (8, "_tail"))
        ]
        self.chunks[1].candidates[0x3004] = 5

    def recover(self, payload: bytes) -> None:
        original = Mock(spec=ce.PE)
        original.section_of.return_value = ".data"
        original.read.return_value = payload
        recover_crt_sections(
            self.chunks,
            {self.path: self.obj},
            cast(ce.PE, self.ours),
            cast(ce.PE, original),
        )

    def test_middle_anchor_recovers_intact_section_neighbors(self) -> None:
        self.recover(self.payload)
        self.assertEqual(self.chunks[0].candidates, {0x3000: 1})
        self.assertEqual(self.chunks[1].candidates, {0x3004: 5})
        self.assertEqual(self.chunks[2].candidates, {0x3008: 1})

    def test_differing_neighbor_rejects_section_layout_inference(self) -> None:
        self.recover(b"HEADANCHDIFF")
        self.assertEqual(self.chunks[0].candidates, {})
        self.assertEqual(self.chunks[2].candidates, {})

    def test_self_relative_pointer_is_rebased_before_comparing(self) -> None:
        self.obj.sections[0].data = bytes(4) + self.payload[4:]
        self.obj.sections[0].relocs = [(0, 0, coff.DIR32)]
        self.obj.symbols = [coff.Symbol("_tail", 8, 1, 2)]
        self.ours.read.return_value = struct.pack("<I", 0x2008) + self.payload[4:]
        self.recover(struct.pack("<I", 0x3008) + self.payload[4:])
        self.assertEqual(self.chunks[0].candidates, {0x3000: 1})
        self.assertEqual(self.chunks[2].candidates, {0x3008: 1})

    def test_truncated_target_cannot_confirm_section(self) -> None:
        self.recover(self.payload[:-1])
        self.assertEqual(self.chunks[0].candidates, {})

    def test_truncated_linked_section_is_rejected(self) -> None:
        self.ours.read.return_value = self.payload[:-1]
        with self.assertRaisesRegex(ValueError, "Truncated CRT initialized section"):
            self.recover(self.payload)


if __name__ == "__main__":
    unittest.main()
