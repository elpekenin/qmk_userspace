"""Utilities to emit errors."""

from typing import NoReturn


def abort(msg: str) -> NoReturn:
    """Display generic error."""
    raise RuntimeError(msg)


def missing(key: str) -> NoReturn:
    """Display error about missing key on a dict."""
    abort(f"Missing required key '{key}'")
