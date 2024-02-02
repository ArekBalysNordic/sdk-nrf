/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "event_triggers.h"

#include <app/server/Server.h>
#include <platform/nrfconnect/Reboot.h>

#if defined(CONFIG_CHIP_DEVICE_PRODUCT_ID) && CONFIG_CHIP_DEVICE_PRODUCT_ID == 32774
#include <app/clusters/door-lock-server/door-lock-server.h>

namespace
{
/* Ensure that the Lock endpoint is the same as in the app_task.cpp file */
constexpr uint16_t kLockEndpointId = 1;
} /* namespace */
#endif

using namespace Nrf::Matter;
using namespace chip;

bool TestEventTrigger::DoesEnableKeyMatch(const ByteSpan &enableKey) const
{
	return !mEnableKey.empty() && mEnableKey.data_equal(enableKey);
}

CHIP_ERROR TestEventTrigger::HandleEventTriggers(uint64_t eventTrigger)
{
	switch (eventTrigger) {
	case EventTriggers::kFactoryReset:
		Server::GetInstance().ScheduleFactoryReset();
		break;
	case EventTriggers::kReboot:
		DeviceLayer::SystemLayer().ScheduleLambda(
			[] { DeviceLayer::Reboot(DeviceLayer::SoftwareRebootReason::kOther); });
		break;
	case EventTriggers::kDiagnosticLogsCrash: {
		/* Trigger the execute of the undefined instruction attempt */
		DeviceLayer::SystemLayer().ScheduleLambda([] {
			uint8_t *undefined = nullptr;
			*undefined = 5;
		});
	} break;
#if defined(CONFIG_CHIP_DEVICE_PRODUCT_ID) && CONFIG_CHIP_DEVICE_PRODUCT_ID == 32774
	case EventTriggers::kDoorLockJammed:
		VerifyOrReturnError(DoorLockServer::Instance().SendLockAlarmEvent(kLockEndpointId,
										  AlarmCodeEnum::kLockJammed),
				    CHIP_ERROR_INTERNAL);
		break;
#endif /* CHIP_DEVICE_PRODUCT_ID == 32774 */
#ifdef CONFIG_CHIP_OTA_REQUESTOR
	case EventTriggers::kOtaStart:
		mOtaTestEventTrigger.HandleEventTrigger(eventTrigger);
		break;
#endif /* CONFIG_CHIP_OTA_REQUESTOR */
	default:
		return CHIP_ERROR_NOT_IMPLEMENTED;
	}
	return CHIP_NO_ERROR;
}
