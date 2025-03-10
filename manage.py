"""Utilities to interact with my userspace."""

from __future__ import annotations

import argparse
import logging
from pathlib import Path
from typing import TYPE_CHECKING

from commands import args
from commands.docs import Docs
from commands.generate.features import Features
from commands.generate.keycode_str import KeycodeStr
from commands.generate.qp_resources import QpResources

if TYPE_CHECKING:
    from commands.base import BaseCommand


THIS = Path(__file__)
QMK = THIS.parent
LOG_FILE = "python.txt"

SUBCOMMANDS: dict[str, type[BaseCommand]] = {
    "docs": Docs,
    "features": Features,
    "keycode_str": KeycodeStr,
    "qp_resources": QpResources,
}


def main() -> int:
    """Run a script's main logic, logging errors to file."""
    parser = argparse.ArgumentParser(
        prog=THIS.stem,
        description=__doc__,
    )

    # common args
    parser.add_argument(
        "--log",
        help="file where to write logs",
        metavar="FILE",
        type=args.file,
        required=False,
        default=None,
    )

    # script-specific args
    subparsers = parser.add_subparsers(dest="subcommand")
    for name, class_ in SUBCOMMANDS.items():
        subparser = subparsers.add_parser(name, help=class_.help())
        class_.add_args(subparser)

    # run logic
    arguments = parser.parse_args()

    log: Path | None = arguments.log
    logging.basicConfig(filename=log)

    subcommand_name: str | None = arguments.subcommand
    if subcommand_name is None:
        parser.print_help()
        return 1

    subcommand = SUBCOMMANDS[subcommand_name]
    return subcommand().run(arguments)


if __name__ == "__main__":
    try:
        ret = main()
    except Exception as e:
        logging.exception("Something went wrong trying to run the command", exc_info=e)
        raise
    else:
        raise SystemExit(ret)
