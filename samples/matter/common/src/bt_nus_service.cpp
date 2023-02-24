/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "bt_nus_service.h"

#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#include <zephyr/settings/settings.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

LOG_MODULE_DECLARE(app, CONFIG_MATTER_LOG_LEVEL);

namespace
{
#if (CONFIG_BT_FIXED_PASSKEY)
constexpr uint32_t kDefaultPasskey = 123456;
#endif
constexpr uint32_t kAdvertisingOptions = BT_LE_ADV_OPT_CONNECTABLE;
constexpr uint8_t kAdvertisingFlags = BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR;
} // namespace

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = NUSService::Connected,
	.disconnected = NUSService::Disconnected,
	.security_changed = NUSService::SecurityChanged,
};
struct bt_conn_auth_cb NUSService::sConnAuthCallbacks = {
	.passkey_display = AuthPasskeyDisplay,
	.cancel = AuthCancel,
};
struct bt_conn_auth_info_cb NUSService::sConnAuthInfoCallbacks = {
	.pairing_complete = PairingComplete,
	.pairing_failed = PairingFailed,
};
struct bt_nus_cb NUSService::sNusCallbacks = {
	.received = RxCallback,
};

NUSService NUSService::sInstance;

CHIP_ERROR NUSService::Init(const char *name, size_t nameLen, uint8_t priority, uint16_t minInterval,
			    uint16_t maxInterval)
{
	VerifyOrReturnError(name && nameLen > 0, CHIP_ERROR_INVALID_ARGUMENT);

	mAdvertisingItems[0] = BT_DATA(BT_DATA_FLAGS, &kAdvertisingFlags, sizeof(kAdvertisingFlags));
	mAdvertisingItems[1] = BT_DATA(BT_DATA_NAME_COMPLETE, name, static_cast<uint8_t>(nameLen));

	mServiceItems[0] = BT_DATA(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL, sizeof(BT_UUID_NUS_VAL));

	mAdvertisingRequest.priority = priority;
	mAdvertisingRequest.options = kAdvertisingOptions;
	mAdvertisingRequest.minInterval = minInterval;
	mAdvertisingRequest.maxInterval = maxInterval;
	mAdvertisingRequest.advertisingData = Span<bt_data>(mAdvertisingItems);
	mAdvertisingRequest.scanResponseData = Span<bt_data>(mServiceItems);

	mAdvertisingRequest.onStarted = [](int rc) {
		if (rc == 0) {
			ChipLogDetail(DeviceLayer, "NUS BLE advertising started");
		} else {
			ChipLogError(DeviceLayer, "Failed to start NUS BLE advertising: %d", rc);
		}
	};
	mAdvertisingRequest.onStopped = []() { ChipLogDetail(DeviceLayer, "NUS BLE advertising stopped"); };

	settings_load();
#if (CONFIG_BT_FIXED_PASSKEY)
	VerifyOrReturnError(bt_passkey_set(kDefaultPasskey) != 0, CHIP_ERROR_INTERNAL);
#endif
	VerifyOrReturnError(bt_conn_auth_cb_register(&sConnAuthCallbacks) == 0, CHIP_ERROR_INTERNAL);
	VerifyOrReturnError(bt_conn_auth_info_cb_register(&sConnAuthInfoCallbacks) == 0, CHIP_ERROR_INTERNAL);
	VerifyOrReturnError(bt_nus_init(&sNusCallbacks) == 0, CHIP_ERROR_INTERNAL);

	return CHIP_NO_ERROR;
}

void NUSService::StartServer()
{
	VerifyOrReturn(!mIsStarted, ChipLogError(DeviceLayer, "NUS service was already started"));

	PlatformMgr().LockChipStack();
	CHIP_ERROR ret = BLEAdvertisingArbiter::InsertRequest(mAdvertisingRequest);
	PlatformMgr().UnlockChipStack();

	if (CHIP_NO_ERROR != ret) {
		ChipLogError(DeviceLayer, "Could not start NUS service");
		mIsStarted = false;
		return;
	}
	mIsStarted = true;
	ChipLogProgress(DeviceLayer, "NUS service started");
}

