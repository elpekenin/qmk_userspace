.. include:: _hint.rst

ls0xx
#####
Support for LS0XX displays (eg: nice!view), extracted from my (quite abandoned) PR.

.. attention::
    Might contain some bug that I haven't found yet.
    Please report if you find something.

    Since this driver is not part of QMK's codebase, some integrations won't work,
    namely, the auto-flushing and the power on/off of (in)activity are not applied
    to this device.

.. c:autodoc:: modules/elpekenin/ls0xx/ls0xx.h
