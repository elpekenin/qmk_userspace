// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define MILLISECONDS(x) ((uint32_t)(x))
#define SECONDS(x) ((x) * MILLISECONDS(1000))
#define MINUTES(x) ((x) * SECONDS(60))
#define HOURS(x) ((x) * MINUTES(60))
#define DAYS(x) ((x) * HOURS(24))
