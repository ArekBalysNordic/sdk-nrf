/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once
#include "app_config.h"
#include "led_util.h"
#include "led_widget.h"

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
	using LedState = bool;

public:
	enum class DeviceState : uint8_t { kDeviceDisconnected, kDeviceConnectedBLE, kDeviceProvisioned };

	bool Init();

	/* LEDS */
	LEDWidget &GetApplicationLed() { return mApplicationLED; }

	void Identify()
	{
		mStateBeforeIdentify = mApplicationLED.GetState();
		mApplicationLED.Blink(LedConsts::kIdentifyBlinkRate_ms);
	}

	void IdentifyStop(bool currentState = Instance().mStateBeforeIdentify) { mApplicationLED.Set(currentState); }

	void ChangeDeviceState(DeviceState newState)
	{
		mState = newState;
		UpdateStatusLED();
	}

private:
	UserInterface() {}
	friend UserInterface &GetUserInterface();

	void UpdateStatusLED();

	/* Leds */
	LEDWidget mStatusLED;
	LEDWidget mApplicationLED;
	LedState mStateBeforeIdentify;
	k_timer mFunctionTimer;
	DeviceState mState = DeviceState::kDeviceDisconnected;
#if NUMBER_OF_LEDS == 4
	FactoryResetLEDsWrapper<2> mFactoryResetLEDs{ { DK_LED3, DK_LED4 } };
#endif

	UserInterface sInstance;
};

inline UserInterface &GetUserInterface()
{
	return UserInterface::sInstance;
}