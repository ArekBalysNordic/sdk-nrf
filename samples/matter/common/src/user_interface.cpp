/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "user_interface.h"

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

UserInterface UserInterface::sInstance;

bool UserInterface::Init()
{
	/* Initialize LEDs */
	LEDWidget::InitGpio();
	LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);
	Instance().mStatusLED.Init(SYSTEM_STATE_LED);
	Instance().mApplicationLED.Init(APPLICATION_STATE_LED);

	/* Initialize buttons */
	int ret = dk_buttons_init(ButtonEventHandler);
	if (ret) {
		LOG_ERR("dk_buttons_init() failed");
		return false;
	}

	/* Initialize function timer */
	k_timer_init(&mFunctionTimer, &FunctionTimerTimeoutCallback, nullptr);
	k_timer_user_data_set(&mFunctionTimer, this);

	UpdateStatusLED();

	return true;
}

void UserInterface::LEDStateUpdateHandler(LEDWidget &ledWidget)
{
	ledWidget.UpdateState();
}

void UserInterface::FunctionTimerTimeoutCallback(k_timer *timer) {}

void UserInterface::UpdateStatusLED()
{
	/* Update the status LED.
	 *
	 * If IPv6 network and service provisioned, keep the LED On constantly.
	 *
	 * If the system has BLE connection(s) uptill the stage above, THEN blink the LED at an even
	 * rate of 100ms.
	 *
	 * Otherwise, blink the LED for a very short time. */
	switch (mState) {
	case DeviceState::kDeviceDisconnected:
		mStatusLED.Blink(LedConsts::StatusLed::Disconnected::kOn_ms,
				 LedConsts::StatusLed::Disconnected::kOff_ms);

		break;
	case DeviceState::kDeviceConnectedBLE:
		mStatusLED.Blink(LedConsts::StatusLed::BleConnected::kOn_ms,
				 LedConsts::StatusLed::BleConnected::kOff_ms);
		break;
	case DeviceState::kDeviceProvisioned:
		mStatusLED.Set(true);
		break;
	}
}

void UserInterface::ButtonEventHandler(uint32_t buttonState, uint32_t hasChanged)
{
	// 	AppEvent button_event;
	// 	button_event.Type = AppEventType::Button;

	// #if NUMBER_OF_BUTTONS == 2
	// 	if (BLE_ADVERTISEMENT_START_AND_LOCK_BUTTON_MASK & hasChanged) {
	// 		button_event.ButtonEvent.PinNo = BLE_ADVERTISEMENT_START_AND_LOCK_BUTTON;
	// 		button_event.ButtonEvent.Action = static_cast<uint8_t>(
	// 			(BLE_ADVERTISEMENT_START_AND_LOCK_BUTTON_MASK & buttonState) ?
	// AppEventType::ButtonPushed
	// : AppEventType::ButtonReleased); button_event.Handler = StartBLEAdvertisementAndLockActionEventHandler;
	// PostEvent(button_event);
	// 	}
	// #else
	// 	if (LOCK_BUTTON_MASK & buttonState & hasChanged) {
	// 		button_event.ButtonEvent.PinNo = LOCK_BUTTON;
	// 		button_event.ButtonEvent.Action = static_cast<uint8_t>(AppEventType::ButtonPushed);
	// 		button_event.Handler = LockActionEventHandler;
	// 		PostEvent(button_event);
	// 	}

	// #ifdef CONFIG_THREAD_WIFI_SWITCHING
	// 	if (THREAD_WIFI_SWITCH_BUTTON_MASK & hasChanged) {
	// 		button_event.ButtonEvent.PinNo = THREAD_WIFI_SWITCH_BUTTON;
	// 		button_event.ButtonEvent.Action = static_cast<uint8_t>((THREAD_WIFI_SWITCH_BUTTON_MASK &
	// buttonState) ? AppEventType::ButtonPushed : AppEventType::ButtonReleased); 		button_event.Handler =
	// SwitchImagesTriggerHandler; PostEvent(button_event);
	// 	}
	// #endif

	// 	if (BLE_ADVERTISEMENT_START_BUTTON_MASK & buttonState & hasChanged) {
	// 		button_event.ButtonEvent.PinNo = BLE_ADVERTISEMENT_START_BUTTON;
	// 		button_event.ButtonEvent.Action = static_cast<uint8_t>(AppEventType::ButtonPushed);
	// 		button_event.Handler = StartBLEAdvertisementHandler;
	// 		PostEvent(button_event);
	// 	}
	// #endif

	// 	if (FUNCTION_BUTTON_MASK & hasChanged) {
	// 		button_event.ButtonEvent.PinNo = FUNCTION_BUTTON;
	// 		button_event.ButtonEvent.Action =
	// 			static_cast<uint8_t>((FUNCTION_BUTTON_MASK & buttonState) ? AppEventType::ButtonPushed :
	// 										    AppEventType::ButtonReleased);
	// 		button_event.Handler = FunctionHandler;
	// 		PostEvent(button_event);
	// 	}
}

// void UserInterface::FunctionTimerEventHandler(const AppEvent &event)
// {
// 	if (event.Type != AppEventType::Timer) {
// 		return;
// 	}

