"""Execute a script."""

from __future__ import annotations

import argparse
import logging
import sys
from pathlib import Path
from typing import NoReturn

import common

HERE = Path(__file__).parent
QMK = HERE.parent.parent.parent
LOG_FILE = "python.txt"


def get_class(script_name: str) -> type[common.ScriptBase]:
    """Check if input is actually a subclass of ScriptBase."""
    script_file = HERE / f"{script_name}.py"
    if not script_file.exists():
        msg = f"'{script_name}' is not a known script."
        raise SystemExit(msg)

    script_module = __import__(script_name)

    cls = getattr(script_module, "Script", None)
    if cls is None:
        msg = f"'{script_file}' does not contain a 'Script' class."
        raise SystemExit(msg)
    if not issubclass(cls, common.ScriptBase):
        msg = f"{cls} must be a subclass of 'utils.ScriptBase'."
        raise SystemExit(msg)

    return cls


def tweak_paths() -> None:
    """Mess with sys.path to get `qmk` visible."""
    sys.path.append(str(QMK / "lib" / "python"))


def main() -> NoReturn:
    """Run a script's main logic, logging errors to file."""
    tweak_paths()

    # NOTE: we have to hack here, doing some manual parsing
    # at this point, we can't yet defer into argparse, because
    # there are script-specific args we dont know ahead of time
    script_name = sys.argv.pop(1)

    ScriptClass = get_class(script_name)  # noqa: N806  # this is a class

    parser = argparse.ArgumentParser()

    # common args
    parser.add_argument(
        "output_directory",
        help="directory where to write generated files",
        type=common.directory_arg,
    )
    parser.add_argument(
        "--log-folder",
        help="directory where to write logs",
        type=common.directory_arg,
    )

    # script-specific args
    ScriptClass.add_args(parser)

    args = parser.parse_args()

    logging.basicConfig(
        filename=args.log_folder / LOG_FILE,
    )

    try:
        ret = ScriptClass().run(args)
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
            logging.exception("Something went wrong in `Script.run()`", exc_info=e)

        raise
    else:
        raise SystemExit(ret)


if __name__ == "__main__":
    main()