void NUSService::StopServer()
{
	VerifyOrReturn(mIsStarted);

	PlatformMgr().LockChipStack();
	BLEAdvertisingArbiter::CancelRequest(mAdvertisingRequest);
	PlatformMgr().UnlockChipStack();

	mIsStarted = false;
	ChipLogProgress(DeviceLayer, "NUS service stopped");
}

void NUSService::RxCallback(struct bt_conn *conn, const uint8_t *const data, uint16_t len)
{
	VerifyOrReturn(bt_conn_get_security(GetNUSService().mBTConnection) >= BT_SECURITY_L2);
	ChipLogDetail(DeviceLayer, "NUS received: %d bytes", len);
	GetNUSService().DispatchCommand(reinterpret_cast<const char *>(data), len);
}

CHIP_ERROR NUSService::RegisterCommand(const char *const name, size_t length, CommandCallback callback, void *context)
{
	VerifyOrReturnError(name, CHIP_ERROR_INVALID_ARGUMENT);

	struct Command newCommand {};
	memcpy(newCommand.command, name, length);
	newCommand.commandLen = length;
	newCommand.callback = callback;
	newCommand.context = context;
	mCommandsList.push_back(newCommand);

	return CHIP_NO_ERROR;
}

void NUSService::DispatchCommand(const char *const data, uint16_t len)
{
	for (auto c : mCommandsList) {
		if (strncmp(data, c.command, len) == 0) {
			if (c.callback) {
				PlatformMgr().LockChipStack();
				c.callback(c.context);
				PlatformMgr().UnlockChipStack();
			}
			return;
		}
	}
	ChipLogError(DeviceLayer, "NUS command unkown!");
}

CHIP_ERROR NUSService::SendData(const char *const data, size_t length)
{
	VerifyOrReturnError(mBTConnection, CHIP_ERROR_SENDING_BLOCKED);
	VerifyOrReturnError(bt_conn_get_security(GetNUSService().mBTConnection) >= BT_SECURITY_L2,
			    CHIP_ERROR_SENDING_BLOCKED);
	VerifyOrReturnError(bt_nus_send(mBTConnection, reinterpret_cast<const uint8_t *const>(data), length) == 0,
			    CHIP_ERROR_SENDING_BLOCKED);
	return CHIP_NO_ERROR;
}

void NUSService::Connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		ChipLogError(DeviceLayer, "NUS Connection failed (err %u)", err);
		return;
	}

	bt_conn_set_security(conn, BT_SECURITY_L3);

	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	ChipLogDetail(DeviceLayer, "NUS BT Connected to %s", addr);
}

void NUSService::Disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	ChipLogProgress(DeviceLayer, "NUS BT Disconnected from %s (reason %u)", addr, reason);
}

void NUSService::SecurityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
	if (!err) {
		char addr[BT_ADDR_LE_STR_LEN];
		bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
		ChipLogDetail(DeviceLayer, "NUS BT Security changed: %s level %u", addr, level);

		GetNUSService().mBTConnection = bt_conn_ref(conn);
	} else {
		ChipLogError(DeviceLayer, "NUS BT Security failed: level %u err %d", level, err);
	}
}

void NUSService::AuthPasskeyDisplay(struct bt_conn *conn, unsigned int passkey)
{
	ChipLogProgress(DeviceLayer, "PROVIDE THE FOLLOWING CODE IN YOUR MOBILE APP: %d", passkey);
}

void NUSService::AuthCancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	ChipLogProgress(DeviceLayer, "NUS BT Pairing cancelled: %s", addr);

	bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

void NUSService::PairingComplete(struct bt_conn *conn, bool bonded)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	ChipLogDetail(DeviceLayer, "NUS BT Pairing completed: %s, bonded: %d", addr, bonded);
}

void NUSService::PairingFailed(struct bt_conn *conn, enum bt_security_err reason)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	ChipLogError(DeviceLayer, "NUS BT Pairing failed to %s : reason %d", addr, static_cast<uint8_t>(reason));
}
