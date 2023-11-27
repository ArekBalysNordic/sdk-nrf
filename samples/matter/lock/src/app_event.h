/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "event_manager.h"

enum class AppEventType : uint8_t { None = 0, NUSCommand, LockEvent, ThreadWiFiSwitch };

struct AppEvent : public Event {
	AppEvent()
		: Event(EventSource::Application, static_cast<uint8_t>(AppEventType::None), nullptr, this,
			sizeof(*this))
	{
	}
	AppEvent(AppEventType type, EventHandler handler = nullptr)
		: Event(EventSource::Application, static_cast<uint8_t>(type), handler, this, sizeof(*this))
	{
	}

	/**
	 * @brief Construct a new App Event object from context
	 *
	 * User must ensure that context exists, is defined and has the same size as the AppEvent class,
	 * if the context does not exist The AppEvent will not be updated with any data.
	 *
	 * @param context serialized context of the AppEvent
	 */
	AppEvent(const void *context)
	{
		if (context) {
			memcpy(this, context, sizeof(*this));
		}
	}

	union {
		struct {
			void *Context;
		} LockEvent;
		struct {
			uint8_t ButtonAction;
		} ThreadWiFiSwitchEvent;
	};
};
