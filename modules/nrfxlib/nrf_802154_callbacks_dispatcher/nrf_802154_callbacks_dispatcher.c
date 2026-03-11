/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file
 *   This file implements the callbacks dispatcher for the nRF IEEE802.15.4 radio driver.
 */

#include "nrf_802154_callbacks_dispatcher.h"
#include <errno.h>
#include <nrf_802154.h>
#include <nrf_802154_types.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/logging/log.h>
#include <zephyr/spinlock.h>
#include <platform/nrf_802154_platform_sl_lptimer.h>
#include <platform/nrf_802154_platform_timestamper.h>

LOG_MODULE_REGISTER(nrf_802154_callbacks_dispatcher, LOG_LEVEL_INF);

static const struct nrf_802154_cb_dispatch_entry *s_active_entry;
static bool s_driver_initialized;
static struct k_spinlock s_dispatcher_lock;
static K_MUTEX_DEFINE(s_switch_mutex);

static const struct nrf_802154_cb_dispatch_entry *entry_lookup(const char *name)
{
	STRUCT_SECTION_FOREACH(nrf_802154_cb_dispatch_entry, entry)
	{
		if (entry->name != NULL && strcmp(entry->name, name) == 0) {
			return entry;
		}
	}

	return NULL;
}

static const struct nrf_802154_cb_dispatch_entry *active_entry_get(void)
{
	const struct nrf_802154_cb_dispatch_entry *entry;
	k_spinlock_key_t key = k_spin_lock(&s_dispatcher_lock);

	entry = s_active_entry;

	k_spin_unlock(&s_dispatcher_lock, key);

	return entry;
}

static void active_entry_set(const struct nrf_802154_cb_dispatch_entry *entry)
{
	k_spinlock_key_t key = k_spin_lock(&s_dispatcher_lock);

	s_active_entry = entry;

	k_spin_unlock(&s_dispatcher_lock, key);
}

static const struct nrf_802154_callbacks *active_client(void)
{
	const struct nrf_802154_cb_dispatch_entry *entry = active_entry_get();

	return entry != NULL ? entry->callbacks : NULL;
}

static int driver_quiesce(void)
{
	for (int attempt = 0; attempt < 100; attempt++) {
		(void)nrf_802154_transmit_at_cancel();

		if (nrf_802154_sleep_if_idle() == NRF_802154_SLEEP_ERROR_NONE) {
			return 0;
		}

		(void)nrf_802154_sleep();
		k_busy_wait(50);
	}

	LOG_ERR("Timed out waiting for radio to enter sleep");
	return -EBUSY;
}

static void driver_shutdown(void)
{
	nrf_802154_deinit();

	/* These cleanups are called explicitly to cover platform resources that are
	 * not fully released by all current driver teardown paths.
	 */
	nrf_802154_platform_timestamper_deinit();
	nrf_802154_platform_sl_lp_timer_deinit();
}

static void driver_startup(void)
{
	nrf_802154_init();
}

int nrf_802154_callbacks_dispatcher_activate(const char *name)
{
	const struct nrf_802154_cb_dispatch_entry *entry = NULL;

	if (name != NULL && name[0] != '\0') {
		entry = entry_lookup(name);
		if (entry == NULL) {
			LOG_ERR("No client found with name: %s", name);
			return -EINVAL;
		}
	}

	active_entry_set(entry);

	if (entry != NULL) {
		LOG_INF("Activated client: %s", entry->name);
	} else {
		LOG_INF("Activated client: none");
	}

	return 0;
}

