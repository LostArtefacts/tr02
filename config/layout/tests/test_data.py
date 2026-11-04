"""The RTTI locator and vftable remain one ABI allocation."""

from __future__ import annotations

from pathlib import Path
from tempfile import TemporaryDirectory
import sys
import unittest

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools"))
sys.path.insert(0, str(ROOT))

import coff
from config.layout.data import Chunk, recover_vftable_prefixes


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


if __name__ == "__main__":
    unittest.main()
