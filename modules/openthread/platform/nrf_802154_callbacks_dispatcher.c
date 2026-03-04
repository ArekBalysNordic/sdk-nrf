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

static const struct nrf_802154_callbacks *s_clients[NRF_802154_CALLBACKS_DISPATCHER_MAX_CLIENTS];
static uint32_t s_active_index = NRF_802154_CALLBACKS_DISPATCHER_INDEX_NONE;

static const struct nrf_802154_callbacks *active_client(void)
{
	if (s_active_index >= NRF_802154_CALLBACKS_DISPATCHER_MAX_CLIENTS) {
		return NULL;
	}
	return s_clients[s_active_index];
}

int nrf_802154_callbacks_dispatcher_register(const struct nrf_802154_callbacks *callbacks,
					     uint32_t *out_index)
{
	if (callbacks == NULL || out_index == NULL) {
		return -EINVAL;
	}
	for (uint32_t i = 0; i < NRF_802154_CALLBACKS_DISPATCHER_MAX_CLIENTS; i++) {
		if (s_clients[i] == NULL) {
			s_clients[i] = callbacks;
			*out_index = i;
			return 0;
		}
	}
	return -ENOMEM;
}

int nrf_802154_callbacks_dispatcher_activate(uint32_t index)
{
	if (index != NRF_802154_CALLBACKS_DISPATCHER_INDEX_NONE &&
	    index >= NRF_802154_CALLBACKS_DISPATCHER_MAX_CLIENTS) {
		return -EINVAL;
	}
	if (index != NRF_802154_CALLBACKS_DISPATCHER_INDEX_NONE && s_clients[index] == NULL) {
		return -EINVAL;
	}

    nrf_802154_deinit();

    if (s_clients[index]->init == NULL) {
        return -EINVAL;
    }
    
    s_clients[index]->init();
	s_active_index = index;
	return 0;
}

int nrf_802154_callbacks_dispatcher_unregister(uint32_t index)
{
	if (index >= NRF_802154_CALLBACKS_DISPATCHER_MAX_CLIENTS || s_clients[index] == NULL) {
		return -EINVAL;
	}
	s_clients[index] = NULL;
	if (s_active_index == index) {
		s_active_index = NRF_802154_CALLBACKS_DISPATCHER_INDEX_NONE;
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

void nrf_802154_transmitted_raw(uint8_t *frame,
				const nrf_802154_transmit_done_metadata_t *metadata)
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