/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#define CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_DEVICES 2
/* Use a default pairing code if one hasn't been provisioned in flash. */
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00

/* When WiFi low power mode is enabled, 
 * increase MRP local active retry interval to work with 
 * 3 DTIMs and the Beacon interval equal to 102.4 ms.
*/
#ifdef CONFIG_NRF_WIFI_LOW_POWER
    #define CHIP_CONFIG_ALLOW_RUNTIME_MRP_OVERRIDE 1
#endif
