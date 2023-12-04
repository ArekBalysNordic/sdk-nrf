/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "board_interface.h"
#include "event_manager.h"

#ifdef CONFIG_MCUMGR_TRANSPORT_BT
#include "dfu_over_smp.h"
#endif

#include <app/server/Server.h>
#include <platform/CHIPDeviceLayer.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

BoardInterface BoardInterface::sInstance;

Identify sIdentify = { IDENTIFY_ENDPOINT, BoardInterface::IdentifyStartHandler, BoardInterface::IdentifyStopHandler,
		       chip::app::Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator };

bool BoardInterface::Init(ButtonHandler buttonCallback)
{
	mButtonCallback = buttonCallback;

	/* Initialize LEDs */
	LEDWidget::InitGpio();
	LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);
	mStatusLED.Init(SYSTEM_STATE_LED);
	mApplicationLED.Init(APPLICATION_STATE_LED);
#if NUMBER_OF_LEDS == 4
	mUserLED1.Init(USER_LED_1);
	mUserLED2.Init(USER_LED_2);
#endif

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

void BoardInterface::UpdateDeviceState(DeviceState state)
{
	mState = state;
	UpdateStatusLED();
}

void BoardInterface::ResetAllLeds()
{
	sInstance.mStatusLED.Set(false);
	sInstance.mApplicationLED.Set(false);
#if NUMBER_OF_LEDS == 4
	sInstance.mUserLED1.Set(false);
	sInstance.mUserLED2.Set(false);
#endif
}

void BoardInterface::LEDStateUpdateHandler(LEDWidget &ledWidget)
{
	CHIP_ERROR err = chip::DeviceLayer::PlatformMgr().ScheduleWork(
		[](intptr_t context) {
			auto ctx = reinterpret_cast<LEDWidget *>(context);
			SystemEvent ledEvent(SystemEventType::UpdateLedState);
			ledEvent.mHandler = UpdateLedStateEventHandler;
			ledEvent.UpdateLedStateEvent.LedWidget = ctx;
			EventManager::PostEvent(ledEvent);
		},
		reinterpret_cast<intptr_t>(&ledWidget));
}

void BoardInterface::UpdateLedStateEventHandler(const void *context)
{
	if (!context) {
		return;
	}
	SystemEvent event(context);

	if (static_cast<SystemEventType>(event.mType) == SystemEventType::UpdateLedState) {
		event.UpdateLedStateEvent.LedWidget->UpdateState();
	}
}

void BoardInterface::UpdateStatusLED()
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

LEDWidget &BoardInterface::GetLED(DeviceLeds led)
{
	switch (led) {
#if NUMBER_OF_LEDS == 4
	case DeviceLeds::kUserLED1:
		return mUserLED1;
	case DeviceLeds::kUserLED2:
		return mUserLED2;
#endif
	case DeviceLeds::kAppLED:
	default:
		return mApplicationLED;
	}
}

void BoardInterface::IdentifyStartHandler(Identify *)
{
	SystemEvent event(SystemEventType::IdentifyStart,
			  [](const void *) { sInstance.mStatusLED.Blink(LedConsts::kIdentifyBlinkRate_ms); });
	EventManager::PostEvent(event);
}

void BoardInterface::IdentifyStopHandler(Identify *)
{
	SystemEvent event(SystemEventType::IdentifyStop,
			  [](const void *) { sInstance.mStatusLED.Set(sInstance.mStateBeforeIdentify); });
	EventManager::PostEvent(event);
}

void BoardInterface::CancelTimer()
{
	k_timer_stop(&mFunctionTimer);
	mFunctionTimerActive = false;
}

void BoardInterface::StartTimer(uint32_t timeoutInMs)
{
	k_timer_start(&mFunctionTimer, K_MSEC(timeoutInMs), K_NO_WAIT);
	mFunctionTimerActive = true;
}

