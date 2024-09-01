// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

/**
 * Generate (pseudo-)random numbers.
 *
 * .. note::
 *   This is just a convenience layer on top of ``lib8tion.h``
 */

/**
 * ----
 */

// -- barrier --

#pragma once

// TODO?: Conditional import
// on the header so that user code can already call ADC for seeing
#include <platforms/chibios/drivers/analog.h>

/**
 * Set the seed for the RNG.
 *
 * .. tip::
 *   I do it by reading ADC's noise twice and multiplying the values.
 */
void rng_set_seed(uint16_t seed);

/**
 * Generate a random number in the ``[min, max]`` range.
 *
 * It will also add some entropy to the RNG.
 */
uint16_t rng_min_max(uint16_t min, uint16_t max);
