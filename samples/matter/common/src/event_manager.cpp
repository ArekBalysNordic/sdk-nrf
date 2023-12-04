/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "event_manager.h"
#include "app_event.h"
#include "system_event.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <platform/Zephyr/SysHeapMalloc.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

constexpr size_t kEventQueueSize = 10;

K_MSGQ_DEFINE(sEventQueue, sizeof(Event), kEventQueueSize, alignof(Event));

CHIP_ERROR EventManager::PostEvent(Event &event)
{
	/* Allocate context on the Heap */
	void* context = chip::DeviceLayer::Malloc::Malloc(event.mContextSize);
	if(!context){
		LOG_ERR("Failed to store context in Heap");
		return CHIP_ERROR_NO_MEMORY;
	}
	memcpy(context, event.mContext, event.mContextSize);

	/* Replace context with the allocated bytes */
	event.mContext = context;
	
	if (k_msgq_put(&sEventQueue, &event, K_NO_WAIT) != 0) {
		LOG_ERR("Failed to post event to app task event queue");
		return CHIP_ERROR_NO_MEMORY;
	}

	return CHIP_NO_ERROR;
}

void EventManager::DispatchEvent()
{
	Event event;
	k_msgq_get(&sEventQueue, &event, K_FOREVER);
	if(event.mHandler)
	{
		event.mHandler(event.mContext);
	}

	/* Release context after the usage */
	chip::DeviceLayer::Malloc::Free(event.mContext);
}
