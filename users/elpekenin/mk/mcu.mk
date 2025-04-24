# notes for future self
# *********************
#  - my chips are B1
#  - *_IN_RAM=1 is pretty much useless, since the functions relocated by it are
#    for the most part copying a few registers + jumping to the code thru vtable

ifeq ($(MCU_SERIES), RP2040)
    # custom script to keep preinit symbol from SDK, needed to init wrappers
    MCU_LDSCRIPT = elpekenin_rp2040
    # TODO: integrate with QMK (eg: make it look at $(USER_PATH)/ld) instead of copying file
    $(shell cp $(USER_PATH)/ld/$(MCU_LDSCRIPT).ld $(TOP_DIR)/platforms/chibios/boards/common/ld)
endif
