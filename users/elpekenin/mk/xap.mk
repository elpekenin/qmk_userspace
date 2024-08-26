ifeq ($(strip $(XAP_ENABLE)), yes)
    SRC += $(USER_SRC)/xap.c

    # TODO: Re-write client code with newer version
    #       And use name instead of id to get assets
    QP_XAP ?= no
    ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
        ifeq ($(strip $(QP_XAP)), yes)
            OPT_DEFS += -DQP_XAP_ENABLE
            SRC += $(USER_SRC)/qp/xap.c
        endif
    endif
endif
