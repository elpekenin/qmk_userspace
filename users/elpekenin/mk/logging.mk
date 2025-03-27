SRC += $(USER_SRC)/logging/backend.c

ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    SRC += $(USER_SRC)/logging/backends/qp.c
endif

ifeq ($(strip $(SPLIT_KEYBOARD)), yes)
    SRC += $(USER_SRC)/logging/backends/split.c
endif

ifeq ($(strip $(XAP_ENABLE)), yes)
    SRC += $(USER_SRC)/logging/backends/xap.c
endif
