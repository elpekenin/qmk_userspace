"""Build step."""

from __future__ import annotations

from typing import TYPE_CHECKING, TypedDict

from elpekenin_userspace.operations.base import Operation
from elpekenin_userspace.result import Err, Ok

if TYPE_CHECKING:
    from typing import Literal

    from typing_extensions import Self

    from elpekenin_userspace.build import Recipe
    from elpekenin_userspace.result import Result


class Stop(Operation):
    """Stop execution."""

    class Args(TypedDict):
        """Arguments for this operation."""

        operation: Literal["stop"]

    @classmethod
    def init_from(
        cls,
        _r: Recipe,
        _a: Args,  # type: ignore[override]  # is not the Union of them all
    ) -> Result[Self, str]:
        """Initialize an instance."""
        return Ok(cls())

    def __str__(self) -> str:
        """Display this operation."""
        return "stop"

    def run(self) -> Result[None, str]:
        """Entrypoint."""
        return Err("Stopping...")
