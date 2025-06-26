.. include:: _hint.rst

.. warning::
    This is currently broken and WIP, dont use it because it will crash your board.

micropython
###########

Embeds microypthon's interpreter into your board

Since that's not very useful by itself, a module is provided to hook into QMK APIs. Namely, you use: ``import qmk``

To execute some code, store it as a string and feed it into the ``mp_embed_exec_str`` function

.. tip::
    Writing a C-string directly is not convenient (eg: no IntelliSense for suggestions/typo detection)

    To circumvent that, you can write a regular ``.py`` file, and then use ``euc py2c <file.py>`` to generate
    the equivalent string in a standalone file, which you can later ``#include``

    You could also point your editor at the ``qmk`` folder, and get auto-completion thanks to the ``.pyi`` files in it

.. important::
    In the future, I plan to integrate ``import ...`` with QMK filesystem API (not a thing yet)

    When (if) this happens, you would be able to run dynamic code, opposed to the current state, where you can only execute something defined as a ``const char*`` at compile time

..
    notes to self
        :lines: to skip "generated file" heading comment

Available APIs are:

.. tabs::

    .. tab:: ``qmk``

        .. literalinclude:: ../../modules/elpekenin/micropython/user_c_modules/qmk/__init__.pyi
            :lines: 6-

    .. tab:: ``qmk.keycode``

        .. literalinclude:: ../../modules/elpekenin/micropython/user_c_modules/qmk/_keycode.pyi
            :lines: 6-

    .. tab:: ``qmk.rgb``

        .. literalinclude:: ../../modules/elpekenin/micropython/user_c_modules/qmk/_rgb.pyi
            :lines: 6-
