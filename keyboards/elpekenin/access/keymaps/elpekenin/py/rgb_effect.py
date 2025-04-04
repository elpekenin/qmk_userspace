"""Testing MicroPython bindings."""

import qmk.rgb  # type: ignore[import-not-found]

color = qmk.rgb.RED if qmk.get_highest_active_layer() == 0 else qmk.rgb.GREEN

qmk.rgb.set_color(0, color)
