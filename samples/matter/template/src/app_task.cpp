/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "app_task.h"
#include "led_widget.h"
#include "thread_util.h"

#include <platform/CHIPDeviceLayer.h>

#include <access/AccessControl.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemError.h>

#ifdef CONFIG_CHIP_OTA_REQUESTOR
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/clusters/ota-requestor/GenericOTARequestorDriver.h>
#include <app/clusters/ota-requestor/OTARequestor.h>
#include <platform/nrfconnect/OTAImageProcessorImpl.h>
#endif

#include <dk_buttons_and_leds.h>
#include <logging/log.h>
#include <settings/settings.h>
#include <zephyr.h>

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

LOG_MODULE_DECLARE(app, CONFIG_MATTER_LOG_LEVEL);

namespace
{
static constexpr size_t kAppEventQueueSize = 10;
static constexpr uint32_t kFactoryResetTriggerTimeout = 6000;

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));
LEDWidget sStatusLED;
LEDWidget sUnusedLED;
LEDWidget sUnusedLED_1;
LEDWidget sUnusedLED_2;

bool sIsThreadProvisioned;
bool sIsThreadEnabled;
bool sHaveBLEConnections;

k_timer sFunctionTimer;

#ifdef CONFIG_CHIP_OTA_REQUESTOR
DefaultOTARequestorStorage sRequestorStorage;
GenericOTARequestorDriver sOTARequestorDriver;
OTAImageProcessorImpl sOTAImageProcessor;
chip::BDXDownloader sBDXDownloader;
chip::OTARequestor sOTARequestor;
#endif
} /* namespace */

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init()
{
	/* Initialize CHIP stack */
	LOG_INF("Init CHIP stack");

	CHIP_ERROR err = chip::Platform::MemoryInit();
	if (err != CHIP_NO_ERROR) {
		LOG_ERR("Platform::MemoryInit() failed");
		return err;
	}

	err = PlatformMgr().InitChipStack();
	if (err != CHIP_NO_ERROR) {
		LOG_ERR("PlatformMgr().InitChipStack() failed");
		return err;
	}

	err = ThreadStackMgr().InitThreadStack();
	if (err != CHIP_NO_ERROR) {
		LOG_ERR("ThreadStackMgr().InitThreadStack() failed");
		return err;
	}

#ifdef CONFIG_OPENTHREAD_MTD_SED
	err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
#elif CONFIG_OPENTHREAD_MTD
	err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#else
	err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_FullEndDevice);
#endif
	if (err != CHIP_NO_ERROR) {
		LOG_ERR("ConnectivityMgr().SetThreadDeviceType() failed");
		return err;
	}

	/* Initialize LEDs */
	LEDWidget::InitGpio();
	LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);

	sStatusLED.Init(DK_LED1);
	sUnusedLED.Init(DK_LED2);
	sUnusedLED_1.Init(DK_LED3);
	sUnusedLED_2.Init(DK_LED4);

	UpdateStatusLED();

	/* Initialize buttons */
	int ret = dk_buttons_init(ButtonEventHandler);
	if (ret) {
		LOG_ERR("dk_buttons_init() failed");
		return chip::System::MapErrorZephyr(ret);
	}

	/* Initialize function timer */
	k_timer_init(&sFunctionTimer, &AppTask::TimerEventHandler, nullptr);
	k_timer_user_data_set(&sFunctionTimer, this);

	/* Initialize CHIP server */
	SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

#ifdef CONFIG_CHIP_OTA_REQUESTOR
	sOTAImageProcessor.SetOTADownloader(&sBDXDownloader);
	sBDXDownloader.SetImageProcessorDelegate(&sOTAImageProcessor);
	sRequestorStorage.Init(Server::GetInstance().GetPersistentStorage());
	sOTARequestor.Init(Server::GetInstance(), sRequestorStorage, sOTARequestorDriver, sBDXDownloader);
	sOTARequestorDriver.Init(&sOTARequestor, &sOTAImageProcessor);
	chip::SetRequestorInstance(&sOTARequestor);
