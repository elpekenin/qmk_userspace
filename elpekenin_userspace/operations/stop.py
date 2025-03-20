"""Build step."""

from __future__ import annotations

import sys
from typing import TYPE_CHECKING

from elpekenin_userspace.operations.base import BaseOperation

if TYPE_CHECKING:
    from typing import Any

    from elpekenin_userspace.build import Recipe


class Stop(BaseOperation):
    """Stop execution."""

    def __init__(
        self,
        _r: Recipe,
        _e: dict[str, Any],
    ) -> None:
        """Initialize an instance."""

    def __str__(self) -> str:
        """Display this operation."""
        return "stop"

    def run(self) -> int:
        """Entrypoint."""
        sys.stderr.write("Stopping...\n")
        return 1
