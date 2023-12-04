/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <lib/core/CHIPError.h>

enum class EventSource : uint8_t { Undefined, Application, System };

struct Event {
	using EventHandler = void (*)(const void *);

	Event(EventSource source, uint8_t type, EventHandler handler = nullptr, void *context = nullptr,
	      size_t contextSize = 0)
	{
		mSource = source;
		mType = type;
		mHandler = handler;
		mContext = context;
		mContextSize = contextSize;
	}
	Event() { mSource = EventSource::Undefined; }

	EventSource mSource;
	uint8_t mType;
	EventHandler mHandler;

private:
	friend class EventManager;

	void *mContext;
	size_t mContextSize;
};

class EventManager {
public:

	/**
	 * @brief Dispatch an event from the event queue and call associated handler.
	 * 
	 * This method should be run in the while loop within the application thread.
	 * 
	 */
	static void DispatchEvent();

	/**
	 * @brief Post an event to the event queue
	 * 
	 * This method cannot be run from the ISR context.
	 * It should be called from the Zephyr's thread.
	 * 
	 * @param event event to be posted.
	 * @return CHIP_ERROR_NO_MEMORY if there is no memory to post new Event
	 * @return CHIP_NO_ERROR if the Event has been posted
	 */
	static CHIP_ERROR PostEvent(Event &event);
};
