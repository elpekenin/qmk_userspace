GAME_ENABLE ?= no

SELECTED_GAME ?=
VALID_GAME_OPTIONS := snake

ifeq ($(strip $(GAME_ENABLE)), yes)
    ifeq ($(filter $(SELECTED_GAME),$(VALID_GAME_OPTIONS)),)
        $(call CATASTROPHIC_ERROR,Invalid SELECTED_GAME,'$(SELECTED_GAME)' is not a valid game)
    endif

    ifneq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
        $(call CATASTROPHIC_ERROR,Invalid settings,Games require Quantum Painter)
    endif

    OPT_DEFS += -DGAME_ENABLE -DGAME_$(shell echo $(SELECTED_GAME) | tr '[:lower:]' '[:upper:]')
    SRC += $(USER_SRC)/games/game.c \
           $(USER_SRC)/games/$(SELECTED_GAME).c
endif
