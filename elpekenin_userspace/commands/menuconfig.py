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

CONF_FILE = Path("users") / "elpekenin" / "kconf.h"


class MenuConfig(BaseCommand):
    """Configure build."""

    def run(self, arguments: Namespace) -> Result[None, str]:
        """Entrypoint."""
        _ = arguments

        if not HAS_KCONFIGLIB:
            return Err("Dependencies missing")

        os.environ.setdefault("MENUCONFIG_STYLE", "monochrome")

        # read Kconfig
        kconfig = kconfiglib.Kconfig()

        # open TUI to configure project
        menuconfig.menuconfig(kconfig)

        # generate header file
        kconfig.write_autoconf(CONF_FILE)

        return Ok(None)
