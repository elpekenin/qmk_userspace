ifeq ($(strip $(SPLIT_KEYBOARD)), yes)
    SRC += $(USER_SRC)/transactions.c
endif
