"""Build step."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path
from typing import TYPE_CHECKING, TypedDict

import elpekenin_userspace.path
from elpekenin_userspace.operations.base import Operation
from elpekenin_userspace.result import Err, Ok, missing

if TYPE_CHECKING:
    from typing import Literal

    from typing_extensions import NotRequired, Self

    from elpekenin_userspace.build import Recipe
    from elpekenin_userspace.result import Result


class Exec(Operation):
    """Run a command."""

    class Args(TypedDict):
        """Arguments for this operation."""

        operation: Literal["exec"]
        cmd: str
        path: NotRequired[str]
        display: NotRequired[str]

    def __init__(
        self,
        path: Path,
        cmd: str,
        display: str,
    ) -> None:
        """Initialize an instance."""
        self.path = path
        self.cmd = cmd
        self.display = display

    @classmethod
    def init_from(
        cls,
        _: Recipe,
        entry: Args,  # type: ignore[override]  # is not the Union of them all
    ) -> Result[Self, str]:
        """Initialize an instance."""
        raw = entry.get("path")
        path = Path.cwd() if raw is None else elpekenin_userspace.path.resolve(raw)

        cmd = entry.get("cmd")
        if cmd is None:
            return missing("cmd")

        display = entry.get("display") or cmd

        return Ok(
            cls(
                path,
                cmd,
                display,
            ),
        )

    def __str__(self) -> str:
        """Display this operation."""
        return self.display

    def run(self) -> Result[None, str]:
        """Entrypoint."""
        process = subprocess.run(  # noqa: S602
            self.cmd,
            cwd=self.path,
            shell=True,
            capture_output=True,
            check=False,
        )

        if process.returncode == 0:
            return Ok(None)

        sys.stdout.write(process.stdout.decode())
        sys.stderr.write(process.stderr.decode())

        return Err("Subprocess failed")
