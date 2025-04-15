# PYTHON_ARGCOMPLETE_OK

"""Utilities to interact with my userspace."""

from __future__ import annotations

import argparse
import sys
from typing import TYPE_CHECKING

try:
    argcomplete: ModuleType | None
    import argcomplete
except ImportError:
    argcomplete = None

from elpekenin_userspace import args
from elpekenin_userspace.commands.build import Build
from elpekenin_userspace.commands.docs import Docs
from elpekenin_userspace.commands.features import Features
from elpekenin_userspace.commands.keycode_str import KeycodeStr
from elpekenin_userspace.commands.micropython import Micropython
from elpekenin_userspace.commands.py2c import Py2C
from elpekenin_userspace.commands.qp_resources import QpResources
from elpekenin_userspace.commands.stubs import Stubs
from elpekenin_userspace.result import is_err

if TYPE_CHECKING:
    from types import ModuleType

    from elpekenin_userspace.commands import BaseCommand


SUBCOMMANDS: dict[str, type[BaseCommand]] = {
    "build": Build,
    "docs": Docs,
    "features": Features,
    "keycode_str": KeycodeStr,
    "micropython": Micropython,
    "qp_resources": QpResources,
    "py2c": Py2C,
    "stubs": Stubs,
}


def get_parser() -> argparse.ArgumentParser:
    """Create the parser."""
    parser = argparse.ArgumentParser(
        description=__doc__,
    )

    # common args
    parser.add_argument(
        "--qmk",
        help="path to your qmk_firmware (needed to make `import qmk` work)",
        metavar="DIR",
        type=args.qmk,
        required=False,
        default=args.qmk(None),
    )

    # script-specific args
    subparsers = parser.add_subparsers(dest="subcommand")
    for name, class_ in SUBCOMMANDS.items():
        subparser = subparsers.add_parser(name, help=class_.help())
        class_.add_args(subparser)

    return parser


def main() -> int:
    """Run a script's main logic, logging errors to file."""
    parser = get_parser()
    if argcomplete is not None:
        argcomplete.autocomplete(parser)
    arguments = parser.parse_args()

    # actual logic
    subcommand_name: str | None = arguments.subcommand
    if subcommand_name is None:
        parser.print_help()
        return 1

    if arguments.qmk is not None:
        lib = arguments.qmk / "lib" / "python"
        sys.path.append(str(lib))
    else:
        sys.stdout.write(
            "[WARN] Could not find QMK's home, code importing `qmk` will fail\n",
        )

    subcommand = SUBCOMMANDS[subcommand_name]

    try:
        res = subcommand().run(arguments)
    except Exception as e:
        t = type(e).__name__
        sys.stdout.write(
            f"Something went wrong trying to run the command: {e} ({t})\n",
        )
        raise
    except KeyboardInterrupt:
        sys.stdout.write("\n")
        return 0

    if is_err(res):
        sys.stdout.write(res.err() + "\n")
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
