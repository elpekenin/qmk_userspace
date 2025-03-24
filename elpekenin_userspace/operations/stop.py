"""Build step."""

from __future__ import annotations

import sys
from typing import TYPE_CHECKING, TypedDict

from elpekenin_userspace.operations.base import BaseOperation

if TYPE_CHECKING:
    from typing import Literal

    from elpekenin_userspace.build import Recipe


class Stop(BaseOperation):
    """Stop execution."""

    class Args(TypedDict):
        """Arguments for this operation."""

        operation: Literal["stop"]

    def __init__(
        self,
        _r: Recipe,
        _e: Args,
    ) -> None:
        """Initialize an instance."""

    def __str__(self) -> str:
        """Display this operation."""
        return "stop"

    def run(self) -> int:
        """Entrypoint."""
        sys.stderr.write("Stopping...\n")
        return 1
