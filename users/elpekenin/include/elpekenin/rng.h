// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// will be seeding with ADC reading
// TODO?: Conditional
#include <platforms/chibios/drivers/analog.h>

void rng_set_seed(uint16_t seed);
uint16_t rng_min_max(uint16_t min, uint16_t max);
