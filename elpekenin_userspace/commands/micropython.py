"""Subcommand to generate a mapping for all char keycodes. (`qmk.keycode`)."""

from __future__ import annotations

from typing import TYPE_CHECKING

import elpekenin_userspace.keycodes
from elpekenin_userspace.codegen import C_HEADER
from elpekenin_userspace.commands import CodegenCommand
from elpekenin_userspace.result import Err, Ok, is_err

if TYPE_CHECKING:
    from argparse import Namespace

    from elpekenin_userspace.keycodes import Spec
    from elpekenin_userspace.result import Result

TEMPLATE = """\
//| {name}: int
{{ MP_ROM_QSTR(MP_QSTR_{name}), MP_ROM_INT({name}) }}
"""


def parse_kc(val: str) -> int:
    """Parse hex str into a value."""
    return int(val, base=16)


def get_basic_range(spec: Spec) -> Result[tuple[int, int], str]:
    """Get the first and last keycodes in basic range."""
    for key, val in spec["ranges"].items():
        if val["define"] == "QK_BASIC":
            first, last = key.split("/")
            return Ok((parse_kc(first), parse_kc(last)))

    return Err("Could not find QK_BASIC range in keycode spec")


class Micropython(CodegenCommand):
    """Generate C file with MicroPython bindings for QMK keycodes."""

    def run(self, arguments: Namespace) -> Result[None, str]:
        """Entrypoint."""
        spec = elpekenin_userspace.keycodes.load_spec(
            "latest",
            home=arguments.qmk,
        )

        res = get_basic_range(spec)
        if is_err(res):
            return res

        first, last = res.ok()

        output_file = arguments.output_directory / "keycodes.c"
        with output_file.open("w") as f:
            f.write(C_HEADER + "\n")

            for value, info in spec["keycodes"].items():
                if parse_kc(value) < first or parse_kc(value) > last:
                    continue

                f.write(TEMPLATE.format(name=info["key"]))

        return Ok(None)
