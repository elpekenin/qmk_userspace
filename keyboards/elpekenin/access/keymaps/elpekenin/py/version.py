"""Testing MicroPython bindings."""

import qmk  # type: ignore[import-not-found]

version = ".".join(map(str, qmk.version_info))
qmk.send_string(f"running QMK: {version}")
