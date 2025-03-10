"""Utilities to interact with my userspace."""

from __future__ import annotations

import argparse
import logging
import sys
from pathlib import Path
from typing import TYPE_CHECKING

import qmk_cli.helpers  # type: ignore[import-untyped]

from commands import args
from commands.docs import Docs
from commands.generate.features import Features
from commands.generate.keycode_str import KeycodeStr
from commands.generate.qp_resources import QpResources

if TYPE_CHECKING:
    from commands.base import BaseCommand


THIS = Path(__file__)
QMK = THIS.parent

SUBCOMMANDS: dict[str, type[BaseCommand]] = {
    "docs": Docs,
    "features": Features,
    "keycode_str": KeycodeStr,
    "qp_resources": QpResources,
}


def find_qmk(arg: Path | None) -> Path:
    """Return argument if received, otherwise try and find QMK."""
    ret = arg or qmk_cli.helpers.find_qmk_firmware()
    if not qmk_cli.helpers.is_qmk_firmware(ret):
        msg = "Could not find QMK's home"
        raise RuntimeError(msg)

    return ret


def main() -> int:
    """Run a script's main logic, logging errors to file."""
    parser = argparse.ArgumentParser(
        prog=THIS.stem,
        description=__doc__,
    )

    # common args
    parser.add_argument(
        "--qmk",
        help="path of your qmk_firmware (needed to make `import qmk` work)",
        metavar="DIR",
        type=args.Directory(require_existence=True),
        required=False,
    )

    parser.add_argument(
        "--log",
        help="file where to write logs",
        metavar="FILE",
        type=args.File(require_existence=False),
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

    lib = find_qmk(arguments.qmk) / "lib" / "python"
    sys.path.append(str(lib))

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
