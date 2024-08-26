ifeq ($(strip $(RGB_MATRIX_ENABLE)), yes)
    SRC += $(USER_SRC)/rgb/matrix/callbacks.c \
           $(USER_SRC)/rgb/matrix/indicators.c
endif
