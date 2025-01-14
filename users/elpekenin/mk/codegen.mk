USER_SCRIPTS = $(USER_PATH)/scripts
USER_GENERATED = $(USER_PATH)/generated

PYTHON = python3
LOG_ARG = --log-folder $(LOG_FOLDER)

# make sure folder exists
$(shell mkdir -p $(USER_GENERATED))

# enabled_features_t
$(USER_GENERATED)/features.c: FORCE
	$(shell $(PYTHON) $(USER_SCRIPTS) features $(USER_GENERATED) $(LOG_ARG))
SRC += $(USER_GENERATED)/features.c

# create a char *keycode_names[] based on qmk.keycodes.load_spec and keymap.c
$(USER_GENERATED)/keycode_str.c: FORCE
	$(shell $(PYTHON) $(USER_SCRIPTS) keycode_str $(USER_GENERATED) $(KEYMAP_C) $(LOG_ARG))
SRC += $(USER_GENERATED)/keycode_str.c

# QP assets
ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    VPATH += $(USER_PATH)/painter/fonts \
             $(USER_PATH)/painter/images

    # not really codegen, but lets convert images to .qgf here
    # it will iterate all files on every folder under $(USER_PATH)/painter/images
    QGF_FORMAT ?= rgb565
    $(shell $(USER_PATH)/painter/images/img2qgf $(QGF_FORMAT))

    # actual codegen
    QP_DIRS := $(KEYBOARD_PATHS) $(KEYMAP_PATH) $(USER_PATH)

    $(USER_GENERATED)/qp_resources.c $(USER_GENERATED)/features_draw.c: FORCE
    	$(shell $(PYTHON) $(USER_SCRIPTS) qp_resources $(USER_GENERATED) $(QP_DIRS) $(LOG_ARG))
    SRC += $(USER_GENERATED)/qp_resources.c $(USER_GENERATED)/features_draw.c

    include $(USER_GENERATED)/qp_resources.mk
endif

# TODO: remove this target (always run) by actual dependencies on the generated files' targets
FORCE:
