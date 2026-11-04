"""Relocation invariants and the built executable's public layout contract."""

from __future__ import annotations

from pathlib import Path
import json
import struct
import sys
import unittest

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools"))
sys.path.insert(0, str(ROOT))

from config.layout.validation import (
    require_coverage,
    require_payload,
    require_unchanged,
)
import compare_exe


class CoverageTests(unittest.TestCase):
    def test_zero_alignment_gaps_need_no_allocation(self) -> None:
        require_coverage(b"GUID\0\0value", 0x1000, [(0x1000, 4), (0x1006, 5)])

    def test_import_library_payload_cannot_be_omitted(self) -> None:
        with self.assertRaisesRegex(ValueError, "0x1010"):
            require_coverage(bytes(16) + b"GUID", 0x1000, [(0x1000, 16)])

    def test_overlapping_aliases_are_covered(self) -> None:
        require_coverage(b"abcdef", 0x1000, [(0x1000, 4), (0x1002, 4)])


class PayloadTests(unittest.TestCase):
    def test_relocate_pointer_without_changing_its_neighbor(self) -> None:
        before = b"AB" + struct.pack("<I", 0x464000) + b"CD"
        after = b"AB" + struct.pack("<I", 0x466000) + b"CD"
        require_payload(before, after, {2: 0x466000})

    def test_unrecorded_data_change_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "outside relocations"):
            require_payload(b"ABCD", b"ABXD", {})

    def test_partial_relocation_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "outside allocation"):
            require_payload(bytes(4), bytes(4), {1: 0x400000})

    def test_truncated_payload_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "outside relocations"):
            require_payload(b"ABCD", b"ABC", {})


class InstructionTests(unittest.TestCase):
    def test_address_operand_and_inline_data_can_change(self) -> None:
        before = b"\xa1" + bytes(4) + b"\xc3" + bytes(4)
        after = b"\xa1" + struct.pack("<I", 0x526320) + b"\xc3DATA"
        require_unchanged(before, after, [(1, 4), (6, 4)])

    def test_opcode_change_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "Instruction bytes changed"):
            require_unchanged(b"\xa1\0\0\0\0", b"\xe9\0\0\0\0", [(1, 4)])

    def test_code_truncation_is_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "changed size"):
            require_unchanged(b"\xc3\x90", b"\xc3", [])


@unittest.skipUnless(
    (ROOT / "build/tomb2.exe").exists() and (ROOT / "build/orig/tomb2.exe").exists(),
    "Build the standalone executable to check its layout",
)
class ExecutableTests(unittest.TestCase):
    def setUp(self) -> None:
        self.ours = compare_exe.PE(ROOT / "build/tomb2.exe")
        self.original = compare_exe.PE(ROOT / "build/orig/tomb2.exe")

    def test_whole_file_parity_reaches_96_percent(self) -> None:
        denominator = max(len(self.ours.data), len(self.original.data))
        identical = compare_exe.same(self.ours.data, self.original.data)
        self.assertGreaterEqual(identical / denominator, 0.96)

    def test_recovered_runtime_state_retains_original_addresses(self) -> None:
        mapping = compare_exe.read_map(str(ROOT / "build/tomb2_exe.map"))
        for name, address in {
            "___fastflag": 0x51C298,
            "__mbctype": 0x51C570,
            "__crtheap": 0x527504,
            "___onexitbegin": 0x527510,
            "__bufin": 0x5263E0,
        }.items():
            with self.subTest(symbol=name):
                self.assertEqual(mapping[name][0], address)

    def test_runtime_blocks_match_and_retain_original_addresses(self) -> None:
        blocks = json.loads((ROOT / "build/layout/crt-code.json").read_text())
        for block in blocks:
            with self.subTest(path=block["path"], section=block["section"]):
                self.assertTrue(block["matched"])
                self.assertEqual(0x459030 + block["offset"], block["original"])

    def test_runtime_exception_scope_tables_survive_code_merging(self) -> None:
        for site in (
            0x459FF6,
            0x45BA86,
            0x45BBF6,
            0x45BD76,
            0x45BF86,
            0x45C086,
            0x45C116,
        ):
            with self.subTest(site=hex(site)):
                self.assertEqual(self.ours.read(site, 4), self.original.read(site, 4))

    def test_runtime_relocations_preserve_negative_and_end_pointer_addends(
        self,
    ) -> None:
        for site in (0x45C70D, 0x45C7A1, 0x45CCFE, 0x4604AA):
            with self.subTest(site=hex(site)):
                self.assertEqual(self.ours.read(site, 4), self.original.read(site, 4))

    def test_import_abi_includes_all_original_slots(self) -> None:
        self.assertEqual(self.ours.imports(), self.original.imports())
        self.assertEqual(self.ours.directory(12), self.original.directory(12))

    def test_import_address_table_preserves_separate_lookup_tokens(self) -> None:
        rva, size = self.original.directory(12)
        self.assertEqual(
            self.ours.read(self.ours.base + rva, size),
            self.original.read(self.original.base + rva, size),
        )

    def test_original_sections_retain_addresses_and_file_offsets(self) -> None:
        for name in self.original.order:
            with self.subTest(section=name):
                self.assertEqual(self.ours.sections[name], self.original.sections[name])

    def test_headers_match_original(self) -> None:
        self.assertEqual(self.ours.data[:0x400], self.original.data[:0x400])

    def test_runtime_state_needs_no_additional_section(self) -> None:
        self.assertEqual(self.ours.order, self.original.order)
        self.assertEqual(self.ours.sizeof_image, self.original.sizeof_image)
        self.assertEqual(self.ours.directory(5), (0, 0))


if __name__ == "__main__":
    unittest.main()
