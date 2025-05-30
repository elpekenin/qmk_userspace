"""Common logic for every command."""

from __future__ import annotations

import argparse
from abc import ABC, abstractmethod
from typing import TYPE_CHECKING, final

from elpekenin_userspace import args

if TYPE_CHECKING:
    from elpekenin_userspace.result import Result


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
    def add_args(cls, parser: argparse.ArgumentParser) -> None:
        """For commands to add their specific arguments."""
        _ = parser  # by default: no-op

    @abstractmethod
    def run(self, arguments: argparse.Namespace) -> Result[None, str]:
        """Logic of command, returns exitcode."""


class CodegenCommand(BaseCommand):
    """Common logic to all commands generating code."""

    @classmethod
    def add_args(cls, parser: argparse.ArgumentParser) -> None:
        """Register codegen-specific arguments."""
        parser.add_argument(
            "--output",
            dest="output_directory",
            help="directory where to write generated files",
            default=argparse.SUPPRESS,
            metavar="DIR",
            type=args.Directory(require_existence=True),
            required=True,
        )
        return super().add_args(parser)