void BoardInterface::FunctionTimerTimeoutCallback(k_timer *timer)
{
	if (!timer) {
		return;
	}

	chip::DeviceLayer::PlatformMgr().ScheduleWork(
		[](intptr_t context) {
			auto ctx = reinterpret_cast<k_timer *>(context);
			SystemEvent timerEvent(SystemEventType::Timer, FunctionTimerEventHandler);
			timerEvent.TimerEvent.Timer = ctx;
			EventManager::PostEvent(timerEvent);
		},
		reinterpret_cast<intptr_t>(timer));
}

void BoardInterface::FunctionTimerEventHandler(const void *context)
{
	if (!context) {
		return;
	}
	SystemEvent event(context);

	/* If we reached here, the button was held past kFactoryResetTriggerTimeout, initiate factory reset */
	if (sInstance.mFunction == SystemEventType::SoftwareUpdate) {
		LOG_INF("Factory reset has been triggered. Release button within %ums to cancel.",
			FactoryResetConsts::kFactoryResetTriggerTimeout);

		/* Start timer for kFactoryResetCancelWindowTimeout to allow user to cancel, if required. */
		sInstance.StartTimer(FactoryResetConsts::kFactoryResetCancelWindowTimeout);
		sInstance.mFunction = SystemEventType::FactoryReset;

		/* Turn off all LEDs before starting blink to make sure blink is coordinated. */
		sInstance.ResetAllLeds();

		sInstance.mStatusLED.Blink(LedConsts::kBlinkRate_ms);
		sInstance.mApplicationLED.Blink(LedConsts::kBlinkRate_ms);
#if NUMBER_OF_LEDS == 4
		sInstance.mUserLED1.Blink(LedConsts::kBlinkRate_ms);
		sInstance.mUserLED2.Blink(LedConsts::kBlinkRate_ms);
#endif
	} else if (sInstance.mFunction == SystemEventType::FactoryReset) {
		/* Actually trigger Factory Reset */
		sInstance.mFunction = SystemEventType::None;
		chip::Server::GetInstance().ScheduleFactoryReset();

	} else if (sInstance.mFunction == SystemEventType::AdvertisingStart) {
		/* The button was held past kAdvertisingTriggerTimeout, start BLE advertisement
		   if we have 2 buttons UI*/
#if NUMBER_OF_BUTTONS == 2
		StartBLEAdvertisement();
		sInstance.mFunction = SystemEventType::None;
#endif
	}
}

void BoardInterface::ButtonEventHandler(uint32_t buttonState, uint32_t hasChanged)
{
	SystemEvent buttonEvent;
	bool isAppButtonEvent = false;
	ButtonActions action;
	DeviceButtons source;

	buttonEvent.mType = static_cast<uint8_t>(SystemEventType::Button);

	if (BLUETOOTH_ADV_BUTTON_MASK & hasChanged) {
		buttonEvent.ButtonEvent.PinNo = BLUETOOTH_ADV_BUTTON;
		buttonEvent.ButtonEvent.Action = static_cast<uint8_t>((BLUETOOTH_ADV_BUTTON_MASK & buttonState) ?
									      SystemEventType::ButtonPushed :
									      SystemEventType::ButtonReleased);
		buttonEvent.mHandler = StartBLEAdvertisementHandler;
		EventManager::PostEvent(buttonEvent);
	}

	if (FUNCTION_BUTTON_MASK & hasChanged) {
		buttonEvent.ButtonEvent.PinNo = FUNCTION_BUTTON;
		buttonEvent.ButtonEvent.Action =
			static_cast<uint8_t>((FUNCTION_BUTTON_MASK & buttonState) ? SystemEventType::ButtonPushed :
										    SystemEventType::ButtonReleased);
		buttonEvent.mHandler = FunctionHandler;
		EventManager::PostEvent(buttonEvent);
	}

#if NUMBER_OF_BUTTONS == 4
	if (APPLICATION_BUTTON_MASK & hasChanged) {
		source = DeviceButtons::kAppButton;
		action = (APPLICATION_BUTTON_MASK & buttonState) ? ButtonActions::kButtonPressed :
								   ButtonActions::kButtonReleased;
		isAppButtonEvent = true;
	}

	if (USER_BUTTON_1_MASK & hasChanged) {
		source = DeviceButtons::kUserButton1;
		action = (USER_BUTTON_1_MASK & buttonState) ? ButtonActions::kButtonPressed :
							      ButtonActions::kButtonReleased;
		isAppButtonEvent = true;
	}
	if (USER_BUTTON_2_MASK & hasChanged) {
		source = DeviceButtons::kUserButton2;
		action = (USER_BUTTON_2_MASK & buttonState) ? ButtonActions::kButtonPressed :
							      ButtonActions::kButtonReleased;
		isAppButtonEvent = true;
	}
#endif

	if (isAppButtonEvent && sInstance.mButtonCallback) {
		sInstance.mButtonCallback(source, action);
	}
}

