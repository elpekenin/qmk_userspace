# tell linker to wrap a function, applied to a list of names
define WRAP
    $(foreach FUNC,$1,$(eval EXTRALDFLAGS += -Wl,--wrap=$(FUNC)))
endef
