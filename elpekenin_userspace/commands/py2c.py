"""Subcommand to convert a python file to the equivalent const char * in C."""

from __future__ import annotations

from typing import TYPE_CHECKING

from elpekenin_userspace import args
from elpekenin_userspace.commands import BaseCommand

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from pathlib import Path

REPLACEMENTS: tuple[tuple[str, str], ...] = (
    ('"', '\\"'),
    ("\n", "\\n"),
)


def generate_c_file(file: Path) -> None:
    """Convert a single file."""
    assert file.is_file()  # noqa: S101  # defensive programming

    code = file.read_text()
    for old, new in REPLACEMENTS:
        code = code.replace(old, new)

    py = file.with_suffix(".c")
    py.write_text(
        f'static const char {file.stem}[] = "{code}";\n',
    )


class Py2C(BaseCommand):
    """Generate C-string equivalent of a .py file."""

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """Command-specific arguments."""
        parser.add_argument(
            "files",
            help="the Python files to be converted",
            metavar="FILE",
            type=args.File(require_existence=True),
            nargs="*",
        )
        return super().add_args(parser)

    def run(self, arguments: Namespace) -> int:
        """Entrypoint."""
        files: list[Path] = arguments.files
        if not files:
            msg = "No file(s) provided."
            raise RuntimeError(msg)

        for file in files:
            generate_c_file(file)

        return 0
