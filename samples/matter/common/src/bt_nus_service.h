/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "app_event.h"

#include <platform/Zephyr/BLEAdvertisingArbiter.h>

#include <bluetooth/services/nus.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/sys/ring_buffer.h>

#include <lib/core/CHIPError.h>

#include <list>

class NUSService {
public:
	using CommandCallback = void (*)(void *context);
	struct Command {
		char command[CONFIG_CHIP_NUS_MAX_COMMAND_LEN];
		size_t commandLen;
		CommandCallback callback;
		void *context;
	};

	/**
	 * @brief Initialize BLE Lock service
	 *
	 * Initialize internal structures and register necessary commands in BLE service server.
	 */
	CHIP_ERROR Init(const char *name, size_t nameLen, uint8_t priority, uint16_t minInterval, uint16_t maxInterval);

	/**
	 * @brief Start BLE Lock service server
	 *
	 * Register BLE Lock service that supports controlling lock and unlock operations
	 * using BLE commands. The BLE Lock service may begin immediately.
	 */
	void StartServer();

	/**
	 * @brief Stop BLE Lock service server
	 *
	 * Unregister BLE Lock service that supports controlling lock and unlock operations
	 * using BLE commands. The BLE Lock service will be disabled and the next service with the highest priority begin immediately.
	 */
	void StopServer();

	/**
	 * @brief Send data to the connected device
	 * 
	 * Send data to the connected and paired device.
	 * 
	 * @return CHIP_ERROR_SENDING_BLOCKED if the secure connection to the device was not established 
	 */
	CHIP_ERROR SendData(const char *const data, size_t length);

	/**
	 * @brief Register a new command for NUS service
	 * 
	 * The new command consist of a callback which will be call when the device receives provided command name.
	 * 
	 * @return CHIP_ERROR 
	 */
	CHIP_ERROR RegisterCommand(const char *const name, size_t length, CommandCallback callback, void *context);

	friend NUSService &GetLockNUSService();
	static NUSService sInstance;

	/**
	 * Static callbacks for BLE connection
	*/
	static void Connected(struct bt_conn *conn, uint8_t err);
	static void Disconnected(struct bt_conn *conn, uint8_t reason);
	static void SecurityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err);

private:

	void DispatchCommand(const char *const data, uint16_t len);

	static void RxCallback(struct bt_conn *conn, const uint8_t *const data, uint16_t len);
	static void AuthPasskeyDisplay(struct bt_conn *conn, unsigned int passkey);
	static void AuthPasskeyConfirm(struct bt_conn *conn, unsigned int passkey);
	static void AuthCancel(struct bt_conn *conn);
	static void PairingComplete(struct bt_conn *conn, bool bonded);
	static void PairingFailed(struct bt_conn *conn, enum bt_security_err reason);

	static struct bt_conn_auth_cb sConnAuthCallbacks;
	static struct bt_conn_auth_info_cb sConnAuthInfoCallbacks;
	static struct bt_nus_cb sNusCallbacks;

	bool mIsStarted = false;
	chip::DeviceLayer::BLEAdvertisingArbiter::Request mAdvertisingRequest = {};
	std::array<bt_data, 2> mAdvertisingItems;
	std::array<bt_data, 1> mServiceItems;
	std::list<Command> mCommandsList;
	struct bt_conn *mBTConnection;

};

inline NUSService &GetNUSService()
{
	return NUSService::sInstance;
}
