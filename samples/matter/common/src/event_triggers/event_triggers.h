/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerDelegate.h>

#include <cstdint>

namespace Nrf::Matter
{
class TestEventTrigger : public chip::TestEventTriggerDelegate {
public:
	enum EventTriggerMask : uint64_t {
		kSystem = 0xF000000000000000,
		kDiagnostics = 0x1000000000000000,
		kMatterStack = 0x0100000000000000,
		kDoorLock = 0x3277400000000000,
	};

	enum EventTriggers : uint64_t {
		/* System */
		kFactoryReset = EventTriggerMask::kSystem,
		kReboot = kFactoryReset + 1,
		/* Diagnostic logs cluster */
		kDiagnosticLogsCrash = EventTriggerMask::kDiagnostics,
		/* Door lock cluster */
		kDoorLockJammed = EventTriggerMask::kDoorLock,
		/* OTA */
		kOtaStart = chip::OTATestEventTriggerDelegate::kOtaQueryTrigger,
	};

	CHIP_ERROR SetEnableKey(const chip::ByteSpan &newEnableKey)
	{
		VerifyOrReturnError(newEnableKey.size() == chip::TestEventTriggerDelegate::kEnableKeyLength,
				    CHIP_ERROR_INVALID_ARGUMENT);
		return CopySpanToMutableSpan(newEnableKey, mEnableKey);
	}

	explicit TestEventTrigger() : mOtaTestEventTrigger(mEnableKey) {}

	/* TestEventTriggerDelegate implementation */
	bool DoesEnableKeyMatch(const chip::ByteSpan &enableKey) const override;
	CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;

private:
	uint8_t mEnableKeyData[chip::TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
										     0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
										     0xcc, 0xdd, 0xee, 0xff };
	chip::MutableByteSpan mEnableKey{ mEnableKeyData };
	chip::OTATestEventTriggerDelegate mOtaTestEventTrigger;
};
} /* namespace Nrf::Matter */