#endif

	ReturnErrorOnFailure(chip::Server::GetInstance().Init());
	ConfigurationMgr().LogDeviceConfig();
	PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

	/*
	 * Add CHIP event handler and start CHIP thread.
	 * Note that all the initialization code should happen prior to this point
	 * to avoid data races between the main and the CHIP threads.
	 */
	PlatformMgr().AddEventHandler(ChipEventHandler, 0);

	err = PlatformMgr().StartEventLoopTask();
	if (err != CHIP_NO_ERROR) {
		LOG_ERR("PlatformMgr().StartEventLoopTask() failed");
		return err;
	}

	if (!ConnectivityMgr().IsThreadProvisioned()) {
		StartDefaultThreadNetwork();
		LOG_INF("Device is not commissioned to a Thread network. Starting with the default configuration");
	} else {
		LOG_INF("Device is commissioned to a Thread network");
	}

	uint8_t ACL_0[] = { 0x15, 0x24, 0x01, 0x01, 0x18 };
	settings_save_one("mt/acl", ACL_0, sizeof(ACL_0));

	uint8_t ACL_N[] = { 0x15, 0x28, 0x01, 0x24, 0x02, 0x00, 0x24, 0x03, 0x20, 0x24, 0x04, 0x01, 0x36, 0x05, 0x04,
			    0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x18, 0x36, 0x06, 0x06, 0xff, 0xff, 0xff, 0xff,
			    0x06, 0xff, 0xff, 0xff, 0xff, 0x06, 0xff, 0xff, 0xff, 0xff, 0x06, 0xff, 0xff, 0xff, 0xff,
			    0x06, 0xff, 0xff, 0xff, 0xff, 0x06, 0xff, 0xff, 0xff, 0xff, 0x18, 0x18 };
	settings_save_one("mt/acl/0", ACL_N, sizeof(ACL_N));
	settings_save_one("mt/acl/1", ACL_N, sizeof(ACL_N));
	settings_save_one("mt/acl/2", ACL_N, sizeof(ACL_N));
	settings_save_one("mt/acl/3", ACL_N, sizeof(ACL_N));
	settings_save_one("mt/acl/4", ACL_N, sizeof(ACL_N));
	settings_save_one("mt/acl/5", ACL_N, sizeof(ACL_N));
	settings_save_one("mt/acl/6", ACL_N, sizeof(ACL_N));
	settings_save_one("mt/acl/7", ACL_N, sizeof(ACL_N));
	settings_save_one("mt/acl/8", ACL_N, sizeof(ACL_N));
	settings_save_one("mt/acl/9", ACL_N, sizeof(ACL_N));
	settings_save_one("mt/acl/a", ACL_N, sizeof(ACL_N));
	settings_save_one("mt/acl/b", ACL_N, sizeof(ACL_N));
	settings_save_one("mt/acl/c", ACL_N, sizeof(ACL_N));
	settings_save_one("mt/acl/d", ACL_N, sizeof(ACL_N));
	settings_save_one("mt/acl/e", ACL_N, sizeof(ACL_N));

	uint8_t ot_c[] = { 0xfd, 0x78, 0x42, 0xc6, 0x82, 0x65, 0x74, 0x11, 0xeb,
			   0xf2, 0xe2, 0x4c, 0x44, 0x22, 0xde, 0x6f, 0x20, 0xd1 };
	settings_save_one("ot/c/53067346", ot_c, sizeof(ot_c));
	uint8_t ot_b[] = { 0x30, 0x77, 0x02, 0x01, 0x01, 0x04, 0x20, 0x93, 0xf5, 0xd7, 0xaa, 0x43, 0x73, 0x18,
			   0x4a, 0xd0, 0x45, 0xab, 0xb4, 0x6b, 0x2d, 0xaa, 0x70, 0x25, 0x55, 0x1c, 0xfd, 0x2a,
			   0x0d, 0xcb, 0x0c, 0xd3, 0x84, 0x3b, 0x35, 0x0e, 0x7f, 0x08, 0x61, 0xa0, 0x0a, 0x06,
			   0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0xa1, 0x44, 0x03, 0x42, 0x00,
			   0x04, 0xea, 0x7f, 0x2f, 0x05, 0x2b, 0x0a, 0xaf, 0x5c, 0x0f, 0x0f, 0x1b, 0xfe, 0x04,
			   0xc2, 0x23, 0x71, 0xab, 0x5f, 0x8f, 0xe0, 0x28, 0x82, 0xf0, 0x96, 0x99, 0xbb, 0x8f,
			   0x9b, 0xa9, 0x04, 0x03, 0xae, 0x89, 0x09, 0xe8, 0xa0, 0xc7, 0x94, 0x6d, 0xa0, 0xdf,
			   0x79, 0x95, 0xc7, 0x99, 0x13, 0x5e, 0x76, 0x9e, 0xfe, 0x57, 0x57, 0x16, 0x29, 0xa8,
			   0x83, 0xab, 0xf0, 0xa4, 0x5b, 0xdb, 0x51, 0x97, 0x56, 0xdb, 0x51, 0x97, 0x56, 0x79 };
	settings_save_one("ot/b/9556b3d3", ot_b, sizeof(ot_b));
	uint8_t ot_7[] = { 0x35, 0x90, 0x66, 0xe1, 0x29, 0x5c, 0x33, 0x6f, 0x99, 0xed, 0x78,
			   0x1b, 0x30, 0x9f, 0x35, 0xb0, 0x32, 0x24, 0x7a, 0xe2, 0xf5, 0xef,
			   0x16, 0x87, 0x2a, 0x8d, 0xc6, 0x0a, 0x84, 0x07, 0xd9, 0x20 };
	settings_save_one("ot/7/b2889d94", ot_7, sizeof(ot_7));
	uint8_t ot_3[] = { 0x02, 0x0c, 0x02, 0xb0, 0x00, 0x00, 0x00, 0x00, 0xea, 0x03, 0x00, 0x00, 0xe8,
			   0x03, 0x00, 0x00, 0x22, 0x1e, 0x5e, 0x65, 0x22, 0x34, 0x84, 0xef, 0x6c, 0x6b,
			   0x30, 0xe7, 0x0f, 0xca, 0x69, 0x96, 0xf7, 0xe7, 0x1a, 0x83, 0x03, 0x00 };
	settings_save_one("ot/3/438a8691", ot_3, sizeof(ot_3));
	uint8_t ot_4[] = { 0xa6, 0x0a, 0x6d, 0x14, 0xb5, 0x69, 0x84, 0xc4, 0x03, 0x00 };
	settings_save_one("ot/4/57052541", ot_4, sizeof(ot_4));
	uint8_t ot_1[] = { 0x0e, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
			   0x12, 0x35, 0x06, 0x00, 0x04, 0x00, 0x1f, 0xff, 0xe0, 0x02, 0x08, 0x11, 0x11, 0x11,
			   0x11, 0x22, 0x22, 0x22, 0x22, 0x07, 0x08, 0xfd, 0x78, 0x42, 0xc6, 0x82, 0x65, 0x74,
			   0x11, 0x05, 0x10, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
			   0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x03, 0x04, 0x41, 0x72, 0x65, 0x6b, 0x01, 0x02, 0x12,
			   0x21, 0x04, 0x10, 0x14, 0x1a, 0xea, 0xd7, 0xa3, 0x18, 0x5e, 0xc0, 0xa8, 0x1c, 0x7e,
			   0x8c, 0x33, 0x68, 0xf7, 0x4e, 0x0c, 0x04, 0x02, 0xa0, 0xff, 0xf8 };
	settings_save_one("ot/1/bfff5b72", ot_1, sizeof(ot_1));
	uint8_t mt_f_1_g[] = { 0x15, 0x24, 0x01, 0x00, 0x24, 0x02, 0x00, 0x24, 0x03, 0x00, 0x24, 0x04,
			       0x00, 0x24, 0x05, 0x00, 0x24, 0x06, 0x01, 0x24, 0x07, 0x00, 0x18 };
	settings_save_one("mt/f/1/g", mt_f_1_g, sizeof(mt_f_1_g));
	uint8_t mt_f_t[] = {0x15, 0x24, 0x01, 0x01, 0x24, 0x02, 0x01, 0x18 };
	settings_save_one("mt/f/t", mt_f_t, sizeof(mt_f_t));
	uint8_t mt_f_1_k_0[] = { 0x15, 0x24, 0x01, 0x00, 0x24, 0x02, 0x01, 0x36, 0x03, 0x15, 0x27, 0x04, 0x01, 0x00,
				 0x00, 0x00, 0x37, 0xa2, 0x03, 0x00, 0x25, 0x05, 0x30, 0x43, 0x30, 0x06, 0x10, 0x40,
				 0x68, 0xd0, 0x65, 0x7c, 0x61, 0xb2, 0x65, 0x83, 0xf3, 0xf1, 0x1d, 0x76, 0x08, 0x55,
				 0xf4, 0x18, 0x15, 0x27, 0x04, 0x10, 0x00, 0x00, 0x00, 0xa8, 0xb2, 0x02, 0x20, 0x24,
				 0x05, 0x00, 0x30, 0x06, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x15, 0x27, 0x04, 0xa8, 0xb2, 0x02,
				 0x20, 0xa0, 0xb2, 0x02, 0x20, 0x24, 0x05, 0x00, 0x30, 0x06, 0x10, 0x00, 0x00, 0x00,
				 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18,
				 0x18, 0x25, 0x07, 0xff, 0xff, 0x18 };
	settings_save_one("mt/f/1/k/0", mt_f_1_k_0, sizeof(mt_f_1_k_0));

	return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::StartApp()
{
	ReturnErrorOnFailure(Init());

	AppEvent event = {};

	while (true) {
		k_msgq_get(&sAppEventQueue, &event, K_FOREVER);
		DispatchEvent(event);
	}

	return CHIP_NO_ERROR;
}