void BoardInterface::FunctionHandler(const void *context)
{
	if (!context) {
		return;
	}

	SystemEvent event(context);

	if (event.ButtonEvent.PinNo != FUNCTION_BUTTON) {
		return;
	}

	if (event.ButtonEvent.Action == static_cast<uint8_t>(SystemEventType::ButtonPushed)) {
		if (!sInstance.mFunctionTimerActive && sInstance.mFunction == SystemEventType::None) {
			sInstance.mFunction = SystemEventType::SoftwareUpdate;
			sInstance.StartTimer(FactoryResetConsts::kFactoryResetTriggerTimeout);
		}
	} else {
		/* If the button was released before factory reset got initiated, trigger a software update. */
		if (sInstance.mFunctionTimerActive && sInstance.mFunction == SystemEventType::SoftwareUpdate) {
			sInstance.CancelTimer();
			sInstance.mFunction = SystemEventType::None;

#ifdef CONFIG_MCUMGR_TRANSPORT_BT
			GetDFUOverSMP().StartServer();
#else
			LOG_INF("Software update is disabled");
#endif
		} else if (sInstance.mFunctionTimerActive && sInstance.mFunction == SystemEventType::FactoryReset) {
			sInstance.ResetAllLeds();
			sInstance.CancelTimer();
			sInstance.UpdateStatusLED();
			sInstance.mFunction = SystemEventType::None;
			LOG_INF("Factory reset has been canceled");
		}
	}
}

void BoardInterface::StartBLEAdvertisementHandler(const void *context)
{
	if (!context) {
		return;
	}

	SystemEvent event(context);

#if NUMBER_OF_BUTTONS == 4
	if (event.ButtonEvent.Action == static_cast<uint8_t>(SystemEventType::ButtonPushed)) {
		StartBLEAdvertisement();
	}

#elif NUMBER_OF_BUTTONS == 2
	if (event.ButtonEvent.Action == static_cast<uint8_t>(SystemEventType::ButtonPushed)) {
		sInstance.StartTimer(AdvertisingConsts::kAdvertisingTriggerTimeout);
		sInstance.mFunction = SystemEventType::AdvertisingStart;
	} else {
		if (sInstance.mFunction == SystemEventType::AdvertisingStart && sInstance.mFunctionTimerActive) {
			sInstance.CancelTimer();
			sInstance.mFunction = SystemEventType::None;

			if (sInstance.mButtonCallback) {
				sInstance.mButtonCallback(DeviceButtons::kAppButton, ButtonActions::kButtonPressed);
			}
		}
	}
#endif
}

void BoardInterface::StartBLEAdvertisement()
{
	if (chip::Server::GetInstance().GetFabricTable().FabricCount() != 0) {
		LOG_INF("Matter service BLE advertising not started - device is already commissioned");
		return;
	}

	if (chip::DeviceLayer::ConnectivityMgr().IsBLEAdvertisingEnabled()) {
		LOG_INF("BLE advertising is already enabled");
		return;
	}

	if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() !=
	    CHIP_NO_ERROR) {
		LOG_ERR("OpenBasicCommissioningWindow() failed");
	}
}
