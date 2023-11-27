/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "app_task.h"
#include "app_event.h"

#include "board_interface.h"
#include "bolt_lock_manager.h"
#include "fabric_table_delegate.h"

#ifdef CONFIG_THREAD_WIFI_SWITCHING
#include "software_images_swapper.h"
#endif

#ifdef CONFIG_THREAD_WIFI_SWITCHING_CLI_SUPPORT
#include <lib/shell/Engine.h>
using chip::Shell::Engine;
using chip::Shell::shell_command_t;
#endif

#ifdef CONFIG_CHIP_NUS
#include "bt_nus_service.h"
#endif

#include <platform/CHIPDeviceLayer.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemError.h>

#ifdef CONFIG_CHIP_WIFI
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/nrfconnect/wifi/NrfWiFiDriver.h>
#endif

#ifdef CONFIG_CHIP_OTA_REQUESTOR
#include "ota_util.h"
#endif

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#ifdef CONFIG_THREAD_WIFI_SWITCHING
#include <pm_config.h>
#endif

#include <app/InteractionModelEngine.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

namespace
{
constexpr EndpointId kLockEndpointId = 1;

#ifdef CONFIG_CHIP_NUS
constexpr uint16_t kAdvertisingIntervalMin = 400;
constexpr uint16_t kAdvertisingIntervalMax = 500;
constexpr uint8_t kLockNUSPriority = 2;
#endif

#ifdef CONFIG_THREAD_WIFI_SWITCHING
k_timer sSwitchImagesTimer;
constexpr uint32_t kSwitchImagesTimeout = 10000;
#endif

bool sIsNetworkProvisioned = false;
bool sIsNetworkEnabled = false;
bool sHaveBLEConnections = false;
} /* namespace */

#ifdef CONFIG_CHIP_WIFI
app::Clusters::NetworkCommissioning::Instance
	sWiFiCommissioningInstance(0, &(NetworkCommissioning::NrfWiFiDriver::Instance()));
#endif

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

#if defined(CONFIG_NET_L2_OPENTHREAD)
	err = ThreadStackMgr().InitThreadStack();
	if (err != CHIP_NO_ERROR) {
		LOG_ERR("ThreadStackMgr().InitThreadStack() failed: %s", ErrorStr(err));
		return err;
	}

#ifdef CONFIG_OPENTHREAD_MTD_SED
	err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_SleepyEndDevice);
#else
	err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif /* CONFIG_OPENTHREAD_MTD_SED */
	if (err != CHIP_NO_ERROR) {
		LOG_ERR("ConnectivityMgr().SetThreadDeviceType() failed: %s", ErrorStr(err));
		return err;
	}

#elif defined(CONFIG_CHIP_WIFI)
	sWiFiCommissioningInstance.Init();
#else
	return CHIP_ERROR_INTERNAL;
#endif /* CONFIG_NET_L2_OPENTHREAD */

	if (!GetBoardInterface().Init(ButtonEventHandler)) {
		LOG_ERR("User interface initialization failed.");
		return CHIP_ERROR_INCORRECT_STATE;
	}

#ifdef CONFIG_THREAD_WIFI_SWITCHING
	k_timer_init(&sSwitchImagesTimer, &AppTask::SwitchImagesTimerTimeoutCallback, nullptr);
#endif

#ifdef CONFIG_CHIP_NUS
	/* Initialize Nordic UART Service for Lock purposes */
	if (!GetNUSService().Init(kLockNUSPriority, kAdvertisingIntervalMin, kAdvertisingIntervalMax)) {
		ChipLogError(Zcl, "Cannot initialize NUS service");
	}
	GetNUSService().RegisterCommand("Lock", sizeof("Lock"), NUSLockCallback, nullptr);
	GetNUSService().RegisterCommand("Unlock", sizeof("Unlock"), NUSUnlockCallback, nullptr);
	if (!GetNUSService().StartServer()) {
		LOG_ERR("GetNUSService().StartServer() failed");
	}
#endif

	/* Initialize lock manager */
	BoltLockMgr().Init(LockStateChanged);

#ifdef CONFIG_CHIP_OTA_REQUESTOR
	/* OTA image confirmation must be done before the factory data init. */
	OtaConfirmNewImage();
