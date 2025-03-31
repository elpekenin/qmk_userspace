"""Subcommand to generate a type for checking if features are enabled."""

from __future__ import annotations

from typing import TYPE_CHECKING

from elpekenin_userspace.codegen import C_HEADER, H_HEADER, lines
from elpekenin_userspace.commands import CodegenCommand

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from pathlib import Path

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


def get_type(features: set[str]) -> str:
    """Compute <stdint.h> type to use based on the amount of features."""
    bits = len(features)
    for size in (8, 16, 32, 64):
        if bits <= size:
            return f"uint{size}_t"

    msg = "Too many features specified, unsupported"
    raise RuntimeError(msg)


def gen_h_file(file: Path, features: set[str]) -> None:
    """Create contents for H file."""
    width = max(map(len, features))

    code = ""
    for feature in features:
        code += f"        bool {feature.lower().ljust(width)}: 1;\n"

    file.write_text(
        lines(
            H_HEADER,
            "",
            "#include <stdbool.h>",
            "#include <stdint.h>",
            "",
            "typedef union {{",
            "    {type} raw;",
            "    struct {{",
            "{code}"  # intentional lack of comma
            "    }};",
            "}} enabled_features_t;",
            "",
            "enabled_features_t get_enabled_features(void);",
            "",
        ).format(type=get_type(features), code=code),
    )


def gen_c_file(file: Path, features: set[str]) -> None:
    """Create contents for C file."""
    code = ""
    for feature in features:
        code += lines(
            f"    #if defined({feature.upper()}_ENABLE)",
            f"        features.{feature.lower()} = true;",
            "    #endif",
            "",
            "",
        )

    file.write_text(
        lines(
            C_HEADER,
            "",
            f'#include "{OUTPUT_NAME}.h"',
            "",
            "enabled_features_t get_enabled_features(void) {{",
            "    enabled_features_t features;",
            "",
            "    features.raw = 0;",
            "",
            "{code}"  # intentional lack of comma
            "    return features;",
            "}}",
            "",
        ).format(code=code),
    )


def gen_draw_file(file: Path, features: set[str]) -> None:
    """Create contents for C file."""
    code = ""
    for feature in features:
        # get alias or keep as is
        short_name = ALIASES.get(feature, feature)
        name = short_name.replace("_", " ").title()

        code += lines(
            f'    qp_drawtext_recolor(device, x, y, font, features.{feature.lower()} ? "{name}: On " : "{name}: Off", {DEFAULT_FG}, {DEFAULT_BG});',  # noqa: E501
            #                         intentional space so it overwrites previous "Off"         ^^^  # noqa: E501
            "    y += font_height;",
            "    // next text does not fit vertically",
            "    if ((y + font_height) > height) {",
            "        // shift half the screen to the right, if not done already",
            "        if (!shifted) {",
            "            shifted = true;",
            "            x = width / 2;",
            "            y = 0;",
            "        } else {",
            '           logging(LOG_WARN, "Can\'t fit more features");',
            "           return;",
            "        }",
            "    }",
            "",
            "",
        )

    file.write_text(
        lines(
            C_HEADER,
            "",
            f'#include "{OUTPUT_NAME}.h"',
            "",
            "#include <quantum/quantum.h>",
            "#include <quantum/color.h>",
            "",
            '#include "elpekenin/qp/assets.h"',
            '#include "elpekenin/build_info.h"',
            '#include "elpekenin/logging.h"',
            "",
            "void draw_features(painter_device_t device) {{",
            '    painter_font_handle_t font = qp_get_font_by_name("font_fira_code");',
            "    if (font == NULL) {{",
            '        logging(LOG_ERROR, "Font was NULL");',
            "        return;",
            "    }}",
            "",
            "    enabled_features_t features    = get_build_info().features;",
            "    uint8_t            font_height = font->line_height;",
            "    uint8_t            x           = 0;",
            "    uint8_t            y           = 0;",
            "",
            "    uint16_t width  = qp_get_width(device);",
            "    uint16_t height = qp_get_height(device);",
            "",
            "    bool shifted = false;",
            "",
            "{code}"  # no comma here intentionally
            "}}",
        ).format(code=code),
    )


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

    def run(self, arguments: Namespace) -> int:
        """Entrypoint."""
        output_directory: Path = arguments.output_directory

        features_list: list[str] = arguments.features
        features = {feature.upper() for feature in features_list}

        gen_h_file(output_directory / f"{OUTPUT_NAME}.h", features)
        gen_c_file(output_directory / f"{OUTPUT_NAME}.c", features)
        gen_draw_file(output_directory / f"{OUTPUT_NAME}_draw.c", features)

        return 0
