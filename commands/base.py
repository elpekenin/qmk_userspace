"""Common logic for any command."""

from __future__ import annotations

from abc import ABC, abstractmethod
from typing import TYPE_CHECKING, final

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace


class BaseCommand(ABC):
    """Represent a command."""

    @final
    @classmethod
    def help(cls) -> str:
        """Help message for a command."""
        doc = cls.__doc__
        if doc is None:
            return "<No help provided>"

        return doc.lower().rstrip(".")

    @staticmethod
    def add_args(parser: ArgumentParser) -> None:
        """For commands to add their specific arguments."""
        _ = parser  # by default: no-op

    @abstractmethod
    def run(self, arguments: Namespace) -> int:
        """Logic of command, returns exitcode."""
        raise NotImplementedError
