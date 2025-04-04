"""Copy files."""

from __future__ import annotations

import shutil
import sys
from typing import TYPE_CHECKING, TypedDict

import elpekenin_userspace.path
from elpekenin_userspace import error
from elpekenin_userspace.operations.base import BaseOperation

if TYPE_CHECKING:
    from typing import Literal

    from elpekenin_userspace.build import Recipe


class Cp(BaseOperation):
    """Copy files/directories."""

    class Args(TypedDict):
        """Arguments for this operation."""

        operation: Literal["cp"]
        src: str
        dst: str

    def __init__(
        self,
        _: Recipe,
        entry: Args,
    ) -> None:
        """Initialize an instance."""
        src = entry.get("src") or error.missing("src")
        self.src = elpekenin_userspace.path.resolve(src)

        dst = entry.get("dst") or error.missing("dst")
        self.dst = elpekenin_userspace.path.resolve(dst)

    def __str__(self) -> str:
        """Display this operation."""
        return f"Copying '{self.src.name}'"

    def run(self) -> int:
        """Entrypoint."""
        if self.src.is_dir():
            if self.dst.is_file():
                msg = "Can't copy directory into file"
                raise RuntimeError(msg)

            shutil.copytree(self.src, self.dst, dirs_exist_ok=True)
            return 0

        if self.src.is_file():
            shutil.copy(self.src, self.dst)
            return 0

        sys.stderr.write(f"Couldn't make sense of '{self.src}'\n")
        return 1