void AppTask::PostEvent(const AppEvent &event)
{
	if (k_msgq_put(&sAppEventQueue, &event, K_NO_WAIT)) {
		LOG_INF("Failed to post event to app task event queue");
	}
}

void AppTask::DispatchEvent(const AppEvent &event)
{
	switch (event.Type) {
	case AppEvent::FunctionPress:
		FunctionPressHandler();
		break;
	case AppEvent::FunctionRelease:
		FunctionReleaseHandler();
		break;
	case AppEvent::FunctionTimer:
		FunctionTimerEventHandler();
		break;
	case AppEvent::UpdateLedState:
		event.UpdateLedStateEvent.LedWidget->UpdateState();
		break;
	default:
		LOG_INF("Unknown event received");
		break;
	}
}

void AppTask::FunctionPressHandler()
{
	sAppTask.StartFunctionTimer(kFactoryResetTriggerTimeout);
	sAppTask.mFunction = TimerFunction::FactoryReset;
}

void AppTask::FunctionReleaseHandler()
{
	if (sAppTask.mFunction == TimerFunction::FactoryReset) {
		sUnusedLED_2.Set(false);
		sUnusedLED_1.Set(false);
		sUnusedLED.Set(false);

		UpdateStatusLED();

		sAppTask.CancelFunctionTimer();
		sAppTask.mFunction = TimerFunction::NoneSelected;
		LOG_INF("Factory Reset has been Canceled");
	}
}

