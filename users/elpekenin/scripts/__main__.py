"""Execute a script."""

from __future__ import annotations

import argparse
import logging
import sys
from pathlib import Path
from typing import NoReturn

import utils

QMK = Path(__file__).parent.parent.parent.parent
LOG_FILE = "python.txt"


def tweak_paths() -> None:
    """Mess with sys.path to get `qmk` visible."""
    sys.path.append(str(QMK / "lib" / "python"))


def main() -> NoReturn:
    """Run a script's main logic, logging errors to file."""
    tweak_paths()

    command_name = sys.argv.pop(1)

    Command: type[utils.CommandBase] = (  # noqa: N806  # this is a class
        __import__(command_name).Command
    )

    parser = argparse.ArgumentParser()

    # common args
    parser.add_argument(
        "output_directory",
        help="directory where to write generated files",
        type=utils.directory_arg,
    )

    parser.add_argument(
        "--log-folder",
        help="directory where to write logs",
        type=utils.directory_arg,
    )

    # script-specific args
    Command.add_args(parser)

    args = parser.parse_args()

    logging.basicConfig(
        filename=args.log_folder / LOG_FILE,
    )

    try:
        ret = Command().run(args)
    except Exception as e:
        output = True

        # if plain exit call, nothing to log
        if (
            isinstance(e, SystemExit)
            and len(e.args) == 1
            and isinstance(e.args[0], int)
        ):
            output = False

        if output:
            logging.exception("Something went wrong in `Command.run()`", exc_info=e)

        raise
    else:
        raise SystemExit(ret)


if __name__ == "__main__":
    main()
