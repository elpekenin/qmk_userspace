TOUCH_SCREEN_ENABLE ?= no
ifeq ($(strip $(TOUCH_SCREEN_ENABLE)), yes)
    QUANTUM_LIB_SRC += spi_master.c
    SRC += $(USER_SRC)/touch/driver.c
endif

SIPO_PINS_ENABLE ?= no
ifeq ($(strip $(SIPO_PINS_ENABLE)), yes)
    SRC += $(USER_SRC)/sipo.c

    # needs a second SPI driver to work, currently not supported on QMK
    SRC += $(USER_SRC)/spi_custom.c

    ifeq ($(strip $(TOUCH_SCREEN_ENABLE)), yes)
        SRC += $(USER_SRC)/touch/sipo.c
    endif

    ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
        SRC += $(USER_SRC)/qp/sipo.c
    endif
endif

KEYLOG_ENABLE ?= no
ifeq ($(strip $(KEYLOG_ENABLE)), yes)
    SRC += $(USER_SRC)/keylog.c
endif
