USER_GENERATED = $(USER_PATH)/generated

PYTHON = python3
COMMON_ARGS = --log $(LOG_FOLDER)/python.txt
CODEGEN_ARGS = --output $(USER_GENERATED)

# make sure folder exists
$(shell mkdir -p $(USER_GENERATED))

# enabled_features_t
$(USER_GENERATED)/features.c: FORCE
	$(shell $(PYTHON) manage.py $(COMMON_ARGS) features $(CODEGEN_ARGS))
SRC += $(USER_GENERATED)/features.c

# create a char *keycode_names[] based on qmk.keycodes.load_spec and keymap.c
#
# NOTE: when both json and c keyamps exist, they are KEYMAP_C and OTHER_KEYMAP_C respectively, apparently
# TODO: support C-only build
$(USER_GENERATED)/keycode_str.c: FORCE
	$(shell $(PYTHON) manage.py $(COMMON_ARGS) keycode_str $(CODEGEN_ARGS) $(OTHER_KEYMAP_C))
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
    	$(shell $(PYTHON) manage.py $(COMMON_ARGS) qp_resources $(CODEGEN_ARGS) $(QP_DIRS))
    SRC += $(USER_GENERATED)/qp_resources.c $(USER_GENERATED)/features_draw.c

    include $(USER_GENERATED)/qp_resources.mk
endif

# TODO: remove this target (always run) by actual dependencies on the generated files' targets
FORCE:
