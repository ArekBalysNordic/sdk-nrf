/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <cstdint>

#include "event_manager.h"
#include "led_widget.h"

enum class SystemEventType : uint8_t {
	None = 0,
	Button,
	ButtonPushed,
	ButtonReleased,
	Timer,
	UpdateLedState,
	SoftwareUpdate,
	FactoryReset,
	AdvertisingStart
};

struct SystemEvent : public Event {
	SystemEvent()
		: Event(EventSource::System, static_cast<uint8_t>(SystemEventType::None), nullptr, this, sizeof(*this))
	{
	}
	SystemEvent(SystemEventType type, EventHandler handler = nullptr)
		: Event(EventSource::System, static_cast<uint8_t>(type), handler, this, sizeof(*this))
	{
	}
	/**
	 * @brief Construct a new System Event object from context
	 *
	 * User must ensure that context exists, is defined and has the same size as the SystemEvent class,
	 * if the context does not exist The SystemEvent will not be updated with any data.
	 *
	 * @param context serialized context of the SystemEvent
	 */
	SystemEvent(const void *context)
	{
		if (context) {
			memcpy(this, context, sizeof(*this));
		}
	}

	union {
		struct {
			uint8_t PinNo;
			uint8_t Action;
		} ButtonEvent;
		struct {
			k_timer *Timer;
		} TimerEvent;
		struct {
			LEDWidget *LedWidget;
		} UpdateLedStateEvent;
	};
};
