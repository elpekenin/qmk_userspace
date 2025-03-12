// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include_next <mcuconf.h>

#undef RP_PWM_USE_PWM5
#define RP_PWM_USE_PWM5 TRUE

#undef RP_SPI_USE_SPI0
#define RP_SPI_USE_SPI0 TRUE

#undef RP_SPI_USE_SPI1
#define RP_SPI_USE_SPI1 TRUE

#undef RP_ADC_USE_ADC1
#define RP_ADC_USE_ADC1 TRUE
