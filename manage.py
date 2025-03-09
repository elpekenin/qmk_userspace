"""Utilities to interact with my userspace."""

from __future__ import annotations

import argparse
import logging
import sys
from pathlib import Path
from typing import TYPE_CHECKING

from commands import args
from commands.generate.features import Features
from commands.generate.keycode_str import KeycodeStr
from commands.generate.qp_resources import QpResources

if TYPE_CHECKING:
    from commands.base import BaseCommand


THIS = Path(__file__)
QMK = THIS.parent
LOG_FILE = "python.txt"

# TODO(elpekenin): docs, compile
SUBCOMMANDS: dict[str, type[BaseCommand]] = {
    "features": Features,
    "keycode_str": KeycodeStr,
    "qp_resources": QpResources,
}


def main() -> int:
    """Run a script's main logic, logging errors to file."""
    sys.path.append(str(QMK / "lib" / "python"))  # make `import qmk` work

    parser = argparse.ArgumentParser(
        prog=THIS.stem,
        description=__doc__,
    )

    # common args
    parser.add_argument(
        "--output",
        dest="output_directory",
        help="directory where to write generated files",
        type=args.directory,
        required=True,
    )
    parser.add_argument(
        "--log-folder",
        help="directory where to write logs",
        type=args.directory,
        required=True,
    )

    # script-specific args
    subparsers = parser.add_subparsers(dest="subcommand", required=True)

    for name, class_ in SUBCOMMANDS.items():
        subparser = subparsers.add_parser(name, help=class_.help())
        class_.add_args(subparser)

    # run logic
    arguments = parser.parse_args()

    logging.basicConfig(filename=arguments.log_folder / LOG_FILE)

    subcommand = SUBCOMMANDS[arguments.subcommand]
    return subcommand().run(arguments)


if __name__ == "__main__":
    try:
        ret = main()
    except Exception as e:
        logging.exception("Something went wrong trying to run the command", exc_info=e)
        raise
    else:
        raise SystemExit(ret)
