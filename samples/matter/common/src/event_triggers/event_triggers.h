/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerHandler.h>

#include <cstdint>

namespace Nrf::Matter
{
class TestEventTrigger : public chip::TestEventTriggerDelegate {
public:

	/* Mask to ignore event trigger value*/
	constexpr static uint64_t EventTriggerMask = 0xFFFF'FFFF'FFFF'0000;

	enum EventTriggerMasks : uint64_t {
		kSystem = 0xF000'0000'0000'0000,
		kDiagnostics = 0x0032'0000'0000'0000,
		kMatterStack = 0x0100'0000'0000'0000,
		kDoorLock = 0x3277'4000'0000'0000,
	};

	enum EventTriggers : uint64_t {
		/* System */
		kFactoryReset = EventTriggerMasks::kSystem,
		kReboot = kFactoryReset & 0x0001'0000,
		/* Diagnostic logs cluster */
		kDiagnosticLogsUser = EventTriggerMasks::kDiagnostics | 0x0000'0000'0000,
		kDiagnosticLogsNetwork = EventTriggerMasks::kDiagnostics | 0x0001'0000'0000,
		kDiagnosticLogsCrash = EventTriggerMasks::kDiagnostics | 0x0002'0000'0000,
		/* Door lock cluster */
		kDoorLockJammed = EventTriggerMasks::kDoorLock,
		/* OTA */
		kOtaStart = chip::OTATestEventTriggerHandler::kOtaQueryTrigger,
	};

	enum EventTriggerValueMask : uint64_t {
		kDiagnosticLogsValue = 0xFFFF,
	};

	CHIP_ERROR SetEnableKey(const chip::ByteSpan &newEnableKey)
	{
		VerifyOrReturnError(newEnableKey.size() == chip::TestEventTriggerDelegate::kEnableKeyLength,
				    CHIP_ERROR_INVALID_ARGUMENT);
		return CopySpanToMutableSpan(newEnableKey, mEnableKey);
	}

	/* TestEventTriggerDelegate implementation */
	bool DoesEnableKeyMatch(const chip::ByteSpan &enableKey) const override;
	CHIP_ERROR HandleEventTriggers(uint64_t eventTrigger) override;

private:
	uint8_t mEnableKeyData[chip::TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
										     0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
										     0xcc, 0xdd, 0xee, 0xff };
	chip::MutableByteSpan mEnableKey{ mEnableKeyData };
	chip::OTATestEventTriggerHandler mOtaTestEventTrigger;
};
} /* namespace Nrf::Matter */
