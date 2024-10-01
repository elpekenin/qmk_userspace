ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    SRC += $(USER_SRC)/logging/backends/qp.c
endif

ifeq ($(strip $(SPLIT_KEYBOARD)), yes)
    SRC += $(USER_SRC)/logging/backends/split.c
endif

ifeq ($(strip $(XAP_ENABLE)), yes)
    SRC += $(USER_SRC)/logging/backends/xap.c
endif

# get QMK's integration with RTT enabled
SEGGER_RTT_DRIVER_REQUIRED ?= no

SRC += $(USER_SRC)/logging/backend.c \
       $(USER_SRC)/logging/frontend.c
