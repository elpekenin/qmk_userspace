UI := $(USER_SRC)/qp/ui

ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    SRC += $(USER_SRC)/qp/assets.c

    SRC += \
        $(UI)/build_match.c \
        $(UI)/computer.c \
        $(UI)/github.c
endif