#endif

#ifdef CONFIG_MCUMGR_TRANSPORT_BT
	/* Initialize DFU over SMP */
	GetDFUOverSMP().Init();
	GetDFUOverSMP().ConfirmNewImage();
#endif

	/* Initialize CHIP server */
#if CONFIG_CHIP_FACTORY_DATA
	ReturnErrorOnFailure(mFactoryDataProvider.Init());
	SetDeviceInstanceInfoProvider(&mFactoryDataProvider);
	SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);
	SetCommissionableDataProvider(&mFactoryDataProvider);
#else
	SetDeviceInstanceInfoProvider(&DeviceInstanceInfoProviderMgrImpl());
	SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif

	static chip::CommonCaseDeviceServerInitParams initParams;
	(void)initParams.InitializeStaticResourcesBeforeServerInit();

	ReturnErrorOnFailure(chip::Server::GetInstance().Init(initParams));
	ConfigurationMgr().LogDeviceConfig();
	PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
	AppFabricTableDelegate::Init();

	/*
	 * Add CHIP event handler and start CHIP thread.
	 * Note that all the initialization code should happen prior to this point to avoid data races
	 * between the main and the CHIP threads.
	 */
	PlatformMgr().AddEventHandler(ChipEventHandler, 0);

	err = PlatformMgr().StartEventLoopTask();
	if (err != CHIP_NO_ERROR) {
		LOG_ERR("PlatformMgr().StartEventLoopTask() failed");
		return err;
	}

#ifdef CONFIG_THREAD_WIFI_SWITCHING_CLI_SUPPORT
	RegisterSwitchCliCommand();
#endif

	return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::StartApp()
{
	ReturnErrorOnFailure(Init());

	while (true) {
		EventManager::DispatchEvent();
	}

	return CHIP_NO_ERROR;
}

void AppTask::ButtonEventHandler(DeviceButtons source, ButtonActions action)
{
	if (DeviceButtons::kAppButton == source && ButtonActions::kButtonPressed == action) {
		AppEvent event(AppEventType::LockEvent, LockActionEventHandler);
		EventManager::PostEvent(event);
	}

#ifdef CONFIG_THREAD_WIFI_SWITCHING
	if (DeviceButtons::kUserButton1 == source) {
		AppEvent event(AppEventType::ThreadWiFiSwitch, SwitchImagesTriggerHandler);
		LOG_INF("Action %d", static_cast<uint8_t>(action));
		event.ThreadWiFiSwitchEvent.ButtonAction = static_cast<uint8_t>(action);
		EventManager::PostEvent(event);
	}
#endif
}

void AppTask::LockActionEventHandler(const void *context)
{
	if (BoltLockMgr().IsLocked()) {
		BoltLockMgr().Unlock(BoltLockManager::OperationSource::kButton);
	} else {
		BoltLockMgr().Lock(BoltLockManager::OperationSource::kButton);
	}
}

#ifdef CONFIG_THREAD_WIFI_SWITCHING
void AppTask::SwitchImagesDone()
{
	/* Wipe out whole settings as they will not apply to the new application image. */
	chip::Server::GetInstance().ScheduleFactoryReset();
}

void AppTask::SwitchImagesEventHandler(const void *context)
{
	LOG_INF("Switching application from " CONFIG_APPLICATION_LABEL " to " CONFIG_APPLICATION_OTHER_LABEL);

#define CONCAT3(a, b, c) UTIL_CAT(UTIL_CAT(a, b), c)
	SoftwareImagesSwapper::ImageLocation source = {
		.app_address = CONCAT3(PM_APP_, CONFIG_APPLICATION_OTHER_IDX, _CORE_APP_ADDRESS),
		.app_size = CONCAT3(PM_APP_, CONFIG_APPLICATION_OTHER_IDX, _CORE_APP_SIZE),
		.net_address = CONCAT3(PM_APP_, CONFIG_APPLICATION_OTHER_IDX, _CORE_NET_ADDRESS),
		.net_size = CONCAT3(PM_APP_, CONFIG_APPLICATION_OTHER_IDX, _CORE_NET_SIZE),
	};
#undef CONCAT3

	SoftwareImagesSwapper::Instance().Swap(source, AppTask::SwitchImagesDone);
}

