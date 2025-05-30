"""Subcommand to run configuration menu."""

from __future__ import annotations

import os
from pathlib import Path
from typing import TYPE_CHECKING

from elpekenin_userspace import args
from elpekenin_userspace.codegen import H_HEADER, MK_HEADER
from elpekenin_userspace.commands import BaseCommand
from elpekenin_userspace.result import Err, Ok

try:
    import kconfiglib  # type: ignore[import-untyped]
    import menuconfig  # type: ignore[import-untyped]
except ImportError:
    HAS_KCONFIGLIB = False
else:
    HAS_KCONFIGLIB = True

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace

    from elpekenin_userspace.result import Result

ELPEKENIN = Path("users") / "elpekenin"
DEFAULT_CONFIG = ELPEKENIN / "mk" / "kconfig.mk"
DEFAULT_AUTOHEADER = ELPEKENIN / "include" / "elpekenin" / "autoconf.h"


class Config(BaseCommand):
    """Configure build."""

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """Command-specific arguments."""
        parser.add_argument(
            "--kconfig",
            help="Kconfig entrypoint",
            metavar="FILE",
            type=args.File(require_existence=True),
            default="Kconfig",
        )

        parser.add_argument(
            "--config",
            help="generated config",
            metavar="FILE",
            type=args.File(),
            default=str(DEFAULT_CONFIG),
        )

        parser.add_argument(
            "--autoheader",
            help="generated header",
            metavar="FILE",
            type=args.File(suffix=".h"),
            default=str(DEFAULT_AUTOHEADER),
        )

        parser.add_argument(
            "--style",
            help="menuconfig style",
            default="monochrome",
        )

        return super().add_args(parser)

    def run(self, arguments: Namespace) -> Result[None, str]:
        """Entrypoint."""
        _ = arguments

        if not HAS_KCONFIGLIB:
            return Err("Dependencies missing")

        mk_file: Path = arguments.config
        h_file: Path = arguments.autoheader

        # setup environment
        environment = {
            "CONFIG_": "",  # no prefix, for QMK settings
            "KCONFIG_AUTOHEADER": str(h_file),
            "KCONFIG_AUTOHEADER_HEADER": H_HEADER,
            "KCONFIG_CONFIG": str(mk_file),
            "KCONFIG_CONFIG_HEADER": MK_HEADER,
            "MENUCONFIG_STYLE": arguments.style,
        }
        for key, val in environment.items():
            env = os.environ.get(key)
            if env is not None and env != val:
                return Err("Environment variable and CLI argument don't match")

            os.environ[key] = val

        # read Kconfig
        kconfig = kconfiglib.Kconfig(str(arguments.kconfig))

        # run TUI to configure project
        menuconfig.menuconfig(kconfig)

        # HACK: "y" -> "yes" in config file, to use it as a makefile  # noqa: FIX004
        #       reading config with "=yes" in it doesn't break (yet?)
        mk_file.write_text(
            mk_file.read_text().replace("=y\n", "=yes\n"),
        )

        # generate header file
        kconfig.write_autoconf()

        # add an X-macro in case we wanna eg print all settings
        settings: list[str] = []
        with h_file.open("r") as f:
            for line in f.readlines():
                if not line.startswith("#define"):
                    continue

                # each line is `#define NAME VALUE`
                settings.append(line.split(" ")[1])

        with h_file.open("a") as f:
            f.writelines(
                [
                    "\n#define FOREACH_AUTOCONF(X)",
                    *(f" \\\n    X({setting})" for setting in settings),
                    "\n",
                ],
            )

        return Ok(None)
