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

#include "../../pwr.h"
#include "../horus/board_horus.h"

uint32_t shutdownRequest;          // Stores intentional shutdown to avoid reboot loop
uint32_t shutdownReason;           // Used for detecting unexpected reboots regardless of reason
uint32_t powerupReason __NOINIT;   // Stores power up reason beyond initialization for emergency mode activation

void pwrInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = PWR_ON_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(PWR_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = PWR_SWITCH_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_Init(PWR_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = AUDIO_SHUTDOWN_GPIO_PIN;
  GPIO_Init(AUDIO_SHUTDOWN_GPIO, &GPIO_InitStructure);

  // Init Module PWR
  // TODO not here
  GPIO_ResetBits(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN);
  GPIO_InitStructure.GPIO_Pin = INTMODULE_PWR_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init(INTMODULE_PWR_GPIO, &GPIO_InitStructure);

  GPIO_ResetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN);
  GPIO_InitStructure.GPIO_Pin = EXTMODULE_PWR_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init(EXTMODULE_PWR_GPIO, &GPIO_InitStructure);

  pwrOn();
}

void pwrOn()
{
  GPIO_SetBits(PWR_GPIO, PWR_ON_GPIO_PIN);
  shutdownRequest = NO_SHUTDOWN_REQUEST;
  shutdownReason = DIRTY_SHUTDOWN;
}

void pwrOff()
{
  // Shutdown the Audio amp
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = AUDIO_SHUTDOWN_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(AUDIO_SHUTDOWN_GPIO, &GPIO_InitStructure);
  GPIO_ResetBits(AUDIO_SHUTDOWN_GPIO, AUDIO_SHUTDOWN_GPIO_PIN);

  // Shutdown the Haptic
  hapticDone();

  shutdownRequest = SHUTDOWN_REQUEST;
  shutdownReason = NORMAL_POWER_OFF;
  GPIO_ResetBits(PWR_GPIO, PWR_ON_GPIO_PIN);
}

uint32_t pwrPressed()
{
  return GPIO_ReadInputDataBit(PWR_GPIO, PWR_SWITCH_GPIO_PIN) == Bit_RESET;
}

void pwrResetHandler()
{
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOJEN;

  // these two NOPs are needed (see STM32F errata sheet) before the peripheral 
  // register can be written after the peripheral clock was enabled
  __ASM volatile ("nop");
  __ASM volatile ("nop");

  // Turn soft power ON now, but only if we got started because of the watchdog
  // or software reset. If the radio was started by user pressing the power button
  // then that button is providing power and we don't need to enable it here.
  //
  // If we were to turn it on here indiscriminately, then the radio can go into the 
  // power on/off loop after being powered off by the user. (issue #2790)
  if (UNREQUESTED_SHUTDOWN()) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = PWR_ON_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(PWR_GPIO, &GPIO_InitStructure);

    if (shutdownReason == DIRTY_SHUTDOWN) {
      powerupReason = DIRTY_SHUTDOWN;
    }

    pwrOn();
  }
}
