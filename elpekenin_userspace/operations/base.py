"""Common logic."""

from __future__ import annotations

from abc import ABC, abstractmethod
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from typing_extensions import Self

    from elpekenin_userspace.build import Recipe
    from elpekenin_userspace.operations import Args
    from elpekenin_userspace.result import Result


class BaseOperation(ABC):
    """Represent every operation."""

    @abstractmethod
    def run(self) -> Result[None, str]:
        """Logic of operation."""
        raise NotImplementedError


class SetupOperation(ABC):
    """Represent a setup operation, out of user control."""

    @classmethod
    @abstractmethod
    def init_from(
        cls,
        recipe: Recipe,
    ) -> Result[Self, str]:
        """Initialize an instance."""
        raise NotImplementedError


class Operation(ABC):
    """Represent an operation available over JSON."""

    @classmethod
    @abstractmethod
    def init_from(
        cls,
        recipe: Recipe,
        entry: Args,
    ) -> Result[Self, str]:
        """Initialize an instance."""
        raise NotImplementedError
