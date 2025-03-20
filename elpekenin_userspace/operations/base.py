"""Common logic."""

from __future__ import annotations

from abc import ABC, abstractmethod
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from typing import Any

    from elpekenin_userspace.build import Recipe


class BaseOperation(ABC):
    """Represent an operation."""

    @abstractmethod
    def __init__(
        self,
        recipe: Recipe,
        entry: dict[str, Any],
    ) -> None:
        """Initialize an instance."""

    @abstractmethod
    def run(self) -> int:
        """Logic of operation."""
