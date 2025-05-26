# PYTHON_ARGCOMPLETE_OK

"""Utilities to interact with my userspace."""

from __future__ import annotations

import argparse
import sys
from typing import TYPE_CHECKING

try:
    import argcomplete
except ImportError:
    HAS_ARGCOMPLETE = False
else:
    HAS_ARGCOMPLETE = True

from elpekenin_userspace import args
from elpekenin_userspace.commands.build import Build
from elpekenin_userspace.commands.config import Config
from elpekenin_userspace.commands.docs import Docs
from elpekenin_userspace.commands.features import Features
from elpekenin_userspace.commands.jsonschema import Jsonschema
from elpekenin_userspace.commands.keycode_str import KeycodeStr
from elpekenin_userspace.commands.micropython import Micropython
from elpekenin_userspace.commands.py2c import Py2C
from elpekenin_userspace.commands.qp_resources import QpResources
from elpekenin_userspace.commands.stubs import Stubs
from elpekenin_userspace.commands.xap import Xap
from elpekenin_userspace.result import is_err

if TYPE_CHECKING:
    from elpekenin_userspace.commands import BaseCommand


SUBCOMMANDS: dict[str, type[BaseCommand]] = {
    "build": Build,
    "config": Config,
    "docs": Docs,
    "features": Features,
    "jsonschema": Jsonschema,
    "keycode_str": KeycodeStr,
    "micropython": Micropython,
    "qp_resources": QpResources,
    "py2c": Py2C,
    "stubs": Stubs,
    "xap": Xap,
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
        default="",
    )

    # script-specific args
    subparsers = parser.add_subparsers(dest="subcommand")
    for name, class_ in SUBCOMMANDS.items():
        subparser = subparsers.add_parser(
            name,
            help=class_.help(),
            formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        )
        class_.add_args(subparser)

    return parser


def main() -> int:
    """Run a script's main logic, logging errors to file."""
    parser = get_parser()
    if HAS_ARGCOMPLETE:
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
    except Exception as e:  # noqa: BLE001  # want to handle all errors
        exc_type = type(e).__name__
        sys.stdout.write(f"{exc_type}: {e}\n")

        call_stack: list[str] = []

        traceback = e.__traceback__
        while True:
            if traceback is None:
                break

            call_stack.append(
                f"    {traceback.tb_frame.f_code.co_filename}:{traceback.tb_lineno}",
            )
            traceback = traceback.tb_next

        for line in reversed(call_stack):
            sys.stdout.write(line + "\n")

        return 1
    except KeyboardInterrupt:
        sys.stdout.write("\n")
        return 0

    if is_err(res):
        sys.stdout.write(res.err() + "\n")
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
