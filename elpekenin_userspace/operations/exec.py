"""Build step."""

from __future__ import annotations

import subprocess
from pathlib import Path
from typing import TYPE_CHECKING, TypedDict

from elpekenin_userspace import error
from elpekenin_userspace.operations.base import BaseOperation

if TYPE_CHECKING:
    from typing import Literal

    from typing_extensions import NotRequired

    from elpekenin_userspace.build import Recipe


class Exec(BaseOperation):
    """Run a command."""

    class Args(TypedDict):
        """Arguments for this operation."""

        operation: Literal["exec"]
        cmd: str
        path: NotRequired[str]
        display: NotRequired[str]

    def __init__(
        self,
        _: Recipe,
        entry: Args,
    ) -> None:
        """Initialize an instance."""
        path = entry.get("path")
        if path is None:
            self.path = Path.cwd()
        else:
            self.path = Path(path)

        self.cmd = entry.get("cmd") or error.missing("cmd")
        self.display = entry.get("display") or self.cmd

    def __str__(self) -> str:
        """Display this operation."""
        return self.display

    def run(self) -> int:
        """Entrypoint."""
        subprocess.run(self.cmd, cwd=self.path, check=True, shell=True)  # noqa: S602
        return 0