// 	/* If we reached here, the button was held past kFactoryResetTriggerTimeout, initiate factory reset */
// 	if (Instance().mFunction == FunctionEvent::SoftwareUpdate) {
// 		LOG_INF("Factory Reset Triggered. Release button within %ums to cancel.", kFactoryResetTriggerTimeout);

// 		/* Start timer for kFactoryResetCancelWindowTimeout to allow user to cancel, if required. */
// 		Instance().StartTimer(kFactoryResetCancelWindowTimeout);
// 		Instance().mFunction = FunctionEvent::FactoryReset;

// 		/* Turn off all LEDs before starting blink to make sure blink is coordinated. */
// 		sStatusLED.Set(false);
// #if NUMBER_OF_LEDS == 4
// 		sFactoryResetLEDs.Set(false);
// #endif

// 		sStatusLED.Blink(LedConsts::kBlinkRate_ms);
// #if NUMBER_OF_LEDS == 4
// 		sFactoryResetLEDs.Blink(LedConsts::kBlinkRate_ms);
// #endif
// 	} else if (Instance().mFunction == FunctionEvent::FactoryReset) {
// 		/* Actually trigger Factory Reset */
// 		Instance().mFunction = FunctionEvent::NoneSelected;
// 		chip::Server::GetInstance().ScheduleFactoryReset();

// 	} else if (Instance().mFunction == FunctionEvent::AdvertisingStart) {
// 		/* The button was held past kAdvertisingTriggerTimeout, start BLE advertisement
// 		   if we have 2 buttons UI*/
// #if NUMBER_OF_BUTTONS == 2
// 		StartBLEAdvertisementHandler(event);
// 		Instance().mFunction = FunctionEvent::NoneSelected;
// #endif
// 	}
// }

// void UserInterface::FunctionHandler(const AppEvent &event)
// {
// 	if (event.ButtonEvent.PinNo != FUNCTION_BUTTON)
// 		return;

// 	/* To trigger software update: press the FUNCTION_BUTTON button briefly (< kFactoryResetTriggerTimeout)
// 	 * To initiate factory reset: press the FUNCTION_BUTTON for kFactoryResetTriggerTimeout +
// 	 * kFactoryResetCancelWindowTimeout All LEDs start blinking after kFactoryResetTriggerTimeout to signal factory
// 	 * reset has been initiated. To cancel factory reset: release the FUNCTION_BUTTON once all LEDs start blinking
// 	 * within the kFactoryResetCancelWindowTimeout.
// 	 */
// 	if (event.ButtonEvent.Action == static_cast<uint8_t>(AppEventType::ButtonPushed)) {
// 		if (!Instance().mFunctionTimerActive && Instance().mFunction == FunctionEvent::NoneSelected) {
// 			Instance().StartTimer(kFactoryResetTriggerTimeout);

// 			Instance().mFunction = FunctionEvent::SoftwareUpdate;
// 		}
// 	} else {
// 		/* If the button was released before factory reset got initiated, trigger a software update. */
// 		if (Instance().mFunctionTimerActive && Instance().mFunction == FunctionEvent::SoftwareUpdate) {
// 			Instance().CancelTimer();
// 			Instance().mFunction = FunctionEvent::NoneSelected;

// #ifdef CONFIG_MCUMGR_TRANSPORT_BT
// 			GetDFUOverSMP().StartServer();
// #else
// 			LOG_INF("Software update is disabled");
// #endif
// 		} else if (Instance().mFunctionTimerActive && Instance().mFunction == FunctionEvent::FactoryReset) {
// #if NUMBER_OF_LEDS == 4
// 			sFactoryResetLEDs.Set(false);
// #endif
// 			UpdateStatusLED();
// 			Instance().CancelTimer();
// 			Instance().mFunction = FunctionEvent::NoneSelected;
// 			LOG_INF("Factory Reset has been Canceled");
// 		}
// 	}
// }

// #if NUMBER_OF_BUTTONS == 2
// void AppTask::StartBLEAdvertisementAndLockActionEventHandler(const AppEvent &event)
// {
// 	if (event.ButtonEvent.Action == static_cast<uint8_t>(AppEventType::ButtonPushed)) {
// 		Instance().StartTimer(kAdvertisingTriggerTimeout);
// 		Instance().mFunction = FunctionEvent::AdvertisingStart;
// 	} else {
// 		if (Instance().mFunction == FunctionEvent::AdvertisingStart && Instance().mFunctionTimerActive) {
// 			Instance().CancelTimer();
// 			Instance().mFunction = FunctionEvent::NoneSelected;

// 			AppEvent button_event;
// 			button_event.Type = AppEventType::Button;
// 			button_event.ButtonEvent.PinNo = BLE_ADVERTISEMENT_START_AND_LOCK_BUTTON;
// 			button_event.ButtonEvent.Action = static_cast<uint8_t>(AppEventType::ButtonReleased);
// 			button_event.Handler = LockActionEventHandler;
// 			PostEvent(button_event);
// 		}
// 	}
// }
// #endif