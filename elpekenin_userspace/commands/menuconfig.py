"""Subcommand to run configuration menu."""

from __future__ import annotations

import os
from pathlib import Path
from typing import TYPE_CHECKING

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
    from argparse import Namespace

    from elpekenin_userspace.result import Result

ELPEKENIN = Path("users") / "elpekenin"
DEFAULT_AUTOHEADER = ELPEKENIN / "include" / "elpekenin" / "autoconf.h"
DEFAULT_KCONFIG_CONFIG = ELPEKENIN / ".config"

DEFAULTS = {
    "KCONFIG_AUTOHEADER": str(DEFAULT_AUTOHEADER),
    "KCONFIG_CONFIG": str(DEFAULT_KCONFIG_CONFIG),
    "MENUCONFIG_STYLE": "monochrome",
}


class MenuConfig(BaseCommand):
    """Configure build."""

    def run(self, arguments: Namespace) -> Result[None, str]:
        """Entrypoint."""
        _ = arguments

        if not HAS_KCONFIGLIB:
            return Err("Dependencies missing")

        # apply default environment values
        for name, value in DEFAULTS.items():
            os.environ.setdefault(name, value)

        # read Kconfig
        kconfig = kconfiglib.Kconfig()

        # run TUI to configure project
        menuconfig.menuconfig(kconfig)

        # generate header file
        file = os.getenv("KCONFIG_AUTOHEADER")
        if file is None:
            return Err("Unreachable")

        Path(file).parent.mkdir(
            parents=True,
            exist_ok=True,
        )  # make sure target directory exists
        kconfig.write_autoconf()

        return Ok(None)
