SRC += $(USER_SRC)/logging/backend.c

ifeq ($(strip $(QP_LOG_ENABLE)), yes)
    SRC += $(USER_SRC)/logging/backends/qp.c
endif

ifeq ($(strip $(SPLIT_LOG_ENABLE)), yes)
    SRC += $(USER_SRC)/logging/backends/split.c
endif

ifeq ($(strip $(UART_LOG_ENABLE)), yes)
    SRC += $(USER_SRC)/logging/backends/uart.c
endif

ifeq ($(strip $(XAP_LOG_ENABLE)), yes)
    SRC += $(USER_SRC)/logging/backends/xap.c
endif
