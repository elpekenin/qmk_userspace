"""Custom types for commands' arguments."""

from __future__ import annotations

from argparse import ArgumentTypeError
from typing import TYPE_CHECKING, cast

import qmk_cli.helpers  # type: ignore[import-untyped]

import elpekenin_userspace.path

if TYPE_CHECKING:
    from pathlib import Path


class PathArg:
    """Shared logic between files and directories."""

    def __init__(
        self,
        *,
        require_existence: bool,
        name: str | None = None,
    ) -> None:
        """Initialize an instance."""
        self.require_existence = require_existence
        self.name = name

    def __call__(self, raw: str) -> Path:
        """Conversion function for arguments."""
        path = elpekenin_userspace.path.resolve(raw)

        if self.require_existence and not path.exists():
            msg = f"{path} does not exist"
            raise ArgumentTypeError(msg)

        if self.name is not None and self.name != path.name:
            msg = f"Expected name to be '{self.name}'"
            raise ArgumentTypeError(msg)

        return path.resolve()


class File(PathArg):
    """Represent a file argument."""

    def __call__(self, raw: str) -> Path:
        """Conversion function for an argument expected to be a file."""
        path = super().__call__(raw)

        if path.exists() and not path.is_file():
            msg = f"{path} is not a file"
            raise ArgumentTypeError(msg)

        return path.resolve()


class Directory(PathArg):
    """Represent a directory argument."""

    def __call__(self, raw: str) -> Path:
        """Conversion function for an argument expected to be a directory."""
        path = super().__call__(raw)

        if path.exists() and not path.is_dir():
            msg = f"{path} is not a directory"
            raise ArgumentTypeError(msg)

        return path.resolve()


def qmk(raw: str) -> Path | None:
    """Represent a directory argument, which is a copy of QMK."""
    if not raw:
        value = cast("Path", qmk_cli.helpers.find_qmk_firmware())
    else:
        value = elpekenin_userspace.path.resolve(raw)

    if not qmk_cli.helpers.is_qmk_firmware(value):
        return None

    return value
