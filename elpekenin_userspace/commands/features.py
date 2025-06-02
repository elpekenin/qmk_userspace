"""Subcommand to generate a type for checking if features are enabled."""

from __future__ import annotations

from typing import TYPE_CHECKING

from elpekenin_userspace.codegen import C_HEADER, H_HEADER
from elpekenin_userspace.commands import CodegenCommand
from elpekenin_userspace.result import Err, Ok, is_err

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from pathlib import Path

    from elpekenin_userspace.result import Result

OUTPUT_NAME = "features"

DEFAULT_FG = "HSV_BLACK"
DEFAULT_BG = "HSV_WHITE"
DEFAULT_FEATURES = {
    "AUDIO",
    "AUTOCORRECT",
    "BOOTMAGIC",
    "COMBO",
    "EXTRAKEY",
    "KEY_OVERRIDE",
    "MOUSEKEY",
    "NKRO",
    "RGB_MATRIX",
    "SIPO_PINS",
    "TAP_DANCE",
    "TOUCH_SCREEN",
    "QP_XAP",
    "QUANTUM_PAINTER",
    "WPM",
    "XAP",
}

ALIASES = {
    "QUANTUM_PAINTER": "PAINTER",
    "UNICODE_COMMON": "UNICODE",
}
"""Shorter names."""


def get_type(features: set[str]) -> Result[str, str]:
    """Compute <stdint.h> type to use based on the amount of features."""
    bits = len(features)
    for size in (8, 16, 32, 64):
        if bits <= size:
            return Ok(f"uint{size}_t")

    return Err("Too many features specified, unsupported")


def gen_h_file(file: Path, features: set[str], type_: str) -> None:
    """Create contents for H file."""
    width = max(map(len, features))

    with file.open("w") as f:
        f.writelines(
            [
                H_HEADER,
                "\n",
                "#include <stdbool.h>\n",
                "#include <stdint.h>\n",
                "\n",
                "typedef union {\n",
                f"    {type_} raw;\n",
                "    struct {\n",
            ],
        )

        for feature in features:
            f.write(f"        bool {feature.lower().ljust(width)}: 1;\n")

        f.writelines(
            [
                "    };\n",
                "} enabled_features_t;\n",
                "\n",
                "enabled_features_t get_enabled_features(void);\n",
            ],
        )


def gen_c_file(file: Path, features: set[str]) -> None:
    """Create contents for C file."""
    with file.open("w") as f:
        f.writelines(
            [
                C_HEADER + "\n",
                "\n",
                f'#include "{OUTPUT_NAME}.h"\n',
                "\n",
                "enabled_features_t get_enabled_features(void) {\n",
                "    return (enabled_features_t){\n",
            ],
        )

        for feature in features:
            f.write(
                f"        .{feature.lower()} = IS_ENABLED({feature}),\n",
            )

        f.writelines(
            [
                "    };\n",
                "}\n",
            ],
        )


def gen_draw_file(file: Path, features: set[str]) -> None:
    """Create contents for C file."""
    with file.open("w") as f:
        f.writelines(
            [
                C_HEADER + "\n",
                "",
                f'#include "{OUTPUT_NAME}.h"',
                "\n",
                "#include <quantum/quantum.h>\n",
                "#include <quantum/color.h>\n",
                "\n",
                '#include "elpekenin/qp/assets.h"\n',
                '#include "elpekenin/build_info.h"\n',
                '#include "elpekenin/logging.h"\n',
                "\n",
                "typedef struct {\n",
                "    uint16_t x;\n",
                "    uint16_t y;\n",
                "    bool    shifted;\n",
                "} drawing_state_t;\n",
                "\n",
                "bool update_state(drawing_state_t *state, painter_device_t device, painter_font_handle_t font) {\n",  # noqa: E501
                "    state->y += font->line_height;\n",
                "    // next line fits\n",
                "    if ((state->y + font->line_height) < qp_get_height(device)) {\n",
                "        return true;\n",
                "    }\n",
                "\n",
                "    // next doesn't fit + already shifted\n",
                "    if (state->shifted) {\n",
                "        return false;\n",
                "    }\n",
                "\n",
                "    // shift\n",
                "    *state = (drawing_state_t){\n",
                "        .x       = qp_get_width(device) / 2,\n",
                "        .y       = 0,\n",
                "        .shifted = true,\n",
                "    };\n",
                "\n",
                "    return true;\n",
                "}\n",
                "\n",
                "void draw_features(painter_device_t device) {\n",
                '    painter_font_handle_t font = qp_get_font_by_name("fira_code");\n',
                "    if (font == NULL) {\n",
                '        logging(LOG_ERROR, "Font was NULL");\n',
                "        return;\n",
                "    }\n",
                "\n",
                "    enabled_features_t features = get_build_info().features;\n",
                "    drawing_state_t    state    = {0};\n",
                "\n",
            ],
        )

        for feature in features:
            # get alias or keep as is
            short_name = ALIASES.get(feature, feature)
            name = short_name.replace("_", " ").title()

            f.writelines(
                [
                    f'    qp_drawtext_recolor(device, state.x, state.y, font, features.{feature.lower()} ? "{name}: On " : "{name}: Off", {DEFAULT_FG}, {DEFAULT_BG});\n',  # noqa: E501
                    #                         intentional space so it overwrites previous "Off"         ^^^  # noqa: E501
                    "    if (!update_state(&state, device, font)) {\n",
                    '        logging(LOG_WARN, "Can\'t fit more features");\n',
                    "        return;\n",
                    "    }\n",
                    "\n",
                ],
            )

        f.write("}\n")


class Features(CodegenCommand):
    """Define a type that holds whether some features are enabled."""

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """Command-specific arguments."""
        parser.add_argument(
            "--foreground",
            help="color to use for text",
            required=False,
            default=DEFAULT_FG,
        )

        parser.add_argument(
            "--background",
            help="color to use for text's background",
            required=False,
            default=DEFAULT_BG,
        )

        parser.add_argument(
            "features",
            help="features to be inspected",
            nargs="*",
            default=DEFAULT_FEATURES,
        )

        return super().add_args(parser)

    def run(self, arguments: Namespace) -> Result[None, str]:
        """Entrypoint."""
        output_directory: Path = arguments.output_directory

        features_list: list[str] = arguments.features
        features = {feature.upper() for feature in features_list}

        res = get_type(features)
        if is_err(res):
            return res

        gen_h_file(output_directory / f"{OUTPUT_NAME}.h", features, res.ok())
        gen_c_file(output_directory / f"{OUTPUT_NAME}.c", features)
        gen_draw_file(output_directory / f"{OUTPUT_NAME}_draw.c", features)

        return Ok(None)
