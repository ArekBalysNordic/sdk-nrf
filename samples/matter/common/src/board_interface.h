/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "board_config.h"
#include "board_consts.h"
#include "led_util.h"
#include "led_widget.h"
#include "system_event.h"

#include <app/clusters/identify-server/identify-server.h>

enum class DeviceState : uint8_t { kDeviceDisconnected, kDeviceConnectedBLE, kDeviceProvisioned };
enum class DeviceLeds : uint8_t { kAppLED, kUserLED1, kUserLED2 };
enum class DeviceButtons : uint8_t { kAppButton, kUserButton1, kUserButton2 };
enum class ButtonActions : uint8_t { kButtonPressed, kButtonReleased };

using ButtonHandler = void (*)(DeviceButtons source, ButtonActions action);

class BoardInterface {
	using LedState = bool;

public:
	bool Init(ButtonHandler buttonCallback = nullptr);

	/* Identify */
	static void IdentifyStartHandler(Identify *);
	static void IdentifyStopHandler(Identify *);

	/* LEDs */
	LEDWidget &GetLED(DeviceLeds led);
	void UpdateDeviceState(DeviceState state);

private:
	BoardInterface() {}
	friend BoardInterface &GetBoardInterface();
	static BoardInterface sInstance;

	/* Identify */
	void IdentifyStop(bool currentState = sInstance.mStateBeforeIdentify) { mApplicationLED.Set(currentState); }
	void IdentifyStart()
	{
		mStateBeforeIdentify = mApplicationLED.GetState();
		mApplicationLED.Blink(LedConsts::kIdentifyBlinkRate_ms);
	}

	/* LEDs */
	void UpdateStatusLED();
	static void LEDStateUpdateHandler(LEDWidget &ledWidget);
	static void UpdateLedStateEventHandler(const void *ctx);
	void ResetAllLeds();

	LEDWidget mStatusLED;
	LEDWidget mApplicationLED;
	LedState mStateBeforeIdentify;
	k_timer mFunctionTimer;
	DeviceState mState = DeviceState::kDeviceDisconnected;
#if NUMBER_OF_LEDS == 4
	LEDWidget mUserLED1;
	LEDWidget mUserLED2;
#endif

	/* Function Timer */
	void CancelTimer();
	void StartTimer(uint32_t timeoutInMs);
	static void FunctionTimerTimeoutCallback(k_timer *timer);
	static void FunctionTimerEventHandler(const void *ctx);

	bool mFunctionTimerActive = false;
	SystemEventType mFunction;

	/* Buttons */
	static void ButtonEventHandler(uint32_t buttonState, uint32_t hasChanged);
	static void FunctionHandler(const void *ctx);
	static void StartBLEAdvertisementHandler(const void *ctx);
	static void StartBLEAdvertisement();

	ButtonHandler mButtonCallback = nullptr;

};

inline BoardInterface &GetBoardInterface()
{
	return BoardInterface::sInstance;
}
