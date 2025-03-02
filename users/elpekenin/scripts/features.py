#! /usr/bin/env python3

# Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
# SPDX-License-Identifier: GPL-2.0-or-later

"""Define a new struct type which holds whether selected features are enabled or not.

Also provides a function to draw the state on a QP screen.
"""

from __future__ import annotations

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from argparse import Namespace
    from collections.abc import Callable
    from pathlib import Path

import common

# == User configuration here ==
TEXT_COLOR = "HSV_BLACK"
BACKGROUND_COLOR = "HSV_WHITE"

# Capitalization here does not matter, code will format it
FEATURES = sorted(
    {
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
        "UNICODE_COMMON",
        "WPM",
        "XAP",
    },
)
SHORT_NAMES = {
    "QUANTUM_PAINTER": "PAINTER",
    "UNICODE_COMMON": "UNICODE",
}
# == Do not edit past here ==

OUTPUT_NAME = "features"
MAX_WIDTH = max(map(len, FEATURES))

# *** Templates ***
H_FILE = common.lines(
    common.H_HEADER,
    "",
    "#include <stdbool.h>",
    "#include <stdint.h>",
    "",
    "typedef union {{",
    "    {type} raw;",
    "    struct {{",
    "{generated_code}",
    "    }};",
    "}} enabled_features_t;",
    "",
    "enabled_features_t get_enabled_features(void);",
    "",
)

C_FILE = common.lines(
    common.C_HEADER,
    "",
    f'#include "{OUTPUT_NAME}.h"',
    "",
    "enabled_features_t get_enabled_features(void) {{",
    "    enabled_features_t features;",
    "",
    "    features.raw = 0;",
    "",
    "{generated_code}",
    "    return features;",
    "}}",
    "",
)

DRAW_FILE = common.lines(
    common.C_HEADER,
    "",
    "#include <quantum/color.h>",
    "",
    f'#include "{OUTPUT_NAME}.h"',
    '#include "elpekenin/qp/graphics.h"',
    '#include "elpekenin/build_info.h"',
    '#include "elpekenin/logging.h"',
    "",
    "void draw_features(painter_device_t device) {{",
    '    painter_font_handle_t font = qp_get_font_by_name("font_fira_code");',
    "    if (font == NULL) {{",
    '        logging(QP, LOG_ERROR, "Font was NULL");',
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
    "{generated_code}"  # no comma here intentionally
    "}}",
)


def _get_type() -> str:
    bits = len(FEATURES)
    for size in (8, 16, 32, 64):
        if bits <= size:
            return f"uint{size}_t"

    msg = "Too many features, unsupported"
    raise ValueError(msg)


def _for_all_features(func: Callable[[str], str]) -> str:
    return "\n".join(func(feature) for feature in FEATURES)


def _h_generator(feature: str) -> str:
    return f"        bool {feature.lower().ljust(MAX_WIDTH)}: 1;"


def _c_generator(feature: str) -> str:
    return common.lines(
        f"    #if defined({feature.upper()}_ENABLE)",
        f"        features.{feature.lower()} = true;",
        "    #endif",
        "",
    )


def _draw_generator(feature: str) -> str:
    # get alias or keep as is
    short_name = SHORT_NAMES.get(feature, feature)
    name = short_name.replace("_", " ").title()

    return common.lines(
        f'    qp_drawtext_recolor(device, x, y, font, features.{feature.lower()} ? "{name}: On " : "{name}: Off", {TEXT_COLOR}, {BACKGROUND_COLOR});',  # noqa: E501
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
        '           logging(QP, LOG_WARN, "Can\'t fit more features");',
        "           return;",
        "        }",
        "    }",
        "",
    )


class Script(common.ScriptBase):
    """Logic of this script."""

    def run(self, args: Namespace) -> int:
        """Entrypoint."""
        output_directory: Path = args.output_directory

        # Gen files
        type_ = _get_type()  # Work out the union type needed
        gen_h = _for_all_features(_h_generator)
        with (output_directory / f"{OUTPUT_NAME}.h").open("w") as f:
            f.write(H_FILE.format(type=type_, generated_code=gen_h))

        gen_c = _for_all_features(_c_generator)
        with (output_directory / f"{OUTPUT_NAME}.c").open("w") as f:
            f.write(C_FILE.format(generated_code=gen_c))

        gen_draw = _for_all_features(_draw_generator)
        with (output_directory / f"{OUTPUT_NAME}_draw.c").open("w") as f:
            f.write(DRAW_FILE.format(generated_code=gen_draw))

        return 0
