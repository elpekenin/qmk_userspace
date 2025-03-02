``micropython``
===============

Embeds microypthon's interpreter into your board.

Since that's not very useful by itself, a module is provided to hook into QMK APIs. Namely, you use: ``import qmk``

.. error::
    TODO: Embed stubs here to show the available API, remove them from git

To execute some code, store it as a string and feed it into the ``mp_embed_exec_str`` function.

.. important::
    In the future, I plan to integrate ``import ...`` with QMK filesystem API (not a thing yet).

    When (if) this happens, you would be able to run dynamic code, opposed to the current state, where you can only execute something defined as a ``const char*`` at compile time.
