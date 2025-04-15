"""Utilities to interact with keycodes."""

from __future__ import annotations

from typing import TYPE_CHECKING, cast

import elpekenin_userspace.path

if TYPE_CHECKING:
    from pathlib import Path
    from typing import TypedDict

    # NOTE(elpekenin): these types are incomplete, only cover used attrs

    class Keycode(TypedDict):
        """QMK's keycode specification."""

        key: str

    class Range(TypedDict):
        """QMK's keycode specification."""

        define: str

    class Spec(TypedDict):
        """QMK's keycode specification."""

        keycodes: dict[str, Keycode]
        ranges: dict[str, Range]


def load_spec(version: str, *, home: Path) -> Spec:
    """Perform some tricks to load keycode specs.

    This is needed because QMK's code is intended to run from its home.
    """
    # lazy import for sys.path patching
    import qmk.keycodes  # type: ignore[import-not-found]

    with elpekenin_userspace.path.chdir(home):
        return cast("Spec", qmk.keycodes.load_spec(version))
