"""Common logic for every command."""

from __future__ import annotations

from abc import ABC, abstractmethod
from typing import TYPE_CHECKING, final

from elpekenin_userspace import args

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

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """For commands to add their specific arguments."""
        _ = parser  # by default: no-op

    @abstractmethod
    def run(self, arguments: Namespace) -> int:
        """Logic of command, returns exitcode."""


class CodegenCommand(BaseCommand):
    """Common logic to all commands generating code."""

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """Register codegen-specific arguments."""
        parser.add_argument(
            "--output",
            dest="output_directory",
            help="directory where to write generated files",
            metavar="DIR",
            type=args.Directory(require_existence=True),
            required=True,
        )
        return super().add_args(parser)
