/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "test_fd_user_data.h"
#include "board/board.h"

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

void Nrf::Matter::ParseUserDataForTestingPurposes(chip::DeviceLayer::FactoryDataProviderBase *provider)
{
	if (!provider) {
		return;
	}

	int32_t matterBleAdvValue = 0;
	size_t matterBleAdvValueSize = sizeof(matterBleAdvValue);

	if (CHIP_NO_ERROR == provider->GetUserKey("matter_ble_adv", &matterBleAdvValue, matterBleAdvValueSize)) {
		if (matterBleAdvValue && matterBleAdvValueSize == sizeof(matterBleAdvValue)) {
			LOG_INF("Found request to start BLE advertisement in the factory data. Starting...");
			Nrf::Board::StartBLEAdvertisement();
		} else {
			LOG_INF("Found request to start BLE advertisement in the factory data. Skipping...");
		}
	}
}