void AppTask::SwitchImagesTimerTimeoutCallback(k_timer *timer)
{
	chip::DeviceLayer::PlatformMgr().ScheduleWork(
		[](intptr_t context) {
			Instance().mSwitchImagesTimerActive = false;
			AppEvent event(AppEventType::ThreadWiFiSwitch, SwitchImagesEventHandler);
			EventManager::PostEvent(event);
		},
		0);
}

void AppTask::SwitchImagesTriggerHandler(const void *context)
{
	if (!context) {
		return;
	}

	AppEvent event(context);

	if (event.ThreadWiFiSwitchEvent.ButtonAction == static_cast<uint8_t>(ButtonActions::kButtonPressed) &&
	    !Instance().mSwitchImagesTimerActive) {
		k_timer_start(&sSwitchImagesTimer, K_MSEC(kSwitchImagesTimeout), K_NO_WAIT);
		Instance().mSwitchImagesTimerActive = true;
		LOG_INF("Keep button pressed for %u ms to switch application from " CONFIG_APPLICATION_LABEL
			" to " CONFIG_APPLICATION_OTHER_LABEL,
			kSwitchImagesTimeout);
	} else if (Instance().mSwitchImagesTimerActive) {
		k_timer_stop(&sSwitchImagesTimer);
		Instance().mSwitchImagesTimerActive = false;
		LOG_INF("Switching application from " CONFIG_APPLICATION_LABEL " to " CONFIG_APPLICATION_OTHER_LABEL
			" cancelled");
	}
}
#endif

