"""Subcommand to generate stub files from C-MicroPython source."""

from __future__ import annotations

from typing import TYPE_CHECKING

from elpekenin_userspace import args
from elpekenin_userspace.codegen import PY_HEADER, lines
from elpekenin_userspace.commands import BaseCommand

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from pathlib import Path

DOCSTRING = "//!"
CONTENT = "//|"


class Pyi:
    """Class to convert C->Pyi."""

    def __init__(self) -> None:
        """Initialize an instance."""
        self.docstring: list[str] = []
        self.content: list[str] = []

    def get_docstring(self) -> str | None:
        """Get the processed docstring."""
        if not self.docstring:
            return None

        if len(self.docstring) == 1:
            return self.docstring[0]

        # trailing newline to make formatter happy
        return lines(*self.docstring) + "\n"

    def get_content(self) -> str | None:
        """Get the processed contents."""
        if not self.content:
            return None

        return lines(*self.content)

    def is_empty(self) -> bool:
        """Whether C file contained any special comment."""
        return not (self.docstring or self.content)

    @staticmethod
    def strip_marker(line: str, marker: str) -> str | None:
        """Remove special marker comments from line."""
        if not line.startswith(marker):
            return None

        offset = len(marker)

        # empty line, just the marker
        if len(line) == offset:
            return ""

        if line[offset] != " ":
            msg = "Whitespace between marker and content in mandatory."
            raise RuntimeError(msg)

        return line[offset + 1 :]

    def process(self, line: str) -> None:
        """Consume a line of C content."""
        for marker, target in (
            (DOCSTRING, self.docstring),
            (CONTENT, self.content),
        ):
            text = self.strip_marker(line, marker)
            if text is None:
                continue

            if marker is DOCSTRING and self.content:
                msg = "Docstring comments must precede any content ones."
                raise RuntimeError(msg)

            target.append(text)
            return


def generate_pyi_file(input_file: Path) -> None:
    """Create contents for PYI file."""
    assert input_file.is_file()  # noqa: S101  # defensive programming

    pyi = Pyi()
    with input_file.open() as f:
        for line in f.readlines():
            pyi.process(line.strip())

    if pyi.is_empty():
        return

    code = ""

    docstring = pyi.get_docstring()
    if docstring is not None:
        code += f'"""{docstring}"""\n\n'

    content = pyi.get_content()
    if content is not None:
        code += content

    output_directory = input_file.parent / "stubs"
    output_directory.mkdir(exist_ok=True)

    output_file = output_directory / (input_file.stem + ".pyi")
    output_file.write_text(
        lines(
            PY_HEADER,
            "",
            f"{code}",
        ).format(code=code),
    )


class Stubs(BaseCommand):
    """Generate .pyi equivalent of a .c source file."""

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """Command-specific arguments."""
        parser.add_argument(
            "files",
            help="the C file to generate stubs for",
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
            generate_pyi_file(file)

        return 0
