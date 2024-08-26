ifeq ($(strip $(MAKECMDGOALS)), uf2-split-left)
    QUANTUM_PAINTER_DRIVERS += il91874_spi
else ifeq ($(strip $(MAKECMDGOALS)), uf2-split-right)
    QUANTUM_PAINTER_DRIVERS += ili9163_spi ili9341_spi surface
endif