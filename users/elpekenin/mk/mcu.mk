ifeq ($(MCU_SERIES), RP2040)
    # NOTE: QMK adds divider and int64_ops

    # custom script to keep preinit symbol from SDK, needed to init wrappers
    # bake a custom LD with some macros for other similar custom sections
    # NOTE: gcc -E does not seem to like .ld's
    # TODO: decouple to reuse on other boards
    $(shell gcc -E $(USER_PATH)/ld/custom_rp2040.h \
                -P \
                -I $(USER_PATH) -I $(USER_PATH)/include \
                -o $(TOP_DIR)/platforms/chibios/boards/common/ld/elpekenin.ld)
    MCU_LDSCRIPT = elpekenin

    # tell ChibiOS' crt0_v6m.S to enable second core
    OPT_DEFS += -UCRT0_EXTRA_CORES_NUMBER \
                -DCRT0_EXTRA_CORES_NUMBER=1

    # ... however, second core's main will be noop without this set
    USE_SECOND_CORE ?= no
    ifeq ($(strip $(USE_SECOND_CORE)), yes)
        OPT_DEFS += -DUSE_SECOND_CORE
    endif

    # wrap some periodic logic, so that QMK's mainloop (core 0)
    # does nothing and we will execute it on the second one instead
    SECOND_CORE_TASKS ?= $(USE_SECOND_CORE)
    ifeq ($(strip $(SECOND_CORE_TASKS)), yes)
        OPT_DEFS += -DSECOND_CORE_TASKS
        MAIN_TASKS := qp_internal_task deferred_exec_task housekeeping_task
        $(call WRAP, $(MAIN_TASKS))
    endif

    SECOND_CORE_MATRIX ?= $(USE_SECOND_CORE)
    ifeq ($(strip $(SECOND_CORE_MATRIX)), yes)
        OPT_DEFS += -DSECOND_CORE_MATRIX
    endif

    # sdk wrappers init + c1 main
    SRC += $(USER_SRC)/mcu/rp2040.c

    PICO_SDK_WRAPPERS ?= yes
    ifeq ($(strip $(PICO_SDK_WRAPPERS)), yes)
        # notes for future self
        # *********************
        #  - my chips are B1
        #  - *_IN_RAM=1 is pretty much useless, since the functions relocated by it are
        #    for the most part copying a few registers + jumping to the code thru vtable
        OPT_DEFS += -DPICO_SDK_WRAPPERS_ENABLED \
                    -DPICO_BITS_IN_RAM=1 \
                    -DPICO_DOUBLE_IN_RAM=1 \
                    -DPICO_FLOAT_IN_RAM=1

        # bit
        SRC += $(PICOSDKROOT)/src/rp2_common/pico_bit_ops/bit_ops_aeabi.S
        BIT_FUNCTIONS := __clzsi2 __clzdi2 __ctzsi2 __ctzdi2 __popcountsi2 __popcountdi2
        $(call WRAP, $(BIT_FUNCTIONS))

        # double
        VPATH += $(PICOSDKROOT)/src/rp2_common/pico_double/include
        SRC   += $(PICOSDKROOT)/src/rp2_common/pico_double/double_aeabi.S \
                 $(PICOSDKROOT)/src/rp2_common/pico_double/double_init_rom.c \
                 $(PICOSDKROOT)/src/rp2_common/pico_double/double_math.c \
                 $(PICOSDKROOT)/src/rp2_common/pico_double/double_v1_rom_shim.S
        DOUBLE_FUNCTIONS := __aeabi_dadd __aeabi_ddiv __aeabi_dmul __aeabi_drsub __aeabi_dsub __aeabi_cdcmpeq \
                            __aeabi_cdrcmple __aeabi_cdcmple __aeabi_dcmpeq __aeabi_dcmplt __aeabi_dcmple \
                            __aeabi_dcmpge __aeabi_dcmpgt __aeabi_dcmpun __aeabi_i2d __aeabi_l2d __aeabi_ui2d \
                            __aeabi_ul2d __aeabi_d2iz __aeabi_d2lz __aeabi_d2uiz __aeabi_d2ulz __aeabi_d2f \
                            sqrt cos sin tan atan2 exp log ldexp copysign trunc floor ceil round sincos asin \
                            acos atan sinh cosh tanh asinh acosh atanh exp2 log2 exp10 log10 pow powint hypot \
                            cbrt fmod drem remainder remquo expm1 log1p fma
        $(call WRAP, $(DOUBLE_FUNCTIONS))

        # float
        VPATH += $(PICOSDKROOT)/src/rp2_common/pico_float/include
        SRC   += $(PICOSDKROOT)/src/rp2_common/pico_float/float_aeabi.S \
                 $(PICOSDKROOT)/src/rp2_common/pico_float/float_init_rom.c \
                 $(PICOSDKROOT)/src/rp2_common/pico_float/float_math.c \
                 $(PICOSDKROOT)/src/rp2_common/pico_float/float_v1_rom_shim.S
        FLOAT_FUNCTIONS := __aeabi_fadd __aeabi_fdiv __aeabi_fmul __aeabi_frsub __aeabi_fsub __aeabi_cfcmpeq \
                           __aeabi_cfrcmple __aeabi_cfcmple __aeabi_fcmpeq __aeabi_fcmplt __aeabi_fcmple \
                           __aeabi_fcmpge __aeabi_fcmpgt __aeabi_fcmpun __aeabi_i2f __aeabi_l2f __aeabi_ui2f \
                           __aeabi_ul2f __aeabi_f2iz __aeabi_f2lz __aeabi_f2uiz __aeabi_f2ulz __aeabi_f2d \
                           sqrtf cosf sinf tanf atan2f expf logf ldexpf copysignf truncf floorf ceilf roundf \
                           sincosf asinf acosf atanf sinhf coshf tanhf asinhf acoshf atanhf exp2f log2f exp10f \
                           log10f powf powintf hypotf cbrtf fmodf dremf remainderf remquof expm1f log1pf fmaf
        $(call WRAP, $(FLOAT_FUNCTIONS))

        # mem, harder to adapt to ChibiOS' bootstrap/init (at crt1.c)
        # SRC += $(PICOSDKROOT)/src/rp2_common/pico_mem_ops/mem_ops_aeabi.S
        # MEM_FUNCTIONS := memcpy __aeabi_memcpy __aeabi_memcpy4 __aeabi_memcpy8 memset __aeabi_memset __aeabi_memset4 __aeabi_memset8
        # $(call WRAP, $(MEM_FUNCTIONS))
    endif
endif
