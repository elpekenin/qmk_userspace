"""Generate the documentation hosted on <qmk.elpekenin.dev>_."""

# noqa: INP001

# NOTE: Environment variables named CONF_* are provided by `py manage.py docs`

from __future__ import annotations

import builtins
import os
from pathlib import Path

# -- Project information
project = "QMK userspace"
copyright = "2024, elpekenin"  # noqa: A001
author = "elpekenin"
release = ""

# -- General configuration
extensions = [
    "hawkmoth",
    "hawkmoth.ext.napoleon",
    "sphinx_tabs.tabs",
]

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# -- Options for HTML output
html_theme = "furo"
html_show_sourcelink = False  # disable "view source" button, showing raw rst
html_static_path: list[str] = []
html_theme_options = {
    "navigation_with_keys": True,
    "footer_icons": [
        {  # Github logo
            "name": "GitHub",
            "url": "https://github.com/elpekenin/qmk_userspace/",
            "html": """
                <svg
                    stroke="currentColor"
                    fill="currentColor"
                    stroke-width="0"
                    viewBox="0 0 16 16"
                >
                    <path
                        fill-rule="evenodd"
                        d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0 0 16 8c0-4.42-3.58-8-8-8z"
                    ></path>
                </svg>
            """,  # noqa: E501
            "class": "",
        },
    ],
}

autodoc_member_order = "bysource"

# -- Hawkmoth config
hawkmoth_root = os.environ["CONF_QMK"]
hawkmoth_clang = os.environ["CONF_HAWKMOTH_CLANG"].split("||")

ROOT = Path(__file__).parent.parent
USERSPACE = ROOT / "users" / "elpekenin"
MODULES = ROOT / "modules" / "elpekenin"


class UriGenerator:
    """Convenience class to generate permalinks to source code.

    It also handles providing links to 2 different repositories.

    Provide a `.format` function to match how hawkmoth uses it.
    It is supposed to be a template string, but we are cooler :)

    FIXME(elpekenin): Prone to breaking if hawkmoth's implementation changes.
    """

    def __init__(self) -> None:
        self.userspace_template = (
            "https://github.com/elpekenin/qmk_userspace/blob/{version}/{source}#L{line}"
        )
        self.modules_template = (
            "https://github.com/elpekenin/qmk_modules/blob/{version}/{source}#L{line}"
        )

    def format(self, **kwargs: str) -> str:
        """Create a link."""
        source = Path(kwargs["source"]).resolve()

        if "version" in kwargs:
            print("WARNING: UriGenerator overwriting `kwargs['version']`")

        if source.is_relative_to(USERSPACE):
            kwargs.update(
                version=os.environ["CONF_USERSPACE_COMMIT"],
            )
            return self.userspace_template.format(**kwargs)

        if source.is_relative_to(MODULES):
            # need to remap source, it starts with "module/elpekenin"
            # but the path in its dedicated repository drops that part
            relative = source.relative_to(MODULES)
            kwargs.update(
                source=str(relative),
                version=os.environ["CONF_MODULES_COMMIT"],
            )
            return self.modules_template.format(**kwargs)

        msg = f"Unknown source: '{source}'"
        raise RuntimeError(msg)


hawkmoth_source_uri = UriGenerator()

# NOTE: sphinx (un)pickles this class. the operation failed due to `pickle`
# not finding the class' name in the `builtins` namespace (i have no idea how or
# why this happens)
# tried to come up with a proper solution... this is the only thing i managed to run ^_^
builtins.__dict__["UriGenerator"] = UriGenerator

hawkmoth_napoleon_transform = None  # apply napoleon transform to every docstring
