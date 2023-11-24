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

#ifndef FUNCTION_BUTTON
#define FUNCTION_BUTTON DK_BTN1
#endif
#ifndef FUNCTION_BUTTON_MASK
#define FUNCTION_BUTTON_MASK DK_BTN1_MSK
#endif

#if NUMBER_OF_BUTTONS == 2
#define BLE_ADVERTISEMENT_START_AND_APPLICATION_BUTTON DK_BTN2
#define BLE_ADVERTISEMENT_START_AND_APPLICATION_BUTTON_MASK DK_BTN2_MSK
#else
#define APPLICATION_BUTTON DK_BTN2
#define APPLICATION_BUTTON_MASK DK_BTN2_MSK
#define USER_BUTTON_1 DK_BTN3
#define USER_BUTTON_1_MASK DK_BTN3_MSK
#define USER_BUTTON_2 DK_BTN4
#define USER_BUTTON_2_MASK DK_BTN4_MSK
#endif

#define SYSTEM_STATE_LED DK_LED1
#define APPLICATION_STATE_LED DK_LED2
#if NUMBER_OF_LEDS == 4
#define USER_LED_1 DK_LED3
#define USER_LED_2 DK_LED4
#endif
