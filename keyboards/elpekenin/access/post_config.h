// Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// real size, used for init function
#define _IL91874_WIDTH 176
#define _IL91874_HEIGHT 264
#define _ILI9163_WIDTH 130
#define _ILI9163_HEIGHT 130
#define _ILI9341_WIDTH 240
#define _ILI9341_HEIGHT 320

// validate rotation value
#if !(ILI9163_ROTATION == 0 || ILI9163_ROTATION == 1 || ILI9163_ROTATION == 2 || ILI9163_ROTATION == 3)
#    error ILI9163_ROTATION has to be within 0-3 (both included)
#endif
#if !(ILI9341_ROTATION == 0 || ILI9341_ROTATION == 1 || ILI9341_ROTATION == 2 || ILI9341_ROTATION == 3)
#    error ILI9341_ROTATION has to be within 0-3 (both included)
#endif
#if !(IL91874_ROTATION == 0 || IL91874_ROTATION == 1 || IL91874_ROTATION == 2 || IL91874_ROTATION == 3)
#    error IL91874_ROTATION has to be within 0-3 (both included)
#endif

// virtual size, used for drawing funcs
#if (ILI9163_ROTATION % 2 == 0)
#    define ILI9163_WIDTH _ILI9163_WIDTH
#    define ILI9163_HEIGHT _ILI9163_HEIGHT
#else
#    define ILI9163_WIDTH _ILI9163_HEIGHT
#    define ILI9163_HEIGHT _ILI9163_WIDTH
#endif

#if (ILI9341_ROTATION % 2 == 0)
#    define ILI9341_WIDTH _ILI9341_WIDTH
#    define ILI9341_HEIGHT _ILI9341_HEIGHT
#else
#    define ILI9341_WIDTH _ILI9341_HEIGHT
#    define ILI9341_HEIGHT _ILI9341_WIDTH
#endif

#if (IL91874_ROTATION % 2 == 0)
#    define IL91874_WIDTH _IL91874_WIDTH
#    define IL91874_HEIGHT _IL91874_HEIGHT
#else
#    define IL91874_WIDTH _IL91874_HEIGHT
#    define IL91874_HEIGHT _IL91874_WIDTH
#endif
