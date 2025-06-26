"""Sphinx extension to automate documenting community modules."""

from __future__ import annotations

from typing import TYPE_CHECKING

import hawkmoth  # type: ignore[import-untyped]
from docutils import nodes

if TYPE_CHECKING:
    from sphinx.application import Sphinx
    from sphinx.util.typing import ExtensionMetadata


class CommunityModuleDoc(hawkmoth.CAutoDocDirective):  # type: ignore[misc]
    """A directive to document a community module."""

    def run(self) -> list[nodes.Node]:
        """Generate documentation for a community module."""
        module_name = self.arguments[0]

        hint = nodes.hint()
        hint += nodes.inline(text="Refer to ")
        hint += nodes.inline(
            "",
            "",
            nodes.reference(
                text="QMK's documentation",
                refuri="https://docs.qmk.fm/features/community_modules",
            ),
        )
        hint += nodes.inline(text=" for details on how to use community modules")

        title = nodes.section(ids=[module_name])
        title += nodes.title(text=module_name)

        self.arguments[0] = f"modules/elpekenin/{module_name}/elpekenin/{module_name}.h"
        autodoc = super().run()

        return [
            hint,
            title,
            *autodoc,
        ]


def setup(app: Sphinx) -> ExtensionMetadata:
    """Add this extension to Sphinx."""
    app.add_directive("community_module.doc", CommunityModuleDoc)

    return {
        "version": "0.1",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
