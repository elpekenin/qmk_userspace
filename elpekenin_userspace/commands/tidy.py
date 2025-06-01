"""Subcommand to run clang-tidy and filter-out harmless issues."""

from __future__ import annotations

import subprocess
import sys
from typing import TYPE_CHECKING

from elpekenin_userspace import args
from elpekenin_userspace.commands import BaseCommand
from elpekenin_userspace.result import Err, Ok

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace

    from elpekenin_userspace.result import Result

WHITELIST = {
    "redefining builtin macro": 0,
    "'linker' input unused": 0,
    "'-mno-thumb-interwork'": 0,
    "'-mpoke-function-name'": 0,
    "'-fsanitize=kernel-address'": 0,
    # shows 'previously defined here'
    "__' macro redefined": 1,
    "'__ARM_ARCH_PROFILE' macro redefined": 1,
    "'__ARM_SIZEOF_MINIMAL_ENUM' macro redefined": 1,
    "'__GXX_ABI_VERSION' macro redefined": 1,
    "'__INT32_C' macro redefined": 1,
    "'__INT64_C' macro redefined": 1,
    "'__INTMAX_C' macro redefined": 1,
    "'__UINT32_C' macro redefined": 1,
    "'__UINT64_C' macro redefined": 1,
    "'__UINTMAX_C' macro redefined": 1,
}
"""Some 'errors' we dont care about, and how many lines to ignore after them."""


def handle_output(process: subprocess.Popen[bytes]) -> Result[None, str]:
    """Handle streaming of process' output, printing it."""
    errors = False

    line = ""
    lines_to_skip = 0

    if process.stdout is None:
        return Err("stdout not captures")

    for raw_line in iter(process.stdout.readline, b""):
        if lines_to_skip > 0:
            lines_to_skip -= 1
            continue

        line = raw_line.decode()

        in_whitelist = False
        for pattern, offset in WHITELIST.items():
            if pattern in line:
                lines_to_skip = offset
                in_whitelist = True
                break

        if in_whitelist:
            continue

        sys.stdout.write(line)

    if errors:
        return Err("Problems found")

    return Ok(None)


class Tidy(BaseCommand):
    """Run clang-tidy and filter its output."""

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """Command-specific arguments."""
        parser.add_argument(
            "--config",
            help="configuration file",
            metavar="FILE",
            type=args.File(require_existence=True),
            default=".clang-tidy",
        )
        return super().add_args(parser)

    def run(self, arguments: Namespace) -> Result[None, str]:
        """Entrypoint."""
        use_color = "1" if sys.stdout.isatty() else "0"

        cmd = [
            "run-clang-tidy-21",
            ".",
            *("-source-filter", "(.*)(users|modules)/elpekenin/(.*)"),
            *("-config-file", arguments.config),
            *("-use-color", use_color),
        ]

        with subprocess.Popen(
            cmd,
            stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
        ) as process:
            while True:
                returncode = process.poll()
                if returncode is not None:
                    break

                output = handle_output(process)

        if returncode == 0:
            return Ok(None)

        return output
