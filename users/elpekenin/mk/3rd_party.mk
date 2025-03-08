3RD_PARTY = $(USER_PATH)/3rd_party

MATH_M0_ENABLE ?= no
MATH_M0_IN_RAM ?= 1
ifeq ($(strip $(MATH_M0_ENABLE)), yes)
    MATH_BASE = $(3RD_PARTY)/ArmMathM0/src

    VPATH += $(MATH_BASE)/include
    OPT_DEFS += -DMATH_M0_IN_RAM=$(MATH_M0_IN_RAM)
    SRC += $(wildcard $(MATH_BASE)/**/*.S)
endif
