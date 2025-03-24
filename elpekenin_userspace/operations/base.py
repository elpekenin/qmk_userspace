"""Common logic."""

from __future__ import annotations

from abc import ABC, abstractmethod
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from elpekenin_userspace.build import Recipe
    from elpekenin_userspace.operations import Args


class BaseOperation(ABC):
    """Represent an operation."""

    def __init__(
        self,
        recipe: Recipe,
        entry: Args,
    ) -> None:
        """Initialize an instance."""
        raise NotImplementedError

    @abstractmethod
    def run(self) -> int:
        """Logic of operation."""
