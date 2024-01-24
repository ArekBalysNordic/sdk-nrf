/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "persistent_storage/persistent_storage_util.h"
#include "util/finite_map.h"

#include <app/clusters/diagnostic-logs-server/DiagnosticLogsProviderDelegate.h>
#include <lib/core/CHIPError.h>

namespace Nrf::Matter
{

class DiagnosticLogProvider : public chip::app::Clusters::DiagnosticLogs::DiagnosticLogsProviderDelegate {
public:
	static inline DiagnosticLogProvider &GetInstance()
	{
		static DiagnosticLogProvider sInstance;
		return sInstance;
	}

	/* DiagnosticsLogsProviderDelegate Interface */
	CHIP_ERROR StartLogCollection(chip::app::Clusters::DiagnosticLogs::IntentEnum intent,
				      chip::app::Clusters::DiagnosticLogs::LogSessionHandle &outHandle,
				      chip::Optional<uint64_t> &outTimeStamp,
				      chip::Optional<uint64_t> &outTimeSinceBoot) override;
	CHIP_ERROR EndLogCollection(chip::app::Clusters::DiagnosticLogs::LogSessionHandle sessionHandle) override;
	CHIP_ERROR CollectLog(chip::app::Clusters::DiagnosticLogs::LogSessionHandle sessionHandle,
			      chip::MutableByteSpan &outBuffer, bool &outIsEndOfLog) override;
	size_t GetSizeForIntent(chip::app::Clusters::DiagnosticLogs::IntentEnum intent) override;
	CHIP_ERROR GetLogForIntent(chip::app::Clusters::DiagnosticLogs::IntentEnum intent,
				   chip::MutableByteSpan &outBuffer, chip::Optional<uint64_t> &outTimeStamp,
				   chip::Optional<uint64_t> &outTimeSinceBoot) override;

private:
	/* The maximum number of the simultaneous sessions */
	constexpr static uint16_t kMaxLogSessionHandle =
		CONFIG_NCS_SAMPLE_MATTER_DIAGNOSTIC_LOGS_MAX_SIMULTANEOUS_SESSIONS;

	static_assert(kMaxLogSessionHandle < chip::app::Clusters::DiagnosticLogs::kInvalidLogSessionHandle);

	DiagnosticLogProvider()
		: mDiagnosticLogsStorageNode("dl", strlen("dl")),
		  mCrashLogsStorageNode("cl", strlen("cl"), &mDiagnosticLogsStorageNode)
	{
	}

	/* Not copyable */
	DiagnosticLogProvider(const DiagnosticLogProvider &) = delete;
	DiagnosticLogProvider &operator=(const DiagnosticLogProvider &) = delete;
	/* Not movable */
	DiagnosticLogProvider(DiagnosticLogProvider &&) = delete;
	DiagnosticLogProvider &operator=(DiagnosticLogProvider &&) = delete;

	CHIP_ERROR GetCrashLogs(chip::MutableByteSpan &outBuffer, bool &outIsEndOfLog);
	CHIP_ERROR ClearCrashLogs();

	Nrf::PersistentStorageNode mDiagnosticLogsStorageNode;
	Nrf::PersistentStorageNode mCrashLogsStorageNode;

	Nrf::FiniteMap<chip::app::Clusters::DiagnosticLogs::IntentEnum, kMaxLogSessionHandle> mIntentMap;
};

} /* namespace Nrf::Matter */
