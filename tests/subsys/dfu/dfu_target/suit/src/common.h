/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef DFU_TARGET_SUIT_TEST_COMMON_H__
#define DFU_TARGET_SUIT_TEST_COMMON_H__

#include <stdint.h>
#include <stddef.h>

#include <sdfw/sdfw_services/suit_service.h>
#include <suit_plat_mem_util.h>
#include <suit_platform.h>

#include <zephyr/ztest.h>
#include <zephyr/devicetree.h>
#include <zephyr/fff.h>

/**
 * @brief Reset all mocked functions.
 *
 */
void reset_fakes(void);

DECLARE_FAKE_VALUE_FUNC(int, suit_trigger_update, suit_plat_mreg_t *, size_t);

#endif // DFU_TARGET_SUIT_TEST_COMMON_H__