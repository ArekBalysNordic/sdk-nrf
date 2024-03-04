/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "event_triggers.h"

#include <app/server/Server.h>
#include <platform/nrfconnect/Reboot.h>

#ifdef CONFIG_NCS_SAMPLE_MATTER_DIAGNOSTIC_LOGS_TEST
#include "diagnostic/diagnostic_logs_provider.h"
#endif

#if defined(CONFIG_CHIP_DEVICE_PRODUCT_ID) && CONFIG_CHIP_DEVICE_PRODUCT_ID == 32774
#include <app/clusters/door-lock-server/door-lock-server.h>

namespace
{
/* Ensure that the Lock endpoint is the same as in the app_task.cpp file */
constexpr uint16_t kLockEndpointId = 1;

#ifdef CONFIG_NCS_SAMPLE_MATTER_DIAGNOSTIC_LOGS_TEST
constexpr size_t kMaxTestingLogsSingleSize = 1024;
char sTempLogBuffer[kMaxTestingLogsSingleSize];
#endif

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
#ifdef CONFIG_CHIP_OTA_REQUESTOR
	if (eventTrigger == EventTriggers::kOtaStart) {
		mOtaTestEventTrigger.HandleEventTrigger(eventTrigger);
		return CHIP_NO_ERROR;
	}
#endif /* CONFIG_CHIP_OTA_REQUESTOR */

	switch (eventTrigger & EventTriggerMask) {
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
#ifdef CONFIG_NCS_SAMPLE_MATTER_DIAGNOSTIC_LOGS_TEST
	case EventTriggers::kDiagnosticLogsUser: {
		memset(sTempLogBuffer, 0x6E, sizeof(sTempLogBuffer));

		size_t logSize = static_cast<size_t>(eventTrigger & kDiagnosticLogsValue);

		if (logSize > kMaxTestingLogsSingleSize) {
			return CHIP_ERROR_NO_MEMORY;
		}

		ChipLogProgress(Zcl, "Storing %zu User logs", logSize);
		if (logSize == 0) {
			DiagnosticLogProvider::GetInstance().ClearTestingBuffer(
				chip::app::Clusters::DiagnosticLogs::IntentEnum::kEndUserSupport);
		} else {
			VerifyOrReturnError(DiagnosticLogProvider::GetInstance().StoreTestingLog(
						    chip::app::Clusters::DiagnosticLogs::IntentEnum::kEndUserSupport,
						    sTempLogBuffer, logSize),
					    CHIP_ERROR_NO_MEMORY);
		}

	} break;
	case EventTriggers::kDiagnosticLogsNetwork: {
		memset(sTempLogBuffer, 0x75, sizeof(sTempLogBuffer));

		size_t logSize = static_cast<size_t>(eventTrigger & kDiagnosticLogsValue);

		if (logSize > kMaxTestingLogsSingleSize) {
			return CHIP_ERROR_NO_MEMORY;
		}

		ChipLogProgress(Zcl, "Storing %zu Network logs", logSize);

		if (logSize == 0) {
			DiagnosticLogProvider::GetInstance().ClearTestingBuffer(
				chip::app::Clusters::DiagnosticLogs::IntentEnum::kNetworkDiag);
		} else {
			VerifyOrReturnError(DiagnosticLogProvider::GetInstance().StoreTestingLog(
						    chip::app::Clusters::DiagnosticLogs::IntentEnum::kNetworkDiag,
						    sTempLogBuffer, logSize),
					    CHIP_ERROR_NO_MEMORY);
		}
	} break;
#endif
#if defined(CONFIG_CHIP_DEVICE_PRODUCT_ID) && CONFIG_CHIP_DEVICE_PRODUCT_ID == 32774
	case EventTriggers::kDoorLockJammed:
		VerifyOrReturnError(DoorLockServer::Instance().SendLockAlarmEvent(kLockEndpointId,
										  AlarmCodeEnum::kLockJammed),
				    CHIP_ERROR_INTERNAL);
		break;
#endif /* CHIP_DEVICE_PRODUCT_ID == 32774 */
	default:
		return CHIP_ERROR_NOT_IMPLEMENTED;
	}

	return CHIP_NO_ERROR;
}
