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
#include <nrf_802154_types.h>
#include <nrf_802154.h>
#include <string.h>
#include <zephyr/init.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(nrf_802154_callbacks_dispatcher, LOG_LEVEL_INF);

static const struct nrf_802154_callbacks *s_active_client;

static const struct nrf_802154_callbacks *active_client(void)
{
	return s_active_client;
}

int nrf_802154_callbacks_dispatcher_activate(const char *name)
{
	if (name == NULL || name[0] == '\0') {
		s_active_client = NULL;
		return 0;
	}

	s_active_client = NULL;
	STRUCT_SECTION_FOREACH(nrf_802154_cb_dispatch_entry, entry)
	{
		if (entry->name != NULL && strcmp(entry->name, name) == 0) {
			s_active_client = entry->callbacks;

			LOG_INF("Activated client: %s", name);
			return 0;
		}
	}

	LOG_ERR("No client found with name: %s", name);
	return -EINVAL;
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
	STRUCT_SECTION_FOREACH(nrf_802154_cb_dispatch_entry, entry)
	{
		if (entry->callbacks != NULL && entry->callbacks->init != NULL) {

			/* Deinitialize the radio driver prior to initializing it again, but skip for the first entry */
			static bool first_entry = true;
			if (!first_entry) {
				nrf_802154_deinit();
			} else {
				first_entry = false;
			}

			/* Initialize the radio driver */
			entry->callbacks->init();

			/* Activate the last registered client to start receiving and transmitting
			 * packets */
			nrf_802154_callbacks_dispatcher_activate(entry->name);
		}
	}

	return 0;
}

SYS_INIT(nrf_802154_callbacks_dispatcher_init, POST_KERNEL,
	 CONFIG_NRF_802154_CALLBACKS_DISPATCHER_INIT_PRIORITY);
