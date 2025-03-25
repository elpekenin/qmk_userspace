# notes for future self
# *********************
#  - my chips are B1
#  - *_IN_RAM=1 is pretty much useless, since the functions relocated by it are
#    for the most part copying a few registers + jumping to the code thru vtable

ifeq ($(MCU_SERIES), RP2040)
    # custom script to keep preinit symbol from SDK, needed to init wrappers
    # copied into correct folder via custom build tool
    MCU_LDSCRIPT = elpekenin
endif
