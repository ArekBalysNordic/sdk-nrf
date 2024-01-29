/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "board/board.h"
#include "board/led_widget.h"
#include "bolt_lock_manager.h"

struct k_timer;
struct Identify;

#ifdef CONFIG_THREAD_WIFI_SWITCHING
enum class SwitchButtonAction : uint8_t { Pressed, Released };
#endif

class AppTask {
public:
	static AppTask &Instance()
	{
		static AppTask sAppTask;
		return sAppTask;
	};

	CHIP_ERROR StartApp();

	void UpdateClusterState(BoltLockManager::State state, BoltLockManager::OperationSource source);
	static void IdentifyStartHandler(Identify *);
	static void IdentifyStopHandler(Identify *);

private:
	CHIP_ERROR Init();

	static void LockActionEventHandler();
	static void ButtonEventHandler(Nrf::ButtonState state, Nrf::ButtonMask hasChanged);
	static void LockStateChanged(BoltLockManager::State state, BoltLockManager::OperationSource source);

#ifdef CONFIG_THREAD_WIFI_SWITCHING
	static void SwitchTransportEventHandler();
	static void SwitchTransportTimerTimeoutCallback(k_timer *timer);
	static void SwitchTransportTriggerHandler(const SwitchButtonAction &action);
#endif

#ifdef CONFIG_CHIP_ICD_UAT_SUPPORT
	static void IcdUatEventHandler();
#endif

#ifdef CONFIG_CHIP_NUS
	static void NUSLockCallback(void *context);
	static void NUSUnlockCallback(void *context);
#endif
};
