"""Copy files."""

from __future__ import annotations

import shutil
import sys
from typing import TYPE_CHECKING, TypedDict

import elpekenin_userspace.path
from elpekenin_userspace.operations.base import Operation
from elpekenin_userspace.result import Err, Ok, is_err, missing

if TYPE_CHECKING:
    from pathlib import Path
    from typing import Literal

    from typing_extensions import Self

    from elpekenin_userspace.build import Recipe
    from elpekenin_userspace.result import Result


def copy(src: Path, dst: Path) -> Result[None, str]:
    """Copy a single file/directory from ``src`` to ``dst``."""
    # if dst doesn't exist, use shutil to copy files
    if not dst.exists():
        if src.is_file():
            shutil.copy(src, dst)
        else:
            shutil.copytree(src, dst)

        return Ok(None)

    # else, copy manually, skipping files that haven't changed

    # dir -> dir, copy children
    if src.is_dir() and dst.is_dir():
        for child in src.iterdir():
            res = copy(child, dst / child.relative_to(src))
            if is_err(res):
                return res

        return Ok(None)

    # file -> file
    if src.is_file() and dst.is_file():
        src_stat = src.stat()
        dst_stat = dst.stat()

        src_ts = src_stat.st_mtime
        dst_ts = dst_stat.st_mtime

        # if src is newer, copy it
        if src_ts > dst_ts:
            shutil.copy2(src, dst)

        # shouldn't edit in build dir
        elif src_ts < dst_ts:
            if src_stat.st_size != dst_stat.st_size:
                return Err(f"File '{src}' was changed in destination")

            sys.stdout.write(f"'{src}' has newer mtime but same size, not copying it\n")

        return Ok(None)

    # other combinations, unsupported
    return Err("Unreachable `src`/`dst` combination")


class Cp(Operation):
    """Copy files/directories."""

    class Args(TypedDict):
        """Arguments for this operation."""

        operation: Literal["cp"]
        src: str
        dst: str

    def __init__(
        self,
        src: Path,
        dst: Path,
    ) -> None:
        """Initialize an instance."""
        self.src = src
        self.dst = dst

    @classmethod
    def init_from(
        cls,
        _: Recipe,
        entry: Args,  # type: ignore[override]  # is not the Union of them all
    ) -> Result[Self, str]:
        """Initialize an instance."""
        src = entry.get("src")
        if src is None:
            return missing("src")

        dst = entry.get("dst")
        if dst is None:
            return missing("dst")

        return Ok(
            cls(
                elpekenin_userspace.path.resolve(src),
                elpekenin_userspace.path.resolve(dst),
            ),
        )

    def __str__(self) -> str:
        """Display this operation."""
        return f"Copying '{self.src.name}'"

    def run(self) -> Result[None, str]:
        """Entrypoint."""
        return copy(self.src, self.dst)
