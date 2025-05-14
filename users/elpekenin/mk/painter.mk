ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    SRC += \
        $(USER_SRC)/qp/assets.c \
        $(USER_SRC)/qp/tasks.c
endif
