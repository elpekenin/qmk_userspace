"""Custom types for commands' arguments."""

from __future__ import annotations

from argparse import ArgumentTypeError
from pathlib import Path


class _Common:
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
        path = Path(raw)
        if self.require_existence and not path.exists():
            msg = f"{path} does not exist"
            raise ArgumentTypeError(msg)

        if self.name is not None and self.name != path.name:
            msg = f"Expected name to be '{self.name}'"
            raise ArgumentTypeError(msg)

        return path.resolve()


class File(_Common):
    """Represent a file argument."""

    def __call__(self, raw: str) -> Path:
        """Conversion function for an argument expected to be a file."""
        path = super().__call__(raw)

        if not path.is_file():
            msg = f"{path} is not a file"
            raise ArgumentTypeError(msg)

        return path.resolve()


class Directory(_Common):
    """Represent a directory argument."""

    def __call__(self, raw: str) -> Path:
        """Conversion function for an argument expected to be a file."""
        path = super().__call__(raw)

        if not path.is_dir():
            msg = f"{path} is not a directory"
            raise ArgumentTypeError(msg)

        return path.resolve()
