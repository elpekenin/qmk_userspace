# notes for future self
# *********************
#  - my chips are B1
#  - *_IN_RAM=1 is pretty much useless, since the functions relocated by it are
#    for the most part copying a few registers + jumping to the code thru vtable

ifeq ($(MCU_SERIES), RP2040)
    # custom script to keep preinit symbol from SDK, needed to init wrappers
    # bake a custom LD with some macros for other similar custom sections
    # NOTE: gcc -E does not seem to like .ld's
    # TODO: decouple to reuse on other boards
    $(shell gcc -E $(USER_PATH)/ld/custom_rp2040.h \
                -P \
                -I $(USER_PATH) -I $(USER_PATH)/include \
                -o $(TOP_DIR)/platforms/chibios/boards/common/ld/elpekenin.ld)
    MCU_LDSCRIPT = elpekenin

    # ... however, second core's main will be noop without this set
    USE_SECOND_CORE ?= no
    ifeq ($(strip $(USE_SECOND_CORE)), yes)
        OPT_DEFS += -DUSE_SECOND_CORE
    endif

    # wrap some periodic logic, so that QMK's mainloop (core 0)
    # does nothing and we will execute it on the second one instead
    SECOND_CORE_TASKS ?= $(USE_SECOND_CORE)
    ifeq ($(strip $(SECOND_CORE_TASKS)), yes)
        OPT_DEFS += -DSECOND_CORE_TASKS
        MAIN_TASKS := qp_internal_task deferred_exec_task housekeeping_task
        $(call WRAP, $(MAIN_TASKS))
    endif

    SECOND_CORE_MATRIX ?= $(USE_SECOND_CORE)
    ifeq ($(strip $(SECOND_CORE_MATRIX)), yes)
        OPT_DEFS += -DSECOND_CORE_MATRIX
    endif

    # logic run on 2nd core
    SRC += $(USER_SRC)/mcu/rp2040.c
endif
