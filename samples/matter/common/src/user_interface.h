/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once
#include "board_util.h"
#include "led_util.h"
#include "led_widget.h"

#include <dk_buttons_and_leds.h>

namespace FactoryResetConsts
{
constexpr uint32_t kFactoryResetTriggerTimeout = 3000;
constexpr uint32_t kFactoryResetCancelWindowTimeout = 3000;
} /* namespace FactoryResetConsts */

namespace AdvertisingConsts
{
#if NUMBER_OF_BUTTONS == 2
constexpr uint32_t kAdvertisingTriggerTimeout = 3000;
#endif
} // namespace AdvertisingConsts

namespace LedConsts
{
constexpr uint32_t kBlinkRate_ms{ 500 };
constexpr uint32_t kIdentifyBlinkRate_ms{ 500 };
namespace StatusLed
{
	namespace BleConnected
	{
		constexpr uint32_t kOn_ms{ 100 };
		constexpr uint32_t kOff_ms{ kOn_ms };
	} /* namespace BleConnected */
	namespace Disconnected
	{
		constexpr uint32_t kOn_ms{ 50 };
		constexpr uint32_t kOff_ms{ 950 };
	} /* namespace Disconnected */

} /* namespace StatusLed */
} /* namespace LedConsts */

class UserInterface {
public:
	enum class DeviceState : uint8_t { kDeviceDisconnected, kDeviceConnectedBLE, kDeviceProvisioned };

	static UserInterface &Instance()
	{
		static UserInterface userInterface;
		return userInterface;
	}

	bool Init();
	LEDWidget &FunctionalLed() { return mFunctionalLED; }
	void ChangeDeviceState(DeviceState newState)
	{
		mState = newState;
		UpdateStatusLED();
	}

private:
	UserInterface() {}

	static void FunctionTimerTimeoutCallback(k_timer *timer);
	static void ButtonEventHandler(uint32_t buttonState, uint32_t hasChanged);
	static void LEDStateUpdateHandler(LEDWidget &ledWidget);
	void UpdateStatusLED();

	/* Leds */
	LEDWidget mStatusLED;
	LEDWidget mFunctionalLED;
	k_timer mFunctionTimer;
	DeviceState mState = DeviceState::kDeviceDisconnected;
#if NUMBER_OF_LEDS == 4
	FactoryResetLEDsWrapper<2> mFactoryResetLEDs{ { DK_LED3, DK_LED4 } };
#endif
};
