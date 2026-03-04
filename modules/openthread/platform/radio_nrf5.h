/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef OT_PLATFORM_RADIO_NRF5_H
#define OT_PLATFORM_RADIO_NRF5_H

#include <nrf_802154_const.h>
#include <nrf_802154_types.h>
#include <stdint.h>

void openthread_platform_radio_set_eui64(uint8_t eui64[EXTENDED_ADDRESS_SIZE]);

/** nRF 802.15.4 radio driver callbacks (OpenThread platform implementation). */

void openthread_nrf_802154_radio_init(void);
void openthread_nrf_802154_received_timestamp_raw(uint8_t *data, int8_t power, uint8_t lqi,
						  uint64_t time);
void openthread_nrf_802154_receive_failed(nrf_802154_rx_error_t error, uint32_t id);
void openthread_nrf_802154_tx_ack_started(const uint8_t *data);
void openthread_nrf_802154_transmitted_raw(uint8_t *frame,
					   const nrf_802154_transmit_done_metadata_t *metadata);
void openthread_nrf_802154_transmit_failed(uint8_t *frame, nrf_802154_tx_error_t error,
					   const nrf_802154_transmit_done_metadata_t *metadata);
void openthread_nrf_802154_energy_detected(const nrf_802154_energy_detected_t *result);
void openthread_nrf_802154_energy_detection_failed(nrf_802154_ed_error_t error);
#if defined(CONFIG_NRF_802154_SER_HOST)
void openthread_nrf_802154_serialization_error(const nrf_802154_ser_err_data_t *err);
#endif

#endif /* OT_PLATFORM_RADIO_NRF5_H */
