/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <zephyr/devicetree.h>
#include <dk_buttons_and_leds.h>

/* Override application configuration */
#include "app_config.h"

#define LEDS_NODE_ID DT_PATH(leds)
#define BUTTONS_NODE_ID DT_PATH(buttons)
#define INCREMENT_BY_ONE(button_or_led) +1
#define NUMBER_OF_LEDS (0 DT_FOREACH_CHILD(LEDS_NODE_ID, INCREMENT_BY_ONE))
#define NUMBER_OF_BUTTONS (0 DT_FOREACH_CHILD(BUTTONS_NODE_ID, INCREMENT_BY_ONE))

/* User configurable BUTTONS */
#ifndef FUNCTION_BUTTON
#define FUNCTION_BUTTON DK_BTN1
#endif
#ifndef APPLICATION_BUTTON
#define APPLICATION_BUTTON DK_BTN2
#endif
/* User buttons 1 & 2 are available only on DKs that have 4 buttons located on the board */
#if NUMBER_OF_BUTTONS == 4
#ifndef USER_BUTTON_1
#define USER_BUTTON_1 DK_BTN3
#endif
#ifndef USER_BUTTON_2
#define USER_BUTTON_2 DK_BTN4
#endif
#endif

/* User configurable LEDS */
#ifndef SYSTEM_STATE_LED
#define SYSTEM_STATE_LED DK_LED1
#endif
#ifndef APPLICATION_STATE_LED
#define APPLICATION_STATE_LED DK_LED2
#endif 
/* User leds 1 & 2 are available only on DKs that have 4 buttons located on the board */
#if NUMBER_OF_LEDS == 4
#ifndef USER_LED_1
#define USER_LED_1 DK_LED3
#endif
#ifdef USER_LED_2
#define USER_LED_2 DK_LED4
#endif
#endif

/* Non-configurable Buttons */
#if NUMBER_OF_BUTTONS == 4
#define BLUETOOTH_ADV_BUTTON USER_BUTTON_2
#define BLUETOOTH_ADV_BUTTON_MASK USER_BUTTON_2_MASK
#else
#ifndef BLUETOOTH_ADV_BUTTON APPLICATION_BUTTON
#define BLUETOOTH_ADV_BUTTON_MASK APPLICATION_BUTTON_MASK
#endif
#endif



