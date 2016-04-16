/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#if defined(SIMU)
  #define __ALIGNED
#else
  #define __ALIGNED __attribute__((aligned(32)))
#endif

#if defined(SIMU)
  #define __DMA
#elif defined(STM32F4) && !defined(BOOT)
  #define __DMA __attribute__((section(".ram"), aligned(32)))
#else
  #define __DMA __ALIGNED
#endif

#if defined(PCBHORUS) && !defined(SIMU)
  #define __SDRAM __attribute__((section(".sdram"), aligned(32)))
#else
  #define __SDRAM __DMA
#endif

#if defined(PCBHORUS) && !defined(SIMU)
  #define __NOINIT __attribute__((section(".noinit")))
#else
  #define __NOINIT
#endif

#if defined(SIMU) || defined(CPUARM) || GCC_VERSION < 472
  typedef int32_t int24_t;
#else
  typedef __int24 int24_t;
#endif

#endif // _DEFINITIONS_H_
