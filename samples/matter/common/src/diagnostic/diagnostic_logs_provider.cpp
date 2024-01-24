/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <app/util/config.h>

#ifndef EMBER_AF_DIAGNOSTIC_LOGS_CLUSTER_SERVER_ENDPOINT_COUNT
#error "Diagnostics cluster is not enabled in the root endpoint"
#endif

#include "diagnostic_logs_provider.h"

#include <app/clusters/diagnostic-logs-server/diagnostic-logs-server.h>

using namespace chip;
using namespace chip::app::Clusters::DiagnosticLogs;
using namespace Nrf;
using namespace Nrf::Matter;

namespace
{
bool IsValidIntent(IntentEnum intent)
{
	return intent < IntentEnum::kUnknownEnumValue;
}

const char *GetIntentStr(IntentEnum intent)
{
	switch (intent) {
	case IntentEnum::kEndUserSupport:
		return "end user";
	case IntentEnum::kNetworkDiag:
		return "network";
	case IntentEnum::kCrashLogs:
		return "crash";
	default:
		return "unknown";
	}

	return "unknown";
}
} /* namespace */

CHIP_ERROR DiagnosticLogProvider::GetCrashLogs(chip::MutableByteSpan &outBuffer, bool &outIsEndOfLog)
{
	return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR DiagnosticLogProvider::ClearCrashLogs()
{
	return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR DiagnosticLogProvider::StartLogCollection(IntentEnum intent, LogSessionHandle &outHandle,
						     Optional<uint64_t> &outTimeStamp,
						     Optional<uint64_t> &outTimeSinceBoot)
{
	VerifyOrReturnError(IsValidIntent(intent), CHIP_ERROR_INVALID_ARGUMENT);

	uint16_t freeSlot = mIntentMap.GetFirstFreeSlot();
	VerifyOrReturnError(freeSlot < kMaxLogSessionHandle, CHIP_ERROR_NO_MEMORY);

	mIntentMap.Insert(freeSlot, std::move(intent));
	outHandle = freeSlot;

	ChipLogProgress(Zcl, "Starting Log collection for %s with session handle %d", GetIntentStr(intent), outHandle);

	return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticLogProvider::EndLogCollection(LogSessionHandle sessionHandle)
{
	VerifyOrReturnError(sessionHandle < kMaxLogSessionHandle, CHIP_ERROR_NO_MEMORY);
	VerifyOrReturnError(mIntentMap.Contains(sessionHandle), CHIP_ERROR_INTERNAL);

	CHIP_ERROR err = CHIP_NO_ERROR;

	IntentEnum intent = mIntentMap[sessionHandle];

	/* Old intent is no needed anymore*/
	mIntentMap.Erase(sessionHandle);

	/* Do the specific action for the removing intent */
	switch (intent) {
	case IntentEnum::kEndUserSupport:
		/* TODO: add support for End User Intent */
		return CHIP_ERROR_NOT_IMPLEMENTED;
	case IntentEnum::kNetworkDiag:
		/* TODO: add support for End User Intent */
		return CHIP_ERROR_NOT_IMPLEMENTED;
	case IntentEnum::kCrashLogs:
		err = ClearCrashLogs();
		break;
	default:
		return CHIP_ERROR_INVALID_ARGUMENT;
	}

	ChipLogProgress(Zcl, "Ending Log collection for %s with session handle %d",
			GetIntentStr(mIntentMap[sessionHandle]), sessionHandle);

	return err;
}

CHIP_ERROR DiagnosticLogProvider::CollectLog(LogSessionHandle sessionHandle, MutableByteSpan &outBuffer,
					     bool &outIsEndOfLog)
{
	VerifyOrReturnError(sessionHandle < kMaxLogSessionHandle, CHIP_ERROR_NO_MEMORY);
	VerifyOrReturnError(mIntentMap.Contains(sessionHandle), CHIP_ERROR_INTERNAL);

	CHIP_ERROR err = CHIP_NO_ERROR;

	switch (mIntentMap[sessionHandle]) {
	case IntentEnum::kEndUserSupport:
		/* TODO: add support for End User Intent */
		return CHIP_ERROR_NOT_IMPLEMENTED;
	case IntentEnum::kNetworkDiag:
		/* TODO: add support for Network Diag Intent */
		return CHIP_ERROR_NOT_IMPLEMENTED;
	case IntentEnum::kCrashLogs:
		err = GetCrashLogs(outBuffer, outIsEndOfLog);
		VerifyOrReturnError(CHIP_NO_ERROR == err, err, outBuffer.reduce_size(0));
		break;
	default:
		return CHIP_ERROR_INVALID_ARGUMENT;
	}

	return CHIP_NO_ERROR;
}

size_t DiagnosticLogProvider::GetSizeForIntent(IntentEnum intent)
{
	/* TODO: Add implementation of this method */
	return 0;
}

CHIP_ERROR DiagnosticLogProvider::GetLogForIntent(IntentEnum intent, MutableByteSpan &outBuffer,
						  Optional<uint64_t> &outTimeStamp,
						  Optional<uint64_t> &outTimeSinceBoot)
{
	LogSessionHandle sessionHandle = kInvalidLogSessionHandle;

	CHIP_ERROR err = StartLogCollection(intent, sessionHandle, outTimeStamp, outTimeSinceBoot);
	VerifyOrExit(CHIP_NO_ERROR == err, );

	bool unusedOutIsEndOfLog;
	err = CollectLog(sessionHandle, outBuffer, unusedOutIsEndOfLog);
	VerifyOrExit(CHIP_NO_ERROR == err, );

	err = EndLogCollection(sessionHandle);

exit:
	if (CHIP_NO_ERROR != err) {
		outBuffer.reduce_size(0);
	}

	return err;
}

void emberAfDiagnosticLogsClusterInitCallback(chip::EndpointId endpoint)
{
	auto &logProvider = DiagnosticLogProvider::GetInstance();

	DiagnosticLogsServer::Instance().SetDiagnosticLogsProviderDelegate(endpoint, &logProvider);
}
