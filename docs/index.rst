#############
elpekenin QMK
#############

.. caution::
  This is not an usual QMK external userspace. I use a custom script to compile (``elpekenin_userspace build``)

  Motivation for this setup is to easily keep code out of tree, but in sync with QMK changes.

************
Introduction
************

This documentation is built using

* `Sphinx <https://www.sphinx-doc.org/en/master/>`_: Documentation framework
* `Furo <https://pradyunsg.me/furo/quickstart/>`_: Theme for Sphinx styling
* `Hawkmoth <https://hawkmoth.readthedocs.io/en/stable/installation.html>`_: Plugin to work with C code in Sphinx
* Of course, some writing in ``.rst`` files

Keep in mind that docs might be out of date with the actual code. However I've put quite some effort in generating it automatically from
doc-comments in the code, in hopes to reduce this friction. If you find anything out-of-sync please let me know

**************
File structure
**************

* docs: files used to generate this page
* keyboards/elpekenin/access: contains source for my custom PCB, and my keymap inside it
* modules/elpekenin: submodule pointing to my community modules
* users/elpekenin: most of the code, structured as

  * src: source code
  * include: header files
  * mk: Makefile logic, broken into "small" sections
  * scripts: Python that generates code at compile time
  * painter: `Quantum Painter <https://docs.qmk.fm/quantum_painter>`_ assets
  * autocorrect: database of corrections
  * 3rd_party: a couple of external dependencies that are used

* Then, ``patches/`` and ``build.json`` will put everything together into a clone of ``qmk_firmware`` to build


.. toctree::
  :hidden:

  modules
  userspace
