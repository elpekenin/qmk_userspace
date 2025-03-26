"""Common logic."""

from __future__ import annotations

from abc import ABC, abstractmethod
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from elpekenin_userspace.build import Recipe
    from elpekenin_userspace.operations import Args


class SetupOperation(ABC):
    """Represent a setup operation, out of user control."""

    @abstractmethod
    def __init__(
        self,
        recipe: Recipe,
    ) -> None:
        """Initialize an instance."""

    @abstractmethod
    def run(self) -> int:
        """Logic of operation."""


class BaseOperation(ABC):
    """Represent an operation available over JSON."""

    @abstractmethod
    def __init__(
        self,
        recipe: Recipe,
        entry: Args,
    ) -> None:
        """Initialize an instance."""

    @abstractmethod
    def run(self) -> int:
        """Logic of operation."""
