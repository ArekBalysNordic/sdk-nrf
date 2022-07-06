/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "thread_util.h"

#include <app/server/Dnssd.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/internal/DeviceNetworkInfo.h>

#ifdef CONFIG_MPSL_FEM
#include <openthread/platform/radio.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#endif

#include <zephyr/zephyr.h>

#ifdef CONFIG_MPSL_FEM
#ifndef CONFIG_FEM_802_15_4_DEFAULT_TX_POWER
#define CONFIG_FEM_802_15_4_DEFAULT_TX_POWER 20
#endif
constexpr static int8_t kMinThreadOutputPower = -40;
constexpr static int8_t kMaxThreadOutputPower = 20;
#endif

void StartDefaultThreadNetwork(uint64_t datasetTimestamp)
{
	using namespace chip::DeviceLayer;

	constexpr uint8_t masterkey[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
					  0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

	chip::Thread::OperationalDataset dataset{};

	uint8_t xpanid[8];
	net_bytes_from_str(xpanid, sizeof(xpanid), CONFIG_OPENTHREAD_XPANID);

	dataset.SetChannel(CONFIG_OPENTHREAD_CHANNEL);
	dataset.SetExtendedPanId(xpanid);
	dataset.SetMasterKey(masterkey);
	dataset.SetNetworkName(CONFIG_OPENTHREAD_NETWORK_NAME);
	dataset.SetPanId(CONFIG_OPENTHREAD_PANID);

	if (datasetTimestamp != 0) {
		dataset.SetActiveTimestamp(datasetTimestamp);
	}

	ThreadStackMgr().SetThreadEnabled(false);
	ThreadStackMgr().SetThreadProvision(dataset.AsByteSpan());
	ThreadStackMgr().SetThreadEnabled(true);

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
	chip::app::DnssdServer::Instance().StartServer();
#endif
}

#ifdef CONFIG_MPSL_FEM
CHIP_ERROR SetDefaultThreadOutputPower()
{
	CHIP_ERROR err;
	/* set output power when FEM is active */
	if (CONFIG_FEM_802_15_4_DEFAULT_TX_POWER != 0 &&
	    CONFIG_FEM_802_15_4_DEFAULT_TX_POWER >= kMinThreadOutputPower &&
	    CONFIG_FEM_802_15_4_DEFAULT_TX_POWER <= kMaxThreadOutputPower) {
		err = chip::DeviceLayer::Internal::MapOpenThreadError(
			otPlatRadioSetTransmitPower(chip::DeviceLayer::ThreadStackMgrImpl().OTInstance(),
						    static_cast<int8_t>(CONFIG_FEM_802_15_4_DEFAULT_TX_POWER)));
	} else {
		return CHIP_ERROR_INVALID_INTEGER_VALUE;
	}
	return err;
}
#endif