int nrf_802154_callbacks_dispatcher_switch(const char *name, bool reinit_clients)
{
	const struct nrf_802154_cb_dispatch_entry *prev_entry;
	const struct nrf_802154_cb_dispatch_entry *next_entry = NULL;
	const struct nrf_802154_callbacks *prev_client;
	const struct nrf_802154_callbacks *next_client;
	int err = 0;

	if (name != NULL && name[0] != '\0') {
		next_entry = entry_lookup(name);
		if (next_entry == NULL) {
			LOG_ERR("No client found with name: %s", name);
			return -EINVAL;
		}
	}

	prev_entry = active_entry_get();
	prev_client = prev_entry != NULL ? prev_entry->callbacks : NULL;
	next_client = next_entry != NULL ? next_entry->callbacks : NULL;

	k_mutex_lock(&s_switch_mutex, K_FOREVER);

	prev_entry = active_entry_get();
	prev_client = prev_entry != NULL ? prev_entry->callbacks : NULL;

	if (prev_entry == next_entry && (!reinit_clients || s_driver_initialized)) {
		k_mutex_unlock(&s_switch_mutex);
		return 0;
	}

	if (reinit_clients) {
		active_entry_set(NULL);

		if (s_driver_initialized && prev_client != NULL && prev_client->deinit != NULL) {
			prev_client->deinit();
		}

		if (s_driver_initialized) {
			err = driver_quiesce();
			if (err != 0) {
				active_entry_set(prev_entry);
				k_mutex_unlock(&s_switch_mutex);
				return err;
			}

			driver_shutdown();
			s_driver_initialized = false;
		}

		if (next_client != NULL) {
			driver_startup();
			s_driver_initialized = true;

			if (next_client->init != NULL) {
				next_client->init();
			}
		}
	}

	active_entry_set(next_entry);
	k_mutex_unlock(&s_switch_mutex);

	if (next_entry != NULL) {
		LOG_INF("Switched client: %s (reinit=%d)", next_entry->name, reinit_clients);
	} else {
		LOG_INF("Switched client: none (reinit=%d)", reinit_clients);
	}

	return 0;
}

void nrf_802154_received_timestamp_raw(uint8_t *data, int8_t power, uint8_t lqi, uint64_t time)
{
	const struct nrf_802154_callbacks *cb = active_client();

	if (cb != NULL && cb->received_timestamp_raw != NULL) {
		cb->received_timestamp_raw(data, power, lqi, time);
	}
}

void nrf_802154_receive_failed(nrf_802154_rx_error_t error, uint32_t id)
{
	const struct nrf_802154_callbacks *cb = active_client();

	if (cb != NULL && cb->receive_failed != NULL) {
		cb->receive_failed(error, id);
	}
}

void nrf_802154_tx_ack_started(const uint8_t *data)
{
	const struct nrf_802154_callbacks *cb = active_client();

	if (cb != NULL && cb->tx_ack_started != NULL) {
		cb->tx_ack_started(data);
	}
}

void nrf_802154_transmitted_raw(uint8_t *frame, const nrf_802154_transmit_done_metadata_t *metadata)
{
	const struct nrf_802154_callbacks *cb = active_client();

	if (cb != NULL && cb->transmitted_raw != NULL) {
		cb->transmitted_raw(frame, metadata);
	}
}

void nrf_802154_transmit_failed(uint8_t *frame, nrf_802154_tx_error_t error,
				const nrf_802154_transmit_done_metadata_t *metadata)
{
	const struct nrf_802154_callbacks *cb = active_client();

	if (cb != NULL && cb->transmit_failed != NULL) {
		cb->transmit_failed(frame, error, metadata);
	}
}

void nrf_802154_energy_detected(const nrf_802154_energy_detected_t *result)
{
	const struct nrf_802154_callbacks *cb = active_client();

	if (cb != NULL && cb->energy_detected != NULL) {
		cb->energy_detected(result);
	}
}

void nrf_802154_energy_detection_failed(nrf_802154_ed_error_t error)
{
	const struct nrf_802154_callbacks *cb = active_client();

	if (cb != NULL && cb->energy_detection_failed != NULL) {
		cb->energy_detection_failed(error);
	}
}

#if defined(CONFIG_NRF_802154_SER_HOST)
void nrf_802154_serialization_error(const nrf_802154_ser_err_data_t *err)
{
	const struct nrf_802154_callbacks *cb = active_client();

	if (cb != NULL && cb->serialization_error != NULL) {
		cb->serialization_error(err);
	}
}
#endif

static int nrf_802154_callbacks_dispatcher_init(void)
{
	return nrf_802154_callbacks_dispatcher_switch("zigbee_nrf_802154_radio", true);
}

SYS_INIT(nrf_802154_callbacks_dispatcher_init, POST_KERNEL,
	 CONFIG_NRF_802154_CALLBACKS_DISPATCHER_INIT_PRIORITY);
