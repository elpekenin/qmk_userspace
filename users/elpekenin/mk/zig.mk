# Compile into a static library, and link it against QMK by means of SRC
ELPEKENIN_LIB = $(USER_PATH)/zig-out/lib/libelpekenin.a

$(ELPEKENIN_LIB):
	cd $(USER_PATH) && zig build -Doptimize=ReleaseFast -Dmcu=$(MCU_SERIES) > $(LOG_FOLDER)/zig.out 2> $(LOG_FOLDER)/zig.err

# # $(if $(ZIG_RET),,$(error Compiling zig failed))
SRC += $(ELPEKENIN_LIB)
