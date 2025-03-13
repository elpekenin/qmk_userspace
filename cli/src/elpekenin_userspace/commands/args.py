"""Custom types for commands' arguments."""

from __future__ import annotations

from argparse import ArgumentTypeError
from pathlib import Path


class File:
    """Represent a file argument."""

    def __init__(self, *, require_existence: bool) -> None:
        """Initialize an instance."""
        self.require_existence = require_existence

    def __call__(self, raw: str) -> Path:
        """Conversion function for and argument expected to be a file."""
        path = Path(raw)
        if not path.exists():
            if self.require_existence:
                msg = f"{path} does not exist"
                raise ArgumentTypeError(msg)

            return path.resolve()

        if not path.is_file():
            msg = f"{path} is not a file"
            raise ArgumentTypeError(msg)

        return path.resolve()


class Directory:
    """Represent a directory argument."""

    def __init__(self, *, require_existence: bool) -> None:
        """Initialize an instance."""
        self.require_existence = require_existence

    def __call__(self, raw: str) -> Path:
        """Conversion function for and argument expected to be a file."""
        path = Path(raw)
        if not path.exists():
            if self.require_existence:
                msg = f"{path} does not exist"
                raise ArgumentTypeError(msg)

            return path.resolve()

        if not path.is_dir():
            msg = f"{path} is not a directory"
            raise ArgumentTypeError(msg)

        return path.resolve()
