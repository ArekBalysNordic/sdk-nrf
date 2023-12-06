/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

enum class AppEventType : uint8_t { None = 0, NUSCommand, LockEvent, ThreadWiFiSwitch };

struct AppEvent {
	AppEvent(AppEventType type) { mType = type; }
	AppEvent() = default;

	union {
		struct {
			void *Context;
		} LockEvent;
		struct {
			uint8_t ButtonAction;
		} ThreadWiFiSwitchEvent;
	};

	AppEventType mType;
};
