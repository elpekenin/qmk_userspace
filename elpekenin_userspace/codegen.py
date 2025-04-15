"""Small snippets reused across scripts."""

from __future__ import annotations

HEADER_TEMPLATE = """\
{comment} THIS FILE WAS GENERATED

{comment} Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
{comment} SPDX-License-Identifier: GPL-2.0-or-later
"""

C_HEADER = HEADER_TEMPLATE.format(comment="//")
H_HEADER = f"{C_HEADER}\n\n#pragma once"

MK_HEADER = HEADER_TEMPLATE.format(comment="#")
PY_HEADER = MK_HEADER
