"""Project-specific standalone linker, loaded through build.hook."""

from __future__ import annotations

from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
from layout import link


def link_exe(objects: list[str], exe_objects: list[str], feats: list[str]) -> None:
    if objects != exe_objects or feats:
        raise ValueError(
            "The recovered standalone layout requires the vanilla VC5 objects"
        )
    link(exe_objects)