void AppTask::ChipEventHandler(const ChipDeviceEvent *event, intptr_t /* arg */)
{
	switch (event->Type) {
	case DeviceEventType::kCHIPoBLEAdvertisingChange:
#ifdef CONFIG_CHIP_NFC_COMMISSIONING
		if (event->CHIPoBLEAdvertisingChange.Result == kActivity_Started) {
			if (NFCMgr().IsTagEmulationStarted()) {
				LOG_INF("NFC Tag emulation is already started");
			} else {
				ShareQRCodeOverNFC(
					chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
			}
		} else if (event->CHIPoBLEAdvertisingChange.Result == kActivity_Stopped) {
			NFCMgr().StopTagEmulation();
		}
#endif
		sHaveBLEConnections = ConnectivityMgr().NumBLEConnections() != 0;
		if (sHaveBLEConnections) {
			GetBoardInterface().UpdateDeviceState(DeviceState::kDeviceConnectedBLE);
		}
		break;
#if defined(CONFIG_NET_L2_OPENTHREAD)
	case DeviceEventType::kDnssdInitialized:
#if CONFIG_CHIP_OTA_REQUESTOR
		InitBasicOTARequestor();
#endif /* CONFIG_CHIP_OTA_REQUESTOR */
		break;
	case DeviceEventType::kThreadStateChange:
		sIsNetworkProvisioned = ConnectivityMgr().IsThreadProvisioned();
		sIsNetworkEnabled = ConnectivityMgr().IsThreadEnabled();
#elif defined(CONFIG_CHIP_WIFI)
	case DeviceEventType::kWiFiConnectivityChange:
		sIsNetworkProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
		sIsNetworkEnabled = ConnectivityMgr().IsWiFiStationEnabled();
#if CONFIG_CHIP_OTA_REQUESTOR
		if (event->WiFiConnectivityChange.Result == kConnectivity_Established) {
			InitBasicOTARequestor();
		}
#endif /* CONFIG_CHIP_OTA_REQUESTOR */
#endif
		if (sIsNetworkEnabled && sIsNetworkProvisioned) {
			GetBoardInterface().UpdateDeviceState(DeviceState::kDeviceProvisioned);
		} else {
			GetBoardInterface().UpdateDeviceState(DeviceState::kDeviceDisconnected);
		}
		break;
	default:
		break;
	}
}

void AppTask::LockStateChanged(BoltLockManager::State state, BoltLockManager::OperationSource source)
{
	switch (state) {
	case BoltLockManager::State::kLockingInitiated:
		LOG_INF("Lock action initiated");
		GetBoardInterface().GetLED(DeviceLeds::kAppLED).Blink(50, 50);
#ifdef CONFIG_CHIP_NUS
		GetNUSService().SendData("locking", sizeof("locking"));
#endif
		break;
	case BoltLockManager::State::kLockingCompleted:
		LOG_INF("Lock action completed");
		GetBoardInterface().GetLED(DeviceLeds::kAppLED).Set(true);
#ifdef CONFIG_CHIP_NUS
		GetNUSService().SendData("locked", sizeof("locked"));
#endif
		break;
	case BoltLockManager::State::kUnlockingInitiated:
		LOG_INF("Unlock action initiated");
		GetBoardInterface().GetLED(DeviceLeds::kAppLED).Blink(50, 50);
#ifdef CONFIG_CHIP_NUS
		GetNUSService().SendData("unlocking", sizeof("unlocking"));
#endif
		break;
	case BoltLockManager::State::kUnlockingCompleted:
		LOG_INF("Unlock action completed");
#ifdef CONFIG_CHIP_NUS
		GetNUSService().SendData("unlocked", sizeof("unlocked"));
#endif
		GetBoardInterface().GetLED(DeviceLeds::kAppLED).Set(false);
		break;
	}

	/* Handle changing attribute state in the application */
	Instance().UpdateClusterState(state, source);
}

void AppTask::UpdateClusterState(BoltLockManager::State state, BoltLockManager::OperationSource source)
{
	DlLockState newLockState;

	switch (state) {
	case BoltLockManager::State::kLockingCompleted:
		newLockState = DlLockState::kLocked;
		break;
	case BoltLockManager::State::kUnlockingCompleted:
		newLockState = DlLockState::kUnlocked;
		break;
	default:
		newLockState = DlLockState::kNotFullyLocked;
		break;
	}

	SystemLayer().ScheduleLambda([newLockState, source] {
		chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> currentLockState;
		chip::app::Clusters::DoorLock::Attributes::LockState::Get(kLockEndpointId, currentLockState);

		if (currentLockState.IsNull()) {
			/* Initialize lock state with start value, but not invoke lock/unlock. */
			chip::app::Clusters::DoorLock::Attributes::LockState::Set(kLockEndpointId, newLockState);
		} else {
			LOG_INF("Updating LockState attribute");

			if (!DoorLockServer::Instance().SetLockState(kLockEndpointId, newLockState, source)) {
				LOG_ERR("Failed to update LockState attribute");
			}
		}
	});
}

#ifdef CONFIG_THREAD_WIFI_SWITCHING_CLI_SUPPORT
void AppTask::RegisterSwitchCliCommand()
{
	static const shell_command_t sSwitchCommand = { [](int, char **) {
							       AppTask::Instance().SwitchImagesEventHandler(AppEvent{});
							       return CHIP_NO_ERROR;
						       },
							"switch_images",
							"Switch between Thread and Wi-Fi application variants" };
	Engine::Root().RegisterCommands(&sSwitchCommand, 1);
}
#endif

#ifdef CONFIG_CHIP_NUS
void AppTask::NUSLockCallback(void *context)
{
	LOG_DBG("Received LOCK command from NUS");
	if (BoltLockMgr().mState == BoltLockManager::State::kLockingCompleted ||
	    BoltLockMgr().mState == BoltLockManager::State::kLockingInitiated) {
		LOG_INF("Device is already locked");
	} else {
		AppEvent nusEvent;
		nus_event.Type = AppEventType::NUSCommand;
		nus_event.Handler = LockActionEventHandler;
		EventManager::PostEvent(nusEvent);
	}
}

void AppTask::NUSUnlockCallback(void *context)
{
	LOG_DBG("Received UNLOCK command from NUS");
	if (BoltLockMgr().mState == BoltLockManager::State::kUnlockingCompleted ||
	    BoltLockMgr().mState == BoltLockManager::State::kUnlockingInitiated) {
		LOG_INF("Device is already unlocked");
	} else {
		AppEvent nus_event;
		nus_event.Type = AppEventType::NUSCommand;
		nus_event.Handler = LockActionEventHandler;
		EventManager::PostEvent(nus_event);
	}
}
#endif
