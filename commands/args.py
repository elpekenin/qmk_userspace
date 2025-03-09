"""Custom types for commands' arguments."""

from __future__ import annotations

from argparse import ArgumentTypeError
from pathlib import Path


def _path_or_raise(raw: str) -> Path:
    """Return the Path if it exists, raise otherwise."""
    path = Path(raw).resolve()
    if path.exists():
        return path

    msg = f"{path} does not exist"
    raise ArgumentTypeError(msg)


def file(raw: str) -> Path:
    """Conversion function for and argument expected to be a file."""
    path = _path_or_raise(raw)
    if not path.is_file():
        msg = f"{path} is not a file"
        raise ArgumentTypeError(msg)

    return path


def directory(raw: str) -> Path:
    """Conversion function for and argument expected to be a directory."""
    path = _path_or_raise(raw)
    if not path.is_dir():
        msg = f"{path} is not a directory"
        raise ArgumentTypeError(msg)

    return path