void AppTask::FunctionTimerEventHandler()
{
	if (sAppTask.mFunction == TimerFunction::FactoryReset) {
		sAppTask.mFunction = TimerFunction::NoneSelected;
		LOG_INF("Factory Reset triggered");

		sStatusLED.Set(true);
		sUnusedLED.Set(true);
		sUnusedLED_1.Set(true);
		sUnusedLED_2.Set(true);

		chip::Server::GetInstance().ScheduleFactoryReset();
	}
}

void AppTask::LEDStateUpdateHandler(LEDWidget &ledWidget)
{
	sAppTask.PostEvent(AppEvent{ AppEvent::UpdateLedState, &ledWidget });
}

void AppTask::UpdateStatusLED()
{
	/* Update the status LED.
	 *
	 * If thread and service provisioned, keep the LED On constantly.
	 *
	 * If the system has ble connection(s) uptill the stage above, THEN blink the LED at an even
	 * rate of 100ms.
	 *
	 * Otherwise, blink the LED On for a very short time. */
	if (sIsThreadProvisioned && sIsThreadEnabled) {
		sStatusLED.Set(true);
	} else if (sHaveBLEConnections) {
		sStatusLED.Blink(100, 100);
	} else {
		sStatusLED.Blink(50, 950);
	}
}

void AppTask::ChipEventHandler(const ChipDeviceEvent *event, intptr_t /* arg */)
{
	switch (event->Type) {
	case DeviceEventType::kCHIPoBLEAdvertisingChange:
		sHaveBLEConnections = ConnectivityMgr().NumBLEConnections() != 0;
		UpdateStatusLED();
		break;
	case DeviceEventType::kThreadStateChange:
		sIsThreadProvisioned = ConnectivityMgr().IsThreadProvisioned();
		sIsThreadEnabled = ConnectivityMgr().IsThreadEnabled();
		UpdateStatusLED();
		break;
	default:
		break;
	}
}

void AppTask::ButtonEventHandler(uint32_t buttonState, uint32_t hasChanged)
{
	if (DK_BTN1_MSK & buttonState & hasChanged) {
		GetAppTask().PostEvent(AppEvent{ AppEvent::FunctionPress });
	} else if (DK_BTN1_MSK & hasChanged) {
		GetAppTask().PostEvent(AppEvent{ AppEvent::FunctionRelease });
	}
}

void AppTask::CancelFunctionTimer()
{
	k_timer_stop(&sFunctionTimer);
}

void AppTask::StartFunctionTimer(uint32_t timeoutInMs)
{
	k_timer_start(&sFunctionTimer, K_MSEC(timeoutInMs), K_NO_WAIT);
}

void AppTask::TimerEventHandler(k_timer *timer)
{
	GetAppTask().PostEvent(AppEvent{ AppEvent::FunctionTimer });
}
